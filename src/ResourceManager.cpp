/**
 * @file ResourceManager.cpp
 * @brief IResourceManager implementation and GetResourceManager (contract: 013-resource-ABI).
 */
#include <te/resource/ResourceManager.h>
#include <te/resource/ResourceLoader.h>
#include <te/resource/Deserializer.h>
#include <te/resource/ResourceImporter.h>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <thread>
#include <atomic>
#include <memory>
#include <cstdint>

namespace te {
namespace resource {

namespace {

/** Compute a deterministic ResourceId from path for cache/addressing (until 002 GUID integration). */
ResourceId IdFromPath(char const* path) {
  ResourceId id{};
  if (!path) return id;
  uint64_t h = 0;
  while (*path) { h = h * 31 + static_cast<unsigned char>(*path++); }
  id.data[0] = h;
  id.data[1] = 0;
  return id;
}

struct CacheEntry {
  IResource* resource = nullptr;
  int refcount = 0;
};

struct AsyncRequest {
  std::string path;
  ResourceType type = ResourceType::Texture;
  LoadCompleteCallback on_done = nullptr;
  void* user_data = nullptr;
  std::atomic<LoadStatus> status{LoadStatus::Pending};
  std::atomic<float> progress{0.f};
  std::atomic<bool> cancelled{false};
  IResource* result = nullptr;
  LoadResult load_result = LoadResult::Error;
};

class ResourceManagerImpl : public IResourceManager {
 public:
  LoadRequestId RequestLoadAsync(char const* path, ResourceType type,
                                LoadCompleteCallback on_done, void* user_data) override {
    if (!path || !on_done) return nullptr;
    auto req = std::make_shared<AsyncRequest>();
    req->path = path;
    req->type = type;
    req->on_done = on_done;
    req->user_data = user_data;
    req->status = LoadStatus::Pending;
    req->progress = 0.f;
    req->cancelled = false;
    {
      std::lock_guard<std::mutex> lock(requests_mutex_);
      requests_[req.get()] = req;
    }
    std::thread([this, req]() {
      if (req->cancelled) {
        req->status = LoadStatus::Cancelled;
        req->on_done(nullptr, LoadResult::Cancelled, req->user_data);
        return;
      }
      req->status = LoadStatus::Loading;
      req->progress = 0.f;
      IResource* res = LoadSync(req->path.c_str(), req->type);
      if (req->cancelled) {
        req->status = LoadStatus::Cancelled;
        req->on_done(nullptr, LoadResult::Cancelled, req->user_data);
        return;
      }
      if (res) {
        req->result = res;
        req->status = LoadStatus::Completed;
        req->load_result = LoadResult::Ok;
        req->progress = 1.f;
      } else {
        req->status = LoadStatus::Failed;
        req->load_result = LoadResult::Error;
      }
      req->on_done(req->result, req->load_result, req->user_data);
    }).detach();
    return req.get();
  }
  LoadStatus GetLoadStatus(LoadRequestId id) const override {
    if (!id) return LoadStatus::Pending;
    std::lock_guard<std::mutex> lock(requests_mutex_);
    auto it = requests_.find(static_cast<AsyncRequest*>(id));
    return it != requests_.end() ? it->second->status.load() : LoadStatus::Pending;
  }
  float GetLoadProgress(LoadRequestId id) const override {
    if (!id) return 0.f;
    std::lock_guard<std::mutex> lock(requests_mutex_);
    auto it = requests_.find(static_cast<AsyncRequest*>(id));
    return it != requests_.end() ? it->second->progress.load() : 0.f;
  }
  void CancelLoad(LoadRequestId id) override {
    if (!id) return;
    std::lock_guard<std::mutex> lock(requests_mutex_);
    auto it = requests_.find(static_cast<AsyncRequest*>(id));
    if (it != requests_.end()) it->second->cancelled = true;
  }

  IResource* GetCached(ResourceId id) const override {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = cache_.find(id);
    if (it == cache_.end() || it->second.refcount <= 0) return nullptr;
    it->second.refcount++;
    return it->second.resource;
  }

