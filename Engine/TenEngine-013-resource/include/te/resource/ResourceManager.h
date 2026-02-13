/**
 * @file ResourceManager.h
 * @brief IResourceManager interface (contract: specs/_contracts/013-resource-ABI.md).
 * 
 * ResourceManager: Coordinator and cache manager.
 * Responsibilities:
 * - Coordinate loading flow: call IResource::Load/LoadAsync
 * - Cache management: cache IResource* by ResourceId
 * - Async scheduling: manage loading queue, threads, callbacks
 * - GUID resolution: ResourceId → path resolution
 * 
 * Specific loading logic is implemented by IResource subclasses.
 */
#ifndef TE_RESOURCE_RESOURCE_MANAGER_H
#define TE_RESOURCE_RESOURCE_MANAGER_H

#include <te/resource/ResourceId.h>
#include <te/resource/ResourceTypes.h>
#include <cstddef>
#include <string>
#include <vector>

namespace te {
namespace resource {

// Forward declaration
class IResource;

/** Opaque handle returned by RequestLoadAsync. */
using LoadRequestId = void*;

/** Callback when async load completes; called on thread specified by IThreadPool::SetCallbackThread. */
using LoadCompleteCallback = void (*)(IResource* resource, LoadResult result, void* user_data);

/** Opaque handle for streaming. */
using StreamingHandle = void*;

/**
 * Resource factory function: creates IResource instance by ResourceType.
 */
using ResourceFactory = IResource* (*)(ResourceType);

/**
 * ResourceManager: Coordinator and cache manager.
 * 
 * Responsibilities:
 * - Coordinate loading flow: call IResource::Load/LoadAsync
 * - Cache management: cache IResource* by ResourceId
 * - Async scheduling: manage loading queue, threads, callbacks
 * - GUID resolution: ResourceId → path resolution
 * 
 * Specific loading logic is implemented by IResource subclasses.
 */
class IResourceManager {
 public:
  virtual ~IResourceManager() = default;

  /**
   * Request async load.
   * Creates resource instance (by ResourceType) and calls IResource::LoadAsync.
   * Thread-safe.
   * 
   * @param path Resource file path (AssetDesc file path)
   * @param type Resource type
   * @param on_done Completion callback
   * @param user_data User data for callback
   * @return LoadRequestId for status tracking
   */
  virtual LoadRequestId RequestLoadAsync(char const* path, ResourceType type,
                                        LoadCompleteCallback on_done, void* user_data) = 0;

  /**
   * Get load status.
   * Thread-safe.
   * 
   * @param id Load request ID
   * @return Current load status
   */
  virtual LoadStatus GetLoadStatus(LoadRequestId id) const = 0;

  /**
   * Get load progress (0.0 to 1.0).
   * Thread-safe.
   * 
   * @param id Load request ID
   * @return Progress value
   */
  virtual float GetLoadProgress(LoadRequestId id) const = 0;

  /**
   * Cancel load request.
   * Callback will still be called with Cancelled result.
   * Thread-safe.
   * 
   * @param id Load request ID
   */
  virtual void CancelLoad(LoadRequestId id) = 0;

  /**
   * Query cache only; returns nullptr on miss, does not trigger load.
   * Thread-safe.
   * 
   * @param id Resource ID
   * @return Cached IResource* or nullptr
   */
  virtual IResource* GetCached(ResourceId id) const = 0;

  /**
   * Synchronous load.
   * Creates resource instance (by ResourceType) and calls IResource::Load.
   * Blocks until completion.
   * Thread-safe.
   * 
   * @param path Resource file path (AssetDesc file path)
   * @param type Resource type
   * @return IResource* on success, nullptr on failure
   */
  virtual IResource* LoadSync(char const* path, ResourceType type) = 0;

  /**
   * Unload resource.
   * Calls IResource::Release and removes from cache when refcount reaches zero.
   * Thread-safe.
   * 
   * @param resource Resource to unload
   */
  virtual void Unload(IResource* resource) = 0;

  /**
   * Request streaming load.
   * 
   * @param id Resource ID
   * @param priority Streaming priority
   * @return StreamingHandle
   */
  virtual StreamingHandle RequestStreaming(ResourceId id, int priority) = 0;

