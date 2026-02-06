// 013-Resource: ResourceManager implementation
#include "te/resource/ResourceManager.h"
#include "te/resource/Resource.h"
#include "te/resource/ResourceLoader.h"
#include "te/resource/ResourceSerializer.h"
#include "te/resource/ResourceImporter.h"
#include "te/resource/ResourceId.h"
#include "te/core/platform.h"
#include "te/core/thread.h"
#include <unordered_map>
#include <mutex>
#include <string>
#include <cstring>
#include <atomic>
#include <tuple>
#include <utility>
#include <vector>

namespace te {
namespace resource {

namespace {

// Build ResourceId from path (full 16-byte deterministic hash for cache key; 002::GUID layout compatible)
ResourceId ResourceIdFromPath(char const* path) {
    ResourceId id{};
    std::hash<std::string> H;
    std::string s(path ? path : "");
    size_t h0 = H(s);
    size_t h1 = H(s + "__te_013");
    for (size_t i = 0; i < 8; ++i) {
        id.data[i] = static_cast<uint8_t>((h0 >> (i * 8)) & 0xff);
        id.data[8 + i] = static_cast<uint8_t>((h1 >> (i * 8)) & 0xff);
    }
    return id;
}

struct AsyncLoadState {
    std::atomic<LoadStatus> status{LoadStatus::Pending};
    std::atomic<float> progress{0.f};
    std::atomic<bool> cancelled{false};
    std::string path;
    ResourceType type{};
    LoadCompleteCallback on_done{};
    void* user_data{};
};

class ResourceManagerImpl;

struct AsyncLoadParams {
    ResourceManagerImpl* manager{};
    LoadRequestId id{};
};

void AsyncLoadTask(void* raw);

class ResourceManagerImpl : public IResourceManager {
    friend void AsyncLoadTask(void*);
public:
    LoadRequestId RequestLoadAsync(char const* path, ResourceType type, LoadCompleteCallback on_done, void* user_data) override {
        if (!path || !on_done) return 0;
        LoadRequestId id = 0;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            id = next_request_id_++;
            request_states_.emplace(std::piecewise_construct, std::forward_as_tuple(id), std::forward_as_tuple());
            request_states_[id].path = path;
            request_states_[id].type = type;
            request_states_[id].on_done = on_done;
            request_states_[id].user_data = user_data;
        }
        AsyncLoadParams* p = new AsyncLoadParams{this, id};
        te::core::GetThreadPool()->SubmitTask(AsyncLoadTask, p);
        return id;
    }
    LoadStatus GetLoadStatus(LoadRequestId id) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = request_states_.find(id);
        return it != request_states_.end() ? it->second.status.load() : LoadStatus::Pending;
    }
    float GetLoadProgress(LoadRequestId id) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = request_states_.find(id);
        return it != request_states_.end() ? it->second.progress.load() : 0.f;
    }
    void CancelLoad(LoadRequestId id) override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = request_states_.find(id);
        if (it != request_states_.end()) it->second.cancelled.store(true);
    }

    IResource* LoadSync(char const* path, ResourceType type) override {
        if (!path) return nullptr;
        ResourceId id = ResourceIdFromPath(path);
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = cache_.find(id);
            if (it != cache_.end()) return it->second;
        }
        std::optional<std::vector<std::uint8_t>> data = te::core::FileRead(std::string(path));
        if (!data || data->empty()) return nullptr;
        IResourceSerializer* ser = nullptr;
        IResourceLoader* loader = nullptr;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto s = serializers_.find(type);
            auto l = loaders_.find(type);
            if (s != serializers_.end()) ser = s->second;
            if (l != loaders_.end()) loader = l->second;
        }
        if (!ser || !loader) return nullptr;
        void* payload = ser->Deserialize(data->data(), data->size());
        if (!payload) return nullptr;
        IResource* resource = loader->CreateFromPayload(type, payload, this);
        if (!resource) return nullptr;
        std::lock_guard<std::mutex> lock(mutex_);
        cache_[id] = resource;
        id_to_path_[id] = path;
        return resource;
    }

    IResource* GetCached(ResourceId id) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = cache_.find(id);
        return it != cache_.end() ? it->second : nullptr;
    }
    void Unload(IResource* resource) override {
        if (!resource) return;
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto it = cache_.begin(); it != cache_.end(); ++it) {
            if (it->second == resource) {
                id_to_path_.erase(it->first);
                resource->Release();
                cache_.erase(it);
                return;
            }
        }
    }

    char const* ResolvePath(ResourceId id) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = id_to_path_.find(id);
        return it != id_to_path_.end() ? it->second.c_str() : nullptr;
    }
    bool ResolvePathCopy(ResourceId id, char* out_buffer, size_t buffer_size) const override {
        if (!out_buffer || buffer_size == 0) return false;
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = id_to_path_.find(id);
        if (it == id_to_path_.end()) return false;
        size_t n = it->second.size();
        if (n >= buffer_size) n = buffer_size - 1;
        std::memcpy(out_buffer, it->second.c_str(), n);
        out_buffer[n] = '\0';
        return true;
    }

    void SetLoadCompleteDispatcher(LoadCompleteDispatcherFn fn) override {
        std::lock_guard<std::mutex> lock(mutex_);
        dispatcher_ = fn;
    }

    StreamingHandle RequestStreaming(ResourceId id, int priority) override {
        std::lock_guard<std::mutex> lock(mutex_);
        StreamingHandle h = next_streaming_handle_++;
        streaming_handles_[h] = std::make_pair(id, priority);
        return h;
    }
    void SetStreamingPriority(StreamingHandle h, int priority) override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = streaming_handles_.find(h);
        if (it != streaming_handles_.end()) it->second.second = priority;
    }

    void RegisterResourceLoader(ResourceType type, IResourceLoader* loader) override {
        std::lock_guard<std::mutex> lock(mutex_);
        loaders_[type] = loader;
    }
    void RegisterSerializer(ResourceType type, IResourceSerializer* serializer) override {
        std::lock_guard<std::mutex> lock(mutex_);
        serializers_[type] = serializer;
    }
    void RegisterImporter(ResourceType type, IResourceImporter* importer) override {
        std::lock_guard<std::mutex> lock(mutex_);
        importers_[type] = importer;
    }

    bool Import(char const* path, ResourceType type, void* out_metadata_or_null) override {
        if (!path) return false;
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = importers_.find(type);
        if (it == importers_.end()) return false;
        return it->second->Convert(path, out_metadata_or_null);
    }
    bool Save(IResource* resource, char const* path) override {
        if (!resource || !path) return false;
        ResourceType type = resource->GetResourceType();
        IResourceSerializer* ser = nullptr;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = serializers_.find(type);
            if (it == serializers_.end()) return false;
            ser = it->second;
        }
        const size_t kBufSize = 1024 * 1024;
        std::vector<std::uint8_t> buf(kBufSize);
        size_t written = 0;
        if (!ser->Serialize(resource, buf.data(), buf.size(), &written) || written == 0)
            return false;
        buf.resize(written);
        return te::core::FileWrite(std::string(path), buf);
    }

    std::unordered_map<ResourceId, IResource*> cache_;
    std::unordered_map<ResourceId, std::string> id_to_path_;
    std::unordered_map<ResourceType, IResourceLoader*> loaders_;
    std::unordered_map<ResourceType, IResourceSerializer*> serializers_;
    std::unordered_map<ResourceType, IResourceImporter*> importers_;
    LoadCompleteDispatcherFn dispatcher_{};
    std::unordered_map<LoadRequestId, AsyncLoadState> request_states_;
    LoadRequestId next_request_id_{1};
    std::unordered_map<StreamingHandle, std::pair<ResourceId, int>> streaming_handles_;
    StreamingHandle next_streaming_handle_{1};
    mutable std::mutex mutex_;
};

