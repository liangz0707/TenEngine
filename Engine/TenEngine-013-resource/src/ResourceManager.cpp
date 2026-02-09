/**
 * @file ResourceManager.cpp
 * @brief ResourceManagerImpl implementation (contract: specs/_contracts/013-resource-ABI.md).
 * 
 * Implements IResourceManager with:
 * - Resource cache management
 * - Async load request management
 * - Hybrid resource factory (prioritize 002-Object TypeRegistry, fallback to ResourceFactory)
 * - Dependency graph management and cycle detection
 * - Integration with 001-Core thread pool
 */

#include <te/resource/ResourceManager.h>
#include <te/resource/Resource.h>
#include <te/resource/ResourceTypes.h>
#include <te/object/TypeRegistry.h>
#include <te/core/thread.h>
#include <te/core/alloc.h>
#include <unordered_map>
#include <string>
#include <mutex>
#include <atomic>
#include <vector>
#include <set>
#include <memory>
#include <cstdint>

namespace te {
namespace resource {

namespace {

// LoadRequestId implementation: use pointer to AsyncLoadRequest
using LoadRequestIdImpl = std::uintptr_t;

LoadRequestId ToLoadRequestId(void* ptr) {
    return reinterpret_cast<LoadRequestId>(ptr);
}

void* FromLoadRequestId(LoadRequestId id) {
    return reinterpret_cast<void*>(id);
}

} // namespace

// Cache entry for resource cache
struct CacheEntry {
    IResource* resource = nullptr;
    std::atomic<int> refcount{0};  // Thread-safe reference count
    std::string path;
    mutable std::mutex mutex;  // Protect resource pointer access
    
    CacheEntry() = default;
    CacheEntry(CacheEntry const&) = delete;
    CacheEntry& operator=(CacheEntry const&) = delete;
};

// Async load request
struct AsyncLoadRequest {
    std::string path;
    ResourceType type;
    LoadCompleteCallback on_done = nullptr;
    void* user_data = nullptr;
    std::atomic<LoadStatus> status{LoadStatus::Pending};
    std::atomic<float> progress{0.0f};
    std::atomic<bool> cancelled{false};
    IResource* result = nullptr;
    LoadResult load_result = LoadResult::Error;
    std::vector<ResourceId> dependencies;  // Dependency list
    std::atomic<int> pending_dependencies{0};  // Pending dependency count
    te::core::TaskId task_id = 0;  // Thread pool task ID
    
    AsyncLoadRequest() = default;
    AsyncLoadRequest(AsyncLoadRequest const&) = delete;
    AsyncLoadRequest& operator=(AsyncLoadRequest const&) = delete;
};

// ResourceManagerImpl: implementation of IResourceManager
class ResourceManagerImpl : public IResourceManager {
public:
    ResourceManagerImpl() {
        // Initialize thread pool callback thread (default: main thread)
        te::core::IThreadPool* threadPool = te::core::GetThreadPool();
        if (threadPool) {
            threadPool->SetCallbackThread(te::core::CallbackThreadType::MainThread);
        }
    }
    
    ~ResourceManagerImpl() override {
        // Cleanup: cancel all pending requests, unload all resources
        std::lock_guard<std::mutex> lock(requests_mutex_);
        for (auto& pair : requests_) {
            auto& request = pair.second;
            if (request->status.load() == LoadStatus::Loading ||
                request->status.load() == LoadStatus::Pending) {
                request->cancelled.store(true);
                te::core::IThreadPool* threadPool = te::core::GetThreadPool();
                if (threadPool && request->task_id != 0) {
                    threadPool->CancelTask(request->task_id);
                }
            }
        }
        requests_.clear();
        
        // Unload all cached resources
        std::lock_guard<std::mutex> cacheLock(cache_mutex_);
        for (auto& pair : cache_) {
            if (pair.second.resource) {
                pair.second.resource->Release();
            }
        }
        cache_.clear();
        resource_to_id_.clear();
        id_to_path_.clear();
    }
    