  IResource* LoadSync(char const* path, ResourceType type) override {
    if (!path) return nullptr;
    // T017: cycle detection - same path re-entered during CreateFromPayload dependency load
    static thread_local std::set<std::string> loading_paths;
    if (loading_paths.count(path)) return nullptr;  // cycle → LoadResult::Error equivalent
    struct LoadingGuard { std::set<std::string>& s; std::string p; LoadingGuard(std::set<std::string>& set, char const* path) : s(set), p(path) { s.insert(p); } ~LoadingGuard() { s.erase(p); } };
    LoadingGuard load_guard(loading_paths, path);
    ResourceId id = IdFromPath(path);
    {
      std::lock_guard<std::mutex> lock(mutex_);
      auto it = cache_.find(id);
      if (it != cache_.end() && it->second.refcount > 0) {
        it->second.refcount++;
        return it->second.resource;
      }
    }
    IResourceLoader* loader = nullptr;
    IDeserializer* deserializer = nullptr;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      auto lit = loaders_.find(type);
      auto dit = deserializers_.find(type);
      if (lit == loaders_.end() || dit == deserializers_.end()) return nullptr;
      loader = lit->second;
      deserializer = dit->second;
    }
    std::vector<char> buffer;
    {
      std::ifstream f(path, std::ios::binary | std::ios::ate);
      if (!f) return nullptr;
      buffer.resize(static_cast<size_t>(f.tellg()));
      f.seekg(0);
      if (!buffer.empty() && !f.read(buffer.data(), static_cast<std::streamsize>(buffer.size())))
        return nullptr;
    }
    void* payload = buffer.empty() ? nullptr : deserializer->Deserialize(buffer.data(), buffer.size());
    if (!payload && !buffer.empty()) return nullptr;
    IResource* resource = loader->CreateFromPayload(type, payload, this);
    if (!resource) return nullptr;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      cache_[id] = CacheEntry{resource, 1};
      resource_to_id_[resource] = id;
      id_to_path_[id] = path;
    }
    return resource;
  }

  void Unload(IResource* resource) override {
    if (!resource) return;
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = resource_to_id_.find(resource);
    if (it == resource_to_id_.end()) return;
    ResourceId id = it->second;
    auto cit = cache_.find(id);
    if (cit != cache_.end()) {
      cit->second.refcount--;
      if (cit->second.refcount <= 0) {
        cache_.erase(cit);
        resource_to_id_.erase(it);
      }
    }
  }

  StreamingHandle RequestStreaming(ResourceId id, int priority) override {
    (void)id; (void)priority;
    return nullptr;  // TODO T023
  }
  void SetStreamingPriority(StreamingHandle h, int priority) override {
    (void)h; (void)priority;
  }

  void RegisterResourceLoader(ResourceType type, IResourceLoader* loader) override {
    std::lock_guard<std::mutex> lock(mutex_);
    loaders_[type] = loader;
  }
  void RegisterDeserializer(ResourceType type, IDeserializer* deserializer) override {
    std::lock_guard<std::mutex> lock(mutex_);
    deserializers_[type] = deserializer;
  }
  void RegisterImporter(ResourceType type, IResourceImporter* importer) override {
    std::lock_guard<std::mutex> lock(mutex_);
    importers_[type] = importer;
  }

  bool Import(char const* path, ResourceType type, void* out_metadata_or_null) override {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = importers_.find(type);
    if (it == importers_.end() || !it->second) return false;
    return it->second->Convert(path, nullptr, out_metadata_or_null);
  }
  bool Save(IResource* resource, char const* path) override {
    if (!resource || !path) return false;
    ResourceType type = resource->GetResourceType();
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = savers_.find(type);
    if (it == savers_.end() || !it->second) return false;
    return it->second(resource, path);  // T027: module produces content; saver or 013 writes
  }
  void RegisterSaver(ResourceType type, SaverFn fn) override {
    std::lock_guard<std::mutex> lock(mutex_);
    savers_[type] = fn;
  }

  char const* ResolvePath(ResourceId id) const override {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = id_to_path_.find(id);
    return it != id_to_path_.end() ? it->second.c_str() : nullptr;
  }

 private:
  mutable std::mutex mutex_;
  mutable std::mutex requests_mutex_;
  mutable std::map<ResourceId, CacheEntry> cache_;
  std::unordered_map<IResource*, ResourceId> resource_to_id_;
  mutable std::map<ResourceId, std::string> id_to_path_;
  std::map<ResourceType, IResourceLoader*> loaders_;
  std::map<ResourceType, IDeserializer*> deserializers_;
  std::map<ResourceType, IResourceImporter*> importers_;
  std::map<ResourceType, IResourceManager::SaverFn> savers_;
  std::map<LoadRequestId, std::shared_ptr<AsyncRequest>> requests_;
};

}  // namespace

IResourceManager* GetResourceManager() {
  static ResourceManagerImpl s_instance;
  return &s_instance;
}

}  // namespace resource
}  // namespace te
