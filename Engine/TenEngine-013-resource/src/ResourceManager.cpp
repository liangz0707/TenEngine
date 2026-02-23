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
#include <te/resource/ResourceId.h>
#include <te/resource/ResourceRepositoryConfig.h>
#include <te/resource/ResourceManifest.h>
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
#include <filesystem>
#include <chrono>
#include <sstream>
#include <map>

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

/** Type directory name under repository (e.g. texture, mesh, material). Extensible. */
char const* GetTypeDirectory(ResourceType type) {
    switch (type) {
        case ResourceType::Texture: return "texture";
        case ResourceType::Mesh: return "mesh";
        case ResourceType::Material: return "material";
        default: return "";
    }
}

/** Primary file extension for the type (e.g. .texture, .mesh). Extensible. */
char const* GetPrimaryExtension(ResourceType type) {
    switch (type) {
        case ResourceType::Texture: return ".texture";
        case ResourceType::Mesh: return ".mesh";
        case ResourceType::Material: return ".material";
        default: return "";
    }
}

/** Compute storage path relative to asset root: repo/typeDir/guidStr/displayName.ext. */
std::string ComputeStoragePath(char const* repo, ResourceType type, ResourceId const& guid, char const* displayName) {
    char const* typeDir = GetTypeDirectory(type);
    char const* ext = GetPrimaryExtension(type);
    if (!typeDir || !typeDir[0] || !ext || !ext[0]) return "";
    std::string guidStr = guid.ToString();
    std::string primaryName = std::string(displayName ? displayName : "") + ext;
    return te::core::PathJoin(te::core::PathJoin(te::core::PathJoin(repo, typeDir), guidStr), primaryName);
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
        std::lock_guard<std::mutex> lock(requests_mutex_);
        te::core::IThreadPool* pool = te::core::GetThreadPool();
        te::core::ITaskExecutor* ioEx = pool ? pool->GetIOExecutor() : nullptr;
        for (auto& pair : requests_) {
            auto& request = pair.second;
            if (request->status.load() == LoadStatus::Loading ||
                request->status.load() == LoadStatus::Pending) {
                request->cancelled.store(true);
                if (ioEx && request->task_id != 0) {
                    ioEx->CancelTask(request->task_id);
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
        id_to_type_.clear();
        id_to_repo_.clear();
    }

    void SetAssetRoot(char const* path) override {
        asset_root_ = path ? path : "";
    }

    void LoadAllManifests() override {
        if (asset_root_.empty()) return;
        {
            std::lock_guard<std::mutex> lock(manifest_mutex_);
            manifests_.clear();
            repo_config_.repositories.clear();
        }
        {
            std::lock_guard<std::mutex> lock(cache_mutex_);
            id_to_path_.clear();
            id_to_type_.clear();
            id_to_repo_.clear();
        }
        {
            std::lock_guard<std::mutex> lock(manifest_mutex_);
            if (!LoadRepositoryConfig(asset_root_.c_str(), repo_config_)) {
                RepositoryInfo def;
                def.name = "main";
                def.root = "main";
                def.virtualPrefix = "Assets";
                repo_config_.repositories.push_back(def);
                SaveRepositoryConfig(asset_root_.c_str(), repo_config_);
                std::string mainDir = te::core::PathJoin(asset_root_, "main");
                std::error_code ec;
                std::filesystem::create_directories(std::filesystem::u8path(mainDir), ec);
            }
        }
        {
            std::lock_guard<std::mutex> lock(manifest_mutex_);
            for (RepositoryInfo const& repo : repo_config_.repositories) {
                std::string manifestPath = te::core::PathJoin(te::core::PathJoin(asset_root_, repo.root), "manifest.json");
                ResourceManifest mf;
                if (LoadManifest(manifestPath.c_str(), mf)) {
                    manifests_[repo.name] = mf;
                    std::lock_guard<std::mutex> clock(cache_mutex_);
                    for (ManifestEntry const& e : mf.resources) {
                        std::string relPath = ComputeStoragePath(e.repository.c_str(), e.type, e.guid, e.displayName.c_str());
                        if (!relPath.empty())
                            id_to_path_[e.guid] = te::core::PathJoin(asset_root_, relPath);
                        id_to_type_[e.guid] = e.type;
                        id_to_repo_[e.guid] = e.repository;
                    }
                } else {
                    manifests_[repo.name] = ResourceManifest();
                }
            }
        }
    }

    ResourceType ResolveType(ResourceId id) const override {
        if (id.IsNull()) return ResourceType::_Count;
        std::lock_guard<std::mutex> lock(cache_mutex_);
        auto it = id_to_type_.find(id);
        return it == id_to_type_.end() ? ResourceType::_Count : it->second;
    }

    IResource* LoadSyncByGuid(ResourceId id) override {
        char const* path = ResolvePath(id);
        if (!path) return nullptr;
        ResourceType type = ResolveType(id);
        if (type == ResourceType::_Count) return nullptr;
        return LoadSync(path, type);
    }

    bool ImportIntoRepository(char const* sourcePath, ResourceType type,
                              char const* repositoryName, char const* parentAssetPath,
                              void* out_metadata_or_null) override {
        if (!sourcePath || !repositoryName) return false;
        std::string repoName(repositoryName);
        std::string parentAP = parentAssetPath ? parentAssetPath : "";
        char const* typeDir = GetTypeDirectory(type);
        if (!typeDir || !typeDir[0]) return false;
        std::string repoRootDir = te::core::PathJoin(asset_root_, GetRepoRoot(repoName.c_str()));
        std::string typeRoot = te::core::PathJoin(repoRootDir, typeDir);
        uint64_t tick = static_cast<uint64_t>(std::chrono::steady_clock::now().time_since_epoch().count());
        std::string tempDir = te::core::PathJoin(typeRoot, "import_" + std::to_string(tick));
        std::error_code ec;
        std::filesystem::create_directories(std::filesystem::u8path(tempDir), ec);
        if (ec) return false;
        std::string sourceFileName = te::core::PathGetFileName(sourcePath);
        std::string destSourcePath = te::core::PathJoin(tempDir, sourceFileName);
        auto fileData = te::core::FileRead(sourcePath);
        if (!fileData || fileData->empty()) return false;
        if (!te::core::FileWrite(destSourcePath, std::vector<std::uint8_t>(fileData->begin(), fileData->end()))) return false;
        IResource* resource = CreateResourceInstance(type);
        if (!resource) return false;
        bool ok = resource->Import(destSourcePath.c_str(), this);
        if (!ok) {
            resource->Release();
            std::filesystem::remove_all(std::filesystem::u8path(tempDir), ec);
            return false;
        }
        ResourceId id = resource->GetResourceId();
        std::string guidStr = id.ToString();
        std::string finalDir = te::core::PathJoin(typeRoot, guidStr);
        std::filesystem::rename(std::filesystem::u8path(tempDir), std::filesystem::u8path(finalDir), ec);
        if (ec) {
            resource->Release();
            return false;
        }
        size_t dot = sourceFileName.find_last_of('.');
        std::string displayName = dot != std::string::npos ? sourceFileName.substr(0, dot) : sourceFileName;
        // assetPath = folder only (where the resource lives); displayName = file name in that folder
        std::string assetPath = parentAP;
        std::string relPath = ComputeStoragePath(repoName.c_str(), type, id, displayName.c_str());
        std::string fullPrimaryPath = relPath.empty() ? "" : te::core::PathJoin(asset_root_, relPath);
        if (relPath.empty()) {
            resource->Release();
            return false;
        }
        {
            std::lock_guard<std::mutex> lock(manifest_mutex_);
            ManifestEntry e;
            e.guid = id;
            e.assetPath = assetPath;
            e.type = type;
            e.repository = repoName;
            e.displayName = displayName;
            auto it = manifests_.find(repoName);
            if (it != manifests_.end()) {
                it->second.resources.push_back(e);
            } else {
                ResourceManifest mf;
                mf.resources.push_back(e);
                manifests_[repoName] = mf;
            }
            {
                std::lock_guard<std::mutex> clock(cache_mutex_);
                id_to_path_[id] = fullPrimaryPath;
                id_to_type_[id] = type;
                id_to_repo_[id] = repoName;
            }
        }
        CacheResource(id, resource, fullPrimaryPath.c_str());
        {
            std::lock_guard<std::mutex> lock(manifest_mutex_);
            std::string manifestPath = te::core::PathJoin(repoRootDir, "manifest.json");
            SaveManifest(manifestPath.c_str(), manifests_[repoName]);
        }
        return true;
    }

    bool CreateRepository(char const* name) override {
        return AddRepository(asset_root_.c_str(), name);
    }

    void GetRepositoryList(std::vector<std::string>& out) const override {
        out.clear();
        std::lock_guard<std::mutex> lock(manifest_mutex_);
        for (auto const& r : repo_config_.repositories)
            out.push_back(r.name);
    }

    void GetResourceInfos(std::vector<ResourceInfo>& out) const override {
        out.clear();
        std::lock_guard<std::mutex> lock(manifest_mutex_);
        for (auto const& kv : manifests_) {
            for (ManifestEntry const& e : kv.second.resources) {
                ResourceInfo info;
                info.guid = e.guid;
                info.assetPath = e.assetPath;
                info.type = e.type;
                info.repository = e.repository;
                info.displayName = e.displayName;
                out.push_back(info);
            }
        }
    }

    void GetAssetFolders(std::vector<std::string>& out) const override {
        out.clear();
        std::lock_guard<std::mutex> lock(manifest_mutex_);
        for (auto const& kv : manifests_) {
            for (std::string const& folder : kv.second.assetFolders)
                out.push_back(folder);
        }
    }

    void GetAssetFoldersForRepository(char const* repositoryName, std::vector<std::string>& out) const override {
        out.clear();
        if (!repositoryName) return;
        std::lock_guard<std::mutex> lock(manifest_mutex_);
        auto it = manifests_.find(repositoryName);
        if (it != manifests_.end())
            out = it->second.assetFolders;
    }

    bool MoveResourceToRepository(ResourceId id, char const* targetRepository) override {
        if (id.IsNull() || !targetRepository) return false;
        std::lock_guard<std::mutex> lock(manifest_mutex_);
        auto pathIt = id_to_path_.find(id);
        auto typeIt = id_to_type_.find(id);
        auto repoIt = id_to_repo_.find(id);
        if (pathIt == id_to_path_.end() || typeIt == id_to_type_.end() || repoIt == id_to_repo_.end()) return false;
        std::string srcRepo = repoIt->second;
        if (srcRepo == targetRepository) return true;
        std::string fullPath = pathIt->second;
        ResourceType type = typeIt->second;
        char const* typeDir = GetTypeDirectory(type);
        if (!typeDir || !typeDir[0]) return false;
        std::string guidStr = id.ToString();
        std::string srcDir = te::core::PathGetDirectory(fullPath);
        std::string destRoot = te::core::PathJoin(asset_root_, GetRepoRoot(targetRepository));
        std::string destTypeRoot = te::core::PathJoin(destRoot, typeDir);
        std::string destDir = te::core::PathJoin(destTypeRoot, guidStr);
        std::error_code ec;
        std::filesystem::path destDirP = std::filesystem::u8path(destDir);
        std::filesystem::create_directories(destDirP, ec);
        if (ec) return false;
        std::filesystem::path srcDirP = std::filesystem::u8path(srcDir);
        for (auto const& entry : std::filesystem::directory_iterator(srcDirP)) {
            std::filesystem::copy(entry.path(), destDirP / entry.path().filename(), std::filesystem::copy_options::overwrite_existing, ec);
            if (ec) return false;
        }
        auto srcManifestIt = manifests_.find(srcRepo);
        auto destManifestIt = manifests_.find(targetRepository);
        if (srcManifestIt == manifests_.end() || destManifestIt == manifests_.end()) return false;
        ManifestEntry newEntry;
        newEntry.guid = id;
        newEntry.type = type;
        newEntry.repository = targetRepository;
        newEntry.displayName = "";
        newEntry.assetPath = "";
        for (ManifestEntry const& e : srcManifestIt->second.resources) {
            if (e.guid == id) {
                newEntry.assetPath = e.assetPath;
                newEntry.displayName = e.displayName;
                break;
            }
        }
        std::string newRelPath = ComputeStoragePath(targetRepository, type, id, newEntry.displayName.c_str());
        if (newRelPath.empty()) return false;
        auto& srcRes = srcManifestIt->second.resources;
        srcRes.erase(std::remove_if(srcRes.begin(), srcRes.end(), [id](ManifestEntry const& e) { return e.guid == id; }), srcRes.end());
        destManifestIt->second.resources.push_back(newEntry);
        {
            std::lock_guard<std::mutex> clock(cache_mutex_);
            id_to_path_[id] = te::core::PathJoin(asset_root_, newRelPath);
            id_to_repo_[id] = targetRepository;
        }
        std::string srcRepoRoot = GetRepoRoot(srcRepo.c_str());
        std::string dstRepoRoot = GetRepoRoot(targetRepository);
        std::string srcManifestPath = te::core::PathJoin(te::core::PathJoin(asset_root_, srcRepoRoot), "manifest.json");
        std::string dstManifestPath = te::core::PathJoin(te::core::PathJoin(asset_root_, dstRepoRoot), "manifest.json");
        SaveManifest(srcManifestPath.c_str(), srcManifestIt->second);
        SaveManifest(dstManifestPath.c_str(), destManifestIt->second);
        std::filesystem::remove_all(std::filesystem::u8path(srcDir), ec);
        (void)ec;
        return true;
    }

    bool UpdateAssetPath(ResourceId id, char const* newAssetPath) override {
        if (id.IsNull()) return false;
        std::string newPath(newAssetPath ? newAssetPath : "");
        std::lock_guard<std::mutex> lock(manifest_mutex_);
        auto repoIt = id_to_repo_.find(id);
        if (repoIt == id_to_repo_.end()) return false;
        std::string repoRootDir = GetRepoRoot(repoIt->second.c_str());
        std::string manifestPathOnDisk = te::core::PathJoin(te::core::PathJoin(asset_root_, repoRootDir), "manifest.json");
        ResourceManifest mf;
        if (!LoadManifest(manifestPathOnDisk.c_str(), mf)) return false;
        bool found = false;
        for (ManifestEntry& e : mf.resources) {
            if (e.guid == id) {
                e.assetPath = newPath;
                found = true;
                break;
            }
        }
        if (!found) return false;
        bool ok = SaveManifest(manifestPathOnDisk.c_str(), mf);
        if (ok) manifests_[repoIt->second] = std::move(mf);
        return ok;
    }

    bool MoveAssetFolder(char const* assetPath, char const* newParentAssetPath) override {
        if (!assetPath) return false;
        std::string prefix = assetPath;
        if (!prefix.empty() && prefix.back() != '/') prefix += "/";
        std::string newParent(newParentAssetPath ? newParentAssetPath : "");
        std::string movedName = te::core::PathGetFileName(assetPath);
        if (movedName.empty()) return false;
        std::string baseNewPath = newParent.empty() ? movedName : (newParent + "/" + movedName);
        std::lock_guard<std::mutex> lock(manifest_mutex_);
        bool any = false;
        for (auto& kv : manifests_) {
            std::string manifestPathOnDisk = te::core::PathJoin(te::core::PathJoin(asset_root_, GetRepoRoot(kv.first.c_str())), "manifest.json");
            ResourceManifest mf;
            if (!LoadManifest(manifestPathOnDisk.c_str(), mf)) continue;
            bool modified = false;
            for (ManifestEntry& e : mf.resources) {
                if (e.assetPath == assetPath || (e.assetPath.size() > prefix.size() && e.assetPath.compare(0, prefix.size(), prefix) == 0)) {
                    std::string suffix = e.assetPath.size() > prefix.size() ? e.assetPath.substr(prefix.size()) : "";
                    e.assetPath = baseNewPath + (suffix.empty() ? "" : "/" + suffix);
                    modified = true;
                    any = true;
                }
            }
            std::vector<std::string> newAssetFolders;
            for (std::string const& folder : mf.assetFolders) {
                if (folder == assetPath || (prefix.size() > 0 && folder.size() >= prefix.size() && folder.compare(0, prefix.size(), prefix) == 0)) {
                    std::string suffix = (folder == assetPath) ? "" : folder.substr(prefix.size());
                    std::string np = baseNewPath + (suffix.empty() ? "" : "/" + suffix);
                    newAssetFolders.push_back(np);
                    modified = true;
                    any = true;
                } else {
                    newAssetFolders.push_back(folder);
                }
            }
            if (modified) {
                mf.assetFolders = std::move(newAssetFolders);
                if (SaveManifest(manifestPathOnDisk.c_str(), mf))
                    kv.second = std::move(mf);
            }
        }
        return any;
    }

    bool AddAssetFolder(char const* repositoryName, char const* assetPath) override {
        if (!repositoryName || !assetPath || !assetPath[0]) return false;
        std::lock_guard<std::mutex> lock(manifest_mutex_);
        std::string repoRootDir = GetRepoRoot(repositoryName);
        std::string manifestPathOnDisk = te::core::PathJoin(te::core::PathJoin(asset_root_, repoRootDir), "manifest.json");
        ResourceManifest mf;
        if (!LoadManifest(manifestPathOnDisk.c_str(), mf)) {
            mf = ResourceManifest();
        }
        std::string ap(assetPath);
        for (std::string const& s : mf.assetFolders)
            if (s == ap) return true;
        mf.assetFolders.push_back(ap);
        bool ok = SaveManifest(manifestPathOnDisk.c_str(), mf);
        if (ok)
            manifests_[repositoryName] = std::move(mf);
        return ok;
    }

    bool RemoveAssetFolder(char const* repositoryName, char const* assetPath) override {
        if (!repositoryName || !repositoryName[0] || !assetPath) return false;
        std::string prefix = assetPath;
        if (!prefix.empty() && prefix.back() != '/') prefix += "/";
        std::string parentPath = te::core::PathGetDirectory(assetPath);
        std::lock_guard<std::mutex> lock(manifest_mutex_);
        std::string repoRootDir = GetRepoRoot(repositoryName);
        if (repoRootDir.empty()) return false;
        std::string manifestPathOnDisk = te::core::PathJoin(te::core::PathJoin(asset_root_, repoRootDir), "manifest.json");
        ResourceManifest mf;
        if (!LoadManifest(manifestPathOnDisk.c_str(), mf)) return false;
        bool modified = false;
        for (ManifestEntry& e : mf.resources) {
            if (e.assetPath == assetPath || (prefix.size() > 0 && e.assetPath.size() >= prefix.size() && e.assetPath.compare(0, prefix.size(), prefix) == 0)) {
                e.assetPath = parentPath;
                modified = true;
            }
        }
        std::vector<std::string> newAssetFolders;
        for (std::string const& folder : mf.assetFolders) {
            if (folder == assetPath || (prefix.size() > 0 && folder.size() >= prefix.size() && folder.compare(0, prefix.size(), prefix) == 0))
                modified = true;
            else
                newAssetFolders.push_back(folder);
        }
        if (!modified) return false;
        mf.assetFolders = std::move(newAssetFolders);
        bool ok = SaveManifest(manifestPathOnDisk.c_str(), mf);
        if (ok) manifests_[repositoryName] = std::move(mf);
        return ok;
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
        
        te::core::IThreadPool* threadPool = te::core::GetThreadPool();
        te::core::ITaskExecutor* ioExecutor = threadPool ? threadPool->GetIOExecutor() : nullptr;
        if (!threadPool || !ioExecutor) {
            request->status.store(LoadStatus::Failed);
            if (on_done) {
                on_done(nullptr, LoadResult::Error, user_data);
            }
            std::lock_guard<std::mutex> lock(requests_mutex_);
            requests_.erase(id);
            return nullptr;
        }

        request->status.store(LoadStatus::Loading);

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

        static auto LoadTaskCallback = [](void* user_data) {
            std::unique_ptr<std::shared_ptr<LoadTaskContext>> owned(
                static_cast<std::shared_ptr<LoadTaskContext>*>(user_data));
            std::shared_ptr<LoadTaskContext> ctx = owned ? *owned : nullptr;
            if (!ctx) return;

            LoadResult result = LoadResult::Error;
            IResource* resource = nullptr;

            if (ctx->request->cancelled.load()) {
                result = LoadResult::Cancelled;
            } else {
                resource = ctx->manager->CreateResourceInstance(ctx->type);
                if (resource) {
                    bool success = resource->Load(ctx->path.c_str(), ctx->manager);
                    if (success) {
                        result = LoadResult::Ok;
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

            ctx->request->result = resource;
            ctx->request->load_result = result;
            ctx->request->status.store(result == LoadResult::Cancelled ? LoadStatus::Cancelled :
                                     (result == LoadResult::Ok ? LoadStatus::Completed : LoadStatus::Failed));

            if (ctx->request->on_done && !ctx->request->cancelled.load()) {
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

                    struct CallbackHolder {
                        std::shared_ptr<CallbackContext> ctx;
                        static void Wrapper(void* user_data) {
                            auto* h = static_cast<CallbackHolder*>(user_data);
                            if (h->ctx) {
                                h->ctx->callback(h->ctx->result, h->ctx->load_result, h->ctx->user_data);
                            }
                            delete h;
                        }
                    };
                    auto* holder = new CallbackHolder{std::move(cbCtx)};
                    pool->SubmitTask(CallbackHolder::Wrapper, holder);
                }
            }
        };

        std::shared_ptr<LoadTaskContext>* contextPtr = new std::shared_ptr<LoadTaskContext>(context);
        te::core::TaskId taskId = ioExecutor->SubmitTaskWithPriority(
            LoadTaskCallback,
            contextPtr,
            0
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
        
        if (request->task_id != 0) {
            te::core::IThreadPool* pool = te::core::GetThreadPool();
            te::core::ITaskExecutor* ioEx = pool ? pool->GetIOExecutor() : nullptr;
            if (ioEx) {
                ioEx->CancelTask(request->task_id);
            }
        }

        request->status.store(LoadStatus::Cancelled);

        if (request->on_done) {
            te::core::IThreadPool* pool = te::core::GetThreadPool();
            if (pool) {
                struct CancelCallbackContext {
                    LoadCompleteCallback callback;
                    IResource* result;
                    void* user_data;
                };
                auto cbCtx = std::make_shared<CancelCallbackContext>();
                cbCtx->callback = request->on_done;
                cbCtx->result = request->result;
                cbCtx->user_data = request->user_data;

                struct CancelCallbackHolder {
                    std::shared_ptr<CancelCallbackContext> ctx;
                    static void Wrapper(void* user_data) {
                        auto* h = static_cast<CancelCallbackHolder*>(user_data);
                        if (h->ctx) {
                            h->ctx->callback(h->ctx->result, LoadResult::Cancelled, h->ctx->user_data);
                        }
                        delete h;
                    }
                };
                auto* holder = new CancelCallbackHolder{std::move(cbCtx)};
                pool->SubmitTask(CancelCallbackHolder::Wrapper, holder);
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
        std::lock_guard<std::mutex> lock(streaming_mutex_);
        uintptr_t h = next_streaming_handle_++;
        streaming_requests_[h] = { id, priority };
        return reinterpret_cast<StreamingHandle>(h);
    }

    void SetStreamingPriority(StreamingHandle h, int priority) override {
        if (!h) return;
        std::lock_guard<std::mutex> lock(streaming_mutex_);
        auto it = streaming_requests_.find(reinterpret_cast<uintptr_t>(h));
        if (it != streaming_requests_.end())
            it->second.priority = priority;
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

    // === Extended interface implementations (stubs) ===

    LoadRequestId RequestLoadAsyncEx(char const* path, ResourceType type,
                                     LoadCompleteCallback on_done,
                                     LoadOptions const& options) override {
        // For now, delegate to basic RequestLoadAsync
        // TODO: Honor LoadOptions (priority, callback thread, etc.)
        (void)options;
        return RequestLoadAsync(path, type, on_done, options.user_data);
    }

    BatchLoadRequestId RequestLoadBatchAsync(
        LoadRequestInfo const* requests, std::size_t count,
        BatchLoadCompleteCallback on_done, void* user_data,
        LoadOptions const& options) override {
        // Stub implementation - load each resource individually
        if (!requests || count == 0) {
            if (on_done) {
                BatchLoadResult result;
                on_done(nullptr, &result, user_data);
            }
            return nullptr;
        }

        // TODO: Implement proper batch loading with progress tracking
        // For now, just invoke callback with placeholder result
        if (on_done) {
            BatchLoadResult result;
            result.totalCount = static_cast<std::size_t>(count);
            result.successCount = 0;
            result.failedCount = static_cast<std::size_t>(count);
            result.cancelledCount = 0;
            result.overallResult = LoadResult::Error;
            on_done(nullptr, &result, user_data);
        }
        return nullptr;
    }

    bool GetBatchLoadResult(BatchLoadRequestId id, BatchLoadResult& out_result) const override {
        (void)id;
        (void)out_result;
        return false;
    }

    void CancelBatchLoad(BatchLoadRequestId id) override {
        (void)id;
    }

    RecursiveLoadState GetRecursiveLoadState(ResourceId id) const override {
        (void)id;
        return RecursiveLoadState::NotLoaded;
    }

    LoadRequestId PreloadDependencies(ResourceId id, LoadCompleteCallback on_done, void* user_data) override {
        (void)id;
        if (on_done) {
            on_done(nullptr, LoadResult::Ok, user_data);
        }
        return nullptr;
    }

    bool GetDependencyTree(ResourceId id, std::vector<ResourceId>& out_deps,
                           std::size_t max_depth) const override {
        (void)id;
        (void)max_depth;
        out_deps.clear();
        return true;
    }

    std::size_t GetTotalMemoryUsage() const override {
        return 0;
    }

    std::size_t GetResourceMemoryUsage(ResourceId id) const override {
        (void)id;
        return 0;
    }

    void SetMemoryBudget(std::size_t budget) override {
        (void)budget;
    }

    std::size_t GetMemoryBudget() const override {
        return 0;
    }

    std::size_t ForceGarbageCollect() override {
        return 0;
    }

    RecursiveLoadState GetRecursiveLoadStateByRequestId(LoadRequestId id) const override {
        (void)id;
        return RecursiveLoadState::NotLoaded;
    }

    bool IsResourceReady(ResourceId id) const override {
        (void)id;
        return false;
    }

    bool IsResourceReadyByRequestId(LoadRequestId id) const override {
        (void)id;
        return false;
    }

    void* SubscribeResourceState(ResourceId id,
                                  ResourceStateCallback callback,
                                  void* user_data) override {
        (void)id;
        (void)callback;
        (void)user_data;
        return nullptr;
    }

    void* SubscribeGlobalResourceState(ResourceStateCallback callback,
                                        void* user_data) override {
        (void)callback;
        (void)user_data;
        return nullptr;
    }

    void UnsubscribeResourceState(void* subscription_handle) override {
        (void)subscription_handle;
    }

private:
    // Resource cache
    mutable std::mutex cache_mutex_;
    std::unordered_map<ResourceId, CacheEntry> cache_;
    std::unordered_map<IResource*, ResourceId> resource_to_id_;
    std::unordered_map<ResourceId, std::string> id_to_path_;
    std::unordered_map<ResourceId, ResourceType> id_to_type_;
    std::unordered_map<ResourceId, std::string> id_to_repo_;

    // Repository and manifest (asset root, config, per-repo manifests)
    mutable std::mutex manifest_mutex_;
    std::string asset_root_;
    RepositoryConfig repo_config_;
    std::map<std::string, ResourceManifest> manifests_;
    
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

    // Streaming requests (id + priority; actual load-by-priority can be wired later)
    struct StreamingEntry { ResourceId id; int priority; };
    mutable std::mutex streaming_mutex_;
    std::unordered_map<uintptr_t, StreamingEntry> streaming_requests_;
    uintptr_t next_streaming_handle_ = 1;
    
    /** Get repository root directory (disk path) for a repository name. Uses repo_config_; falls back to name if not found. */
    std::string GetRepoRoot(char const* repositoryName) const {
        if (!repositoryName || !repositoryName[0]) return std::string();
        for (RepositoryInfo const& r : repo_config_.repositories)
            if (r.name == repositoryName) return r.root.empty() ? r.name : r.root;
        return std::string(repositoryName);
    }

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