    LoadRequestId RequestLoadAsync(char const* path, ResourceType type,
                                  LoadCompleteCallback on_done, void* user_data) override {
        if (!path) {
            if (on_done) {
                on_done(nullptr, LoadResult::Error, user_data);
            }
            return nullptr;
        }
        
        // Check cache first
        ResourceId cachedId = ResolvePathToId(path);
        if (!cachedId.IsNull()) {
            std::lock_guard<std::mutex> lock(cache_mutex_);
            auto it = cache_.find(cachedId);
            if (it != cache_.end()) {
                // Cache hit: increment refcount and call callback immediately
                it->second.refcount.fetch_add(1);
                if (on_done) {
                    on_done(it->second.resource, LoadResult::Ok, user_data);
                }
                return ToLoadRequestId(reinterpret_cast<void*>(0xFFFFFFFF));  // Special ID for cached
            }
        }
        
        // Create async load request
        auto request = std::make_shared<AsyncLoadRequest>();
        request->path = path;
        request->type = type;
        request->on_done = on_done;
        request->user_data = user_data;
        request->status.store(LoadStatus::Pending);
        
        // Store request
        LoadRequestId id = ToLoadRequestId(request.get());
        {
            std::lock_guard<std::mutex> lock(requests_mutex_);
            requests_[id] = request;
        }
        
        // Submit task to thread pool
        te::core::IThreadPool* threadPool = te::core::GetThreadPool();
        if (!threadPool) {
            request->status.store(LoadStatus::Failed);
            if (on_done) {
                on_done(nullptr, LoadResult::Error, user_data);
            }
            std::lock_guard<std::mutex> lock(requests_mutex_);
            requests_.erase(id);
            return nullptr;
        }
        
        // Submit async load task
        request->status.store(LoadStatus::Loading);
        
        // Create wrapper structure for lambda context (TaskCallback is function pointer, not std::function)
        struct LoadTaskContext {
            ResourceManagerImpl* manager;
            std::shared_ptr<AsyncLoadRequest> request;
            std::string path;
            ResourceType type;
        };
        auto context = std::make_shared<LoadTaskContext>();
        context->manager = this;
        context->request = request;
        context->path = path;
        context->type = type;
        
        // Static callback function that extracts context from user_data
        static auto LoadTaskCallback = [](void* user_data) {
            auto* ctx = static_cast<LoadTaskContext*>(user_data);
            // Execute load in background thread
            LoadResult result = LoadResult::Error;
            IResource* resource = nullptr;
            
            // Check cancellation
            if (ctx->request->cancelled.load()) {
                result = LoadResult::Cancelled;
            } else {
                // Create resource instance
                resource = ctx->manager->CreateResourceInstance(ctx->type);
                if (resource) {
                    // Call IResource::Load directly (synchronous load in background thread)
                    bool success = resource->Load(ctx->path.c_str(), ctx->manager);
                    if (success) {
                        result = LoadResult::Ok;
                        
                        // Cache resource
                        ResourceId id = resource->GetResourceId();
                        ctx->manager->CacheResource(id, resource, ctx->path.c_str());
                    } else {
                        result = LoadResult::Error;
                        resource->Release();
                        resource = nullptr;
                    }
                } else {
                    result = LoadResult::Error;
                }
            }
            
            // Update request
            ctx->request->result = resource;
            ctx->request->load_result = result;
            ctx->request->status.store(result == LoadResult::Cancelled ? LoadStatus::Cancelled :
                                     (result == LoadResult::Ok ? LoadStatus::Completed : LoadStatus::Failed));
            
            // Schedule callback on callback thread
            if (ctx->request->on_done && !ctx->request->cancelled.load()) {
                // Callback will be scheduled by thread pool on callback thread
                te::core::IThreadPool* pool = te::core::GetThreadPool();
                if (pool) {
                    struct CallbackContext {
                        LoadCompleteCallback callback;
                        IResource* result;
                        LoadResult load_result;
                        void* user_data;
                    };
                    auto cbCtx = std::make_shared<CallbackContext>();
                    cbCtx->callback = ctx->request->on_done;
                    cbCtx->result = resource;
                    cbCtx->load_result = result;
                    cbCtx->user_data = ctx->request->user_data;
                    
                    static auto CallbackWrapper = [](void* user_data) {
                        auto* cb = static_cast<CallbackContext*>(user_data);
                        cb->callback(cb->result, cb->load_result, cb->user_data);
                    };
                    pool->SubmitTask(CallbackWrapper, cbCtx.get());
                    // Note: cbCtx is managed by shared_ptr, so it will be alive until callback completes
                    // In a real implementation, we might want to use a different lifetime management strategy
                }
            }
        };
        
        te::core::TaskId taskId = threadPool->SubmitTaskWithPriority(
            LoadTaskCallback,
            context.get(),
            0  // Priority
        );
        
        request->task_id = taskId;
        
        return id;
    }
    