static void InvokeLoadComplete(ResourceManagerImpl* m, LoadCompleteCallback cb, IResource* r, LoadResult res, void* ud) {
    LoadCompleteDispatcherFn fn = nullptr;
    { std::lock_guard<std::mutex> lock(m->mutex_); fn = m->dispatcher_; }
    if (fn) fn(cb, r, res, ud);
    else cb(r, res, ud);
}

void AsyncLoadTask(void* raw) {
    AsyncLoadParams* p = static_cast<AsyncLoadParams*>(raw);
    ResourceManagerImpl* m = p->manager;
    LoadRequestId id = p->id;
    delete p;

    bool cancelled_early = false;
    LoadCompleteCallback early_cb = nullptr;
    void* early_ud = nullptr;
    AsyncLoadState* state = nullptr;
    {
        std::lock_guard<std::mutex> lock(m->mutex_);
        auto it = m->request_states_.find(id);
        if (it == m->request_states_.end()) return;
        state = &it->second;
        if (state->cancelled.load()) {
            state->status = LoadStatus::Cancelled;
            early_cb = state->on_done;
            early_ud = state->user_data;
            m->request_states_.erase(it);
            cancelled_early = true;
        } else {
            state->status = LoadStatus::Loading;
        }
    }
    if (cancelled_early) {
        InvokeLoadComplete(m, early_cb, nullptr, LoadResult::Cancelled, early_ud);
        return;
    }
    std::string path = state->path;
    ResourceType type = state->type;
    IResourceSerializer* ser = nullptr;
    IResourceLoader* loader = nullptr;
    {
        std::lock_guard<std::mutex> lock(m->mutex_);
        auto s = m->serializers_.find(type);
        auto l = m->loaders_.find(type);
        if (s != m->serializers_.end()) ser = s->second;
        if (l != m->loaders_.end()) loader = l->second;
    }
    if (!ser || !loader) {
        LoadCompleteCallback cb;
        void* ud;
        {
            std::lock_guard<std::mutex> lock(m->mutex_);
            state->status = LoadStatus::Failed;
            cb = state->on_done;
            ud = state->user_data;
            m->request_states_.erase(id);
        }
        InvokeLoadComplete(m, cb, nullptr, LoadResult::Error, ud);
        return;
    }
    state->progress = 0.3f;
    if (state->cancelled.load()) {
        LoadCompleteCallback cb; void* ud;
        { std::lock_guard<std::mutex> lock(m->mutex_); state->status = LoadStatus::Cancelled; cb = state->on_done; ud = state->user_data; m->request_states_.erase(id); }
        InvokeLoadComplete(m, cb, nullptr, LoadResult::Cancelled, ud);
        return;
    }
    std::optional<std::vector<std::uint8_t>> data = te::core::FileRead(path);
    state->progress = 0.7f;
    if (!data || data->empty()) {
        LoadCompleteCallback cb; void* ud;
        { std::lock_guard<std::mutex> lock(m->mutex_); state->status = LoadStatus::Failed; cb = state->on_done; ud = state->user_data; m->request_states_.erase(id); }
        InvokeLoadComplete(m, cb, nullptr, LoadResult::NotFound, ud);
        return;
    }
    if (state->cancelled.load()) {
        LoadCompleteCallback cb; void* ud;
        { std::lock_guard<std::mutex> lock(m->mutex_); state->status = LoadStatus::Cancelled; cb = state->on_done; ud = state->user_data; m->request_states_.erase(id); }
        InvokeLoadComplete(m, cb, nullptr, LoadResult::Cancelled, ud);
        return;
    }
    void* payload = ser->Deserialize(data->data(), data->size());
    if (!payload) {
        LoadCompleteCallback cb; void* ud;
        { std::lock_guard<std::mutex> lock(m->mutex_); state->status = LoadStatus::Failed; cb = state->on_done; ud = state->user_data; m->request_states_.erase(id); }
        InvokeLoadComplete(m, cb, nullptr, LoadResult::Error, ud);
        return;
    }
    IResource* resource = loader->CreateFromPayload(type, payload, m);
    if (!resource) {
        LoadCompleteCallback cb; void* ud;
        { std::lock_guard<std::mutex> lock(m->mutex_); state->status = LoadStatus::Failed; cb = state->on_done; ud = state->user_data; m->request_states_.erase(id); }
        InvokeLoadComplete(m, cb, nullptr, LoadResult::Error, ud);
        return;
    }
    ResourceId rid = ResourceIdFromPath(path.c_str());
    LoadCompleteCallback on_done = state->on_done;
    void* user_data = state->user_data;
    bool ok = false;
    {
        std::lock_guard<std::mutex> lock(m->mutex_);
        if (state->cancelled.load()) {
            resource->Release();
            state->status = LoadStatus::Cancelled;
            m->request_states_.erase(id);
            ok = false;
        } else {
            m->cache_[rid] = resource;
            m->id_to_path_[rid] = path;
            state->status = LoadStatus::Completed;
            state->progress = 1.f;
            m->request_states_.erase(id);
            ok = true;
        }
    }
    if (ok) InvokeLoadComplete(m, on_done, resource, LoadResult::Ok, user_data);
    else InvokeLoadComplete(m, on_done, nullptr, LoadResult::Cancelled, user_data);
}

} // namespace

IResourceManager* GetResourceManager() {
    static ResourceManagerImpl s_manager;
    return &s_manager;
}

} // namespace resource
} // namespace te
