/**
 * @file ResourceManager.h
 * @brief IResourceManager and load types (contract: specs/_contracts/013-resource-ABI.md).
 */
#ifndef TE_RESOURCE_RESOURCE_MANAGER_H
#define TE_RESOURCE_RESOURCE_MANAGER_H

#include <te/resource/ResourceId.h>
#include <te/resource/ResourceTypes.h>
#include <te/resource/Resource.h>
#include <cstddef>

namespace te {
namespace resource {

/** Opaque handle returned by RequestLoadAsync. */
using LoadRequestId = void*;

enum class LoadStatus { Pending, Loading, Completed, Failed, Cancelled };
enum class LoadResult { Ok, NotFound, Error, Cancelled };

/** Callback when root and all recursive dependencies are loaded; called once. */
using LoadCompleteCallback = void (*)(IResource* resource, LoadResult result, void* user_data);

/** Opaque handle for streaming. */
using StreamingHandle = void*;

class IResourceLoader;
class IResourceImporter;
class IDeserializer;

class IResourceManager {
 public:
  virtual ~IResourceManager() = default;

  virtual LoadRequestId RequestLoadAsync(char const* path, ResourceType type,
                                        LoadCompleteCallback on_done, void* user_data) = 0;
  virtual LoadStatus GetLoadStatus(LoadRequestId id) const = 0;
  virtual float GetLoadProgress(LoadRequestId id) const = 0;
  virtual void CancelLoad(LoadRequestId id) = 0;

  /** Query cache only; returns nullptr on miss, does not trigger load. */
  virtual IResource* GetCached(ResourceId id) const = 0;

  virtual IResource* LoadSync(char const* path, ResourceType type) = 0;
  virtual void Unload(IResource* resource) = 0;

  virtual StreamingHandle RequestStreaming(ResourceId id, int priority) = 0;
  virtual void SetStreamingPriority(StreamingHandle h, int priority) = 0;

  virtual void RegisterResourceLoader(ResourceType type, IResourceLoader* loader) = 0;
  virtual void RegisterDeserializer(ResourceType type, IDeserializer* deserializer) = 0;
  virtual void RegisterImporter(ResourceType type, IResourceImporter* importer) = 0;

  virtual bool Import(char const* path, ResourceType type, void* out_metadata_or_null) = 0;
  virtual bool Save(IResource* resource, char const* path) = 0;

  /** Register per-type saver: module produces content from IResource and writes (or 013 writes via unified API). */
  using SaverFn = bool (*)(IResource* resource, char const* path);
  virtual void RegisterSaver(ResourceType type, SaverFn fn) = 0;

  /** Resolve ResourceId to path; returns nullptr if unresolved. */
  virtual char const* ResolvePath(ResourceId id) const = 0;
};

/** Global accessor; provided by Subsystems or singleton. Caller does not own pointer. */
IResourceManager* GetResourceManager();

}  // namespace resource
}  // namespace te

#endif  // TE_RESOURCE_RESOURCE_MANAGER_H