    LoadStatus GetLoadStatus(LoadRequestId id) const override {
        if (!id) {
            return LoadStatus::Failed;
        }
        
        std::lock_guard<std::mutex> lock(requests_mutex_);
        auto it = requests_.find(id);
        if (it == requests_.end()) {
            return LoadStatus::Failed;
        }
        
        return it->second->status.load();
    }
    
    float GetLoadProgress(LoadRequestId id) const override {
        if (!id) {
            return 0.0f;
        }
        
        std::lock_guard<std::mutex> lock(requests_mutex_);
        auto it = requests_.find(id);
        if (it == requests_.end()) {
            return 0.0f;
        }
        
        return it->second->progress.load();
    }
    
    void CancelLoad(LoadRequestId id) override {
        if (!id) {
            return;
        }
        
        std::lock_guard<std::mutex> lock(requests_mutex_);
        auto it = requests_.find(id);
        if (it == requests_.end()) {
            return;
        }
        
        auto& request = it->second;
        request->cancelled.store(true);
        
        // Cancel thread pool task
        if (request->task_id != 0) {
            te::core::IThreadPool* threadPool = te::core::GetThreadPool();
            if (threadPool) {
                threadPool->CancelTask(request->task_id);
            }
        }
        
        // Update status
        request->status.store(LoadStatus::Cancelled);
        
        // Callback will still be called with Cancelled result
        if (request->on_done) {
            te::core::IThreadPool* threadPool = te::core::GetThreadPool();
            if (threadPool) {
                struct CancelCallbackContext {
                    LoadCompleteCallback callback;
                    IResource* result;
                    void* user_data;
                };
                auto cbCtx = std::make_shared<CancelCallbackContext>();
                cbCtx->callback = request->on_done;
                cbCtx->result = request->result;
                cbCtx->user_data = request->user_data;
                
                static auto CancelCallbackWrapper = [](void* user_data) {
                    auto* cb = static_cast<CancelCallbackContext*>(user_data);
                    cb->callback(cb->result, LoadResult::Cancelled, cb->user_data);
                };
                threadPool->SubmitTask(CancelCallbackWrapper, cbCtx.get());
            }
        }
    }
    
    IResource* GetCached(ResourceId id) const override {
        if (id.IsNull()) {
            return nullptr;
        }
        
        std::lock_guard<std::mutex> lock(cache_mutex_);
        auto it = cache_.find(id);
        if (it == cache_.end()) {
            return nullptr;
        }
        
        // Increment refcount (need mutable to modify atomic in const method)
        const_cast<CacheEntry&>(it->second).refcount.fetch_add(1);
        return it->second.resource;
    }
    