  /**
   * Set streaming priority.
   * 
   * @param h Streaming handle
   * @param priority New priority
   */
  virtual void SetStreamingPriority(StreamingHandle h, int priority) = 0;

  /**
   * Register resource factory (creates IResource instance by ResourceType).
   * Called by resource type modules during initialization.
   * 
   * @param type Resource type
   * @param factory Factory function: IResource* (*)(ResourceType)
   */
  virtual void RegisterResourceFactory(ResourceType type, ResourceFactory factory) = 0;

  /**
   * Import resource from source file.
   * Creates resource instance (by ResourceType) and calls IResource::Import.
   * 
   * @param path Source file path
   * @param type Resource type
   * @param out_metadata_or_null Optional output metadata
   * @return true on success
   */
  virtual bool Import(char const* path, ResourceType type, void* out_metadata_or_null) = 0;

  /**
   * Save resource to file.
   * Calls IResource::Save.
   * 
   * @param resource Resource to save
   * @param path Output file path (AssetDesc file path)
   * @return true on success
   */
  virtual bool Save(IResource* resource, char const* path) = 0;

  /**
   * Resolve ResourceId to path; returns nullptr if unresolved.
   * Thread-safe.
   * 
   * @param id Resource ID
   * @return File path or nullptr
   */
  virtual char const* ResolvePath(ResourceId id) const = 0;

  /**
   * Set asset root directory (e.g. project assets path). Must be set before LoadAllManifests.
   */
  virtual void SetAssetRoot(char const* path) = 0;

  /**
   * Load repository config and all repo manifests. Call after SetAssetRoot.
   */
  virtual void LoadAllManifests() = 0;

  /**
   * Resolve ResourceId to ResourceType from manifest. Returns ResourceType::_Count if not found.
   */
  virtual ResourceType ResolveType(ResourceId id) const = 0;

  /**
   * Load resource by GUID (resolve path/type then LoadSync). Returns nullptr on failure.
   */
  virtual IResource* LoadSyncByGuid(ResourceId id) = 0;

  /**
   * Import into repository under parent asset path. Storage path is computed internally (repo/type/guid/displayName.ext).
   * parentAssetPath is the selected folder in UI; displayName from source filename.
   */
  virtual bool ImportIntoRepository(char const* sourcePath, ResourceType type,
                                    char const* repositoryName, char const* parentAssetPath,
                                    void* out_metadata_or_null) = 0;

  /**
   * Create a new repository (add to config, create directory).
   */
  virtual bool CreateRepository(char const* name) = 0;

  /**
   * Get list of repository names.
   */
  virtual void GetRepositoryList(std::vector<std::string>& out) const = 0;

  /** Single resource info for enumeration. Only asset path is exposed; storage path is internal. */
  struct ResourceInfo {
    ResourceId guid;
    std::string assetPath;
    ResourceType type = ResourceType::Custom;
    std::string repository;
    std::string displayName;
  };

  /**
   * Enumerate all resources from all manifests (for resource manager UI).
   */
  virtual void GetResourceInfos(std::vector<ResourceInfo>& out) const = 0;

  /**
   * Get all asset folder paths (empty folders) from all manifests.
   */
  virtual void GetAssetFolders(std::vector<std::string>& out) const = 0;

  /**
   * Get asset folders for one repository (for building repo-scoped tree).
   */
  virtual void GetAssetFoldersForRepository(char const* repositoryName, std::vector<std::string>& out) const = 0;

  /**
   * Move resource to another repository (physical move + update both manifests).
   */
  virtual bool MoveResourceToRepository(ResourceId id, char const* targetRepository) = 0;

  /**
   * Update resource asset path only (no physical move). newAssetPath must be under same repo.
   */
  virtual bool UpdateAssetPath(ResourceId id, char const* newAssetPath) = 0;

  /**
   * Move all resources under assetPath to newParentAssetPath (asset path only).
   */
  virtual bool MoveAssetFolder(char const* assetPath, char const* newParentAssetPath) = 0;

  /**
   * Add empty asset folder (persisted in manifest).
   */
  virtual bool AddAssetFolder(char const* repositoryName, char const* assetPath) = 0;
};

/** Global accessor; provided by Subsystems or singleton. Caller does not own pointer. */
IResourceManager* GetResourceManager();

}  // namespace resource
}  // namespace te

#endif  // TE_RESOURCE_RESOURCE_MANAGER_H
