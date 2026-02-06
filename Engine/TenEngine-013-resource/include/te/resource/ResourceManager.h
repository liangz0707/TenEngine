// 013-Resource: IResourceManager and related types per ABI (te/resource/ResourceManager.h)
#pragma once

#include "te/resource/ResourceId.h"
#include "te/resource/ResourceTypes.h"
#include "te/resource/Resource.h"
#include <cstddef>

namespace te {
namespace resource {

class IResourceLoader;
class IResourceSerializer;
class IResourceImporter;

// --- Load request and status ---
using LoadRequestId = size_t;
enum class LoadStatus { Pending, Loading, Completed, Failed, Cancelled };
enum class LoadResult { Ok, NotFound, Error, Cancelled };
using LoadCompleteCallback = void (*)(IResource* resource, LoadResult result, void* user_data);
// 若已 SetLoadCompleteDispatcher，013 在加载完成后调用 Dispatcher 而非直接调用 callback
using LoadCompleteDispatcherFn = void (*)(LoadCompleteCallback cb, IResource* r, LoadResult res, void* user_data);

// --- Streaming ---
using StreamingHandle = size_t;

class IResourceManager {
public:
    virtual ~IResourceManager() = default;

    // Async load (primary entry)
    virtual LoadRequestId RequestLoadAsync(char const* path, ResourceType type, LoadCompleteCallback on_done, void* user_data) = 0;
    virtual LoadStatus GetLoadStatus(LoadRequestId id) const = 0;
    virtual float GetLoadProgress(LoadRequestId id) const = 0;
    virtual void CancelLoad(LoadRequestId id) = 0;

    // Sync load
    virtual IResource* LoadSync(char const* path, ResourceType type) = 0;

    // Cache and lifecycle
    virtual IResource* GetCached(ResourceId id) const = 0;
    virtual void Unload(IResource* resource) = 0;

    // Addressing（ResolvePath 返回指针有效期至下次可能修改缓存的 API 调用前；长期持有请用 ResolvePathCopy）
    virtual char const* ResolvePath(ResourceId id) const = 0;
    virtual bool ResolvePathCopy(ResourceId id, char* out_buffer, size_t buffer_size) const = 0;

    // Load complete dispatch（若设置则回调经 fn 投递，如主线程）
    virtual void SetLoadCompleteDispatcher(LoadCompleteDispatcherFn fn) = 0;

    // Streaming
    virtual StreamingHandle RequestStreaming(ResourceId id, int priority) = 0;
    virtual void SetStreamingPriority(StreamingHandle h, int priority) = 0;

    // Registration（序列化与反序列化一致：单接口 RegisterSerializer，Load 用 Deserialize、Save 用 Serialize）
    virtual void RegisterResourceLoader(ResourceType type, IResourceLoader* loader) = 0;
    virtual void RegisterSerializer(ResourceType type, IResourceSerializer* serializer) = 0;
    virtual void RegisterImporter(ResourceType type, IResourceImporter* importer) = 0;

    // Import / Save
    virtual bool Import(char const* path, ResourceType type, void* out_metadata_or_null) = 0;
    virtual bool Save(IResource* resource, char const* path) = 0;
};

// Global access; caller does not own the pointer.
IResourceManager* GetResourceManager();

} // namespace resource
} // namespace te