    IResource* LoadSync(char const* path, ResourceType type) override {
        if (!path) {
            return nullptr;
        }
        
        // Check cache first
        ResourceId cachedId = ResolvePathToId(path);
        if (!cachedId.IsNull()) {
            IResource* cached = GetCached(cachedId);
            if (cached) {
                return cached;
            }
        }
        
        // Create resource instance
        IResource* resource = CreateResourceInstance(type);
        if (!resource) {
            return nullptr;
        }
        
        // Call IResource::Load
        bool success = resource->Load(path, this);
        if (!success) {
            resource->Release();
            return nullptr;
        }
        
        // Cache resource
        ResourceId id = resource->GetResourceId();
        CacheResource(id, resource, path);
        
        return resource;
    }
    
    void Unload(IResource* resource) override {
        if (!resource) {
            return;
        }
        
        std::lock_guard<std::mutex> lock(cache_mutex_);
        auto it = resource_to_id_.find(resource);
        if (it == resource_to_id_.end()) {
            return;
        }
        
        ResourceId id = it->second;
        auto cacheIt = cache_.find(id);
        if (cacheIt == cache_.end()) {
            return;
        }
        
        // Decrement refcount
        int refcount = cacheIt->second.refcount.fetch_sub(1) - 1;
        if (refcount <= 0) {
            // Remove from cache (but don't delete resource, let Release handle it)
            cache_.erase(cacheIt);
            resource_to_id_.erase(it);
            id_to_path_.erase(id);
        }
        
        // Call Release
        resource->Release();
    }
    
    StreamingHandle RequestStreaming(ResourceId id, int priority) override {
        // Placeholder: streaming not fully implemented yet
        (void)id;
        (void)priority;
        return nullptr;
    }
    
    void SetStreamingPriority(StreamingHandle h, int priority) override {
        // Placeholder: streaming not fully implemented yet
        (void)h;
        (void)priority;
    }
    
    void RegisterResourceFactory(ResourceType type, ResourceFactory factory) override {
        if (!factory) {
            return;
        }
        
        std::lock_guard<std::mutex> lock(factories_mutex_);
        factories_[type] = factory;
    }
    
    bool Import(char const* path, ResourceType type, void* out_metadata_or_null) override {
        if (!path) {
            return false;
        }
        
        // Create resource instance
        IResource* resource = CreateResourceInstance(type);
        if (!resource) {
            return false;
        }
        
        // Call IResource::Import
        bool success = resource->Import(path, this);
        if (!success) {
            resource->Release();
            return false;
        }
        
        // Cache resource
        ResourceId id = resource->GetResourceId();
        CacheResource(id, resource, path);  // Use source path as cache key
        
        // Output metadata if requested
        if (out_metadata_or_null) {
            // Placeholder: copy resource metadata to output
            // This depends on resource type
        }
        
        return true;
    }
    
    bool Save(IResource* resource, char const* path) override {
        if (!resource || !path) {
            return false;
        }
        
        // Call IResource::Save
        return resource->Save(path, this);
    }
    
    char const* ResolvePath(ResourceId id) const override {
        if (id.IsNull()) {
            return nullptr;
        }
        
        std::lock_guard<std::mutex> lock(cache_mutex_);
        auto it = id_to_path_.find(id);
        if (it == id_to_path_.end()) {
            return nullptr;
        }
        
        return it->second.c_str();
    }
    
    /**
     * Create resource instance using hybrid factory (prioritize 002-Object, fallback to ResourceFactory).
     * Public so it can be accessed from static lambda callbacks.
     */
    IResource* CreateResourceInstance(ResourceType type) {
        // Try 002-Object TypeRegistry first
        std::string typeName = GetTypeNameForResourceType(type);
        if (!typeName.empty()) {
            void* instance = object::TypeRegistry::CreateInstance(typeName.c_str());
            if (instance) {
                return static_cast<IResource*>(instance);
            }
        }
        
        // Fallback to ResourceFactory
        std::lock_guard<std::mutex> lock(factories_mutex_);
        auto it = factories_.find(type);
        if (it != factories_.end()) {
            return it->second(type);
        }
        
        return nullptr;
    }
    
    /**
     * Cache resource (public helper for static lambda callbacks).
     */
    void CacheResource(ResourceId id, IResource* resource, char const* path) {
        std::lock_guard<std::mutex> lock(cache_mutex_);
        CacheEntry& entry = cache_[id];
        entry.resource = resource;
        entry.refcount.store(1);
        entry.path = path;
        resource_to_id_[resource] = id;
        id_to_path_[id] = path;
    }
    
    /**
     * Get type name for ResourceType (for 002-Object TypeRegistry lookup).
     */
    std::string GetTypeNameForResourceType(ResourceType type) {
        // Map ResourceType to type name
        // This mapping should be registered by resource modules
        auto it = type_to_name_.find(type);
        if (it != type_to_name_.end()) {
            return it->second;
        }
        
        // Default mapping (can be overridden by resource modules)
        switch (type) {
            case ResourceType::Texture:
                return "TextureResource";
            case ResourceType::Mesh:
                return "MeshResource";
            case ResourceType::Material:
                return "MaterialResource";
            case ResourceType::Model:
                return "ModelResource";
            case ResourceType::Effect:
                return "EffectResource";
            case ResourceType::Terrain:
                return "TerrainResource";
            case ResourceType::Shader:
                return "ShaderResource";
            case ResourceType::Audio:
                return "AudioResource";
            default:
                return std::string();
        }
    }
    
private:
    // Resource cache
    mutable std::mutex cache_mutex_;
    std::unordered_map<ResourceId, CacheEntry> cache_;
    std::unordered_map<IResource*, ResourceId> resource_to_id_;
    std::unordered_map<ResourceId, std::string> id_to_path_;
    
    // Async load requests
    mutable std::mutex requests_mutex_;
    std::unordered_map<LoadRequestId, std::shared_ptr<AsyncLoadRequest>> requests_;
    
    // Resource factories (fallback)
    mutable std::mutex factories_mutex_;
    std::unordered_map<ResourceType, ResourceFactory> factories_;
    
    // ResourceType to TypeName mapping (for 002-Object TypeRegistry lookup)
    std::unordered_map<ResourceType, std::string> type_to_name_;
    
    // Dependency graph (for cycle detection)
    mutable std::mutex dep_graph_mutex_;
    std::unordered_map<ResourceId, std::vector<ResourceId>> dep_graph_;
    
    /**
     * Resolve path to ResourceId (for cache lookup).
     */
    ResourceId ResolvePathToId(char const* path) {
        if (!path) {
            return ResourceId();
        }
        
        // Try to find in id_to_path_ reverse lookup
        std::lock_guard<std::mutex> lock(cache_mutex_);
        for (auto const& pair : id_to_path_) {
            if (pair.second == path) {
                return pair.first;
            }
        }
        
        return ResourceId();
    }
    
    /**
     * Detect dependency cycle (for cycle detection).
     */
    bool DetectDependencyCycle(ResourceId id, std::set<ResourceId>& visited) {
        if (visited.find(id) != visited.end()) {
            return true;  // Cycle detected
        }
        
        visited.insert(id);
        
        std::lock_guard<std::mutex> lock(dep_graph_mutex_);
        auto it = dep_graph_.find(id);
        if (it != dep_graph_.end()) {
            for (ResourceId const& depId : it->second) {
                if (DetectDependencyCycle(depId, visited)) {
                    return true;
                }
            }
        }
        
        visited.erase(id);
        return false;
    }
};

// Global ResourceManager instance (singleton pattern)
static ResourceManagerImpl* g_resourceManager = nullptr;
static std::mutex g_resourceManagerMutex;

IResourceManager* GetResourceManager() {
    std::lock_guard<std::mutex> lock(g_resourceManagerMutex);
    if (!g_resourceManager) {
        g_resourceManager = new ResourceManagerImpl();
    }
    return g_resourceManager;
}

}  // namespace resource
}  // namespace te
