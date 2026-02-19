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
#include <functional>

namespace te {
namespace resource {

// Forward declaration
class IResource;

/** Opaque handle returned by RequestLoadAsync. */
using LoadRequestId = void*;

/** Opaque handle returned by RequestLoadBatchAsync. */
using BatchLoadRequestId = void*;

/** Callback when async load completes; called on thread specified by callback strategy. */
using LoadCompleteCallback = void (*)(IResource* resource, LoadResult result, void* user_data);

/** Opaque handle for streaming. */
using StreamingHandle = void*;

/** Opaque handle for batch load operations. */
using BatchLoadHandle = void*;

/**
 * Resource factory function: creates IResource instance by ResourceType.
 */
using ResourceFactory = IResource* (*)(ResourceType);

/**
 * Batch load complete callback.
 * Called when all resources in a batch have completed loading (success or failure).
 */
using BatchLoadCompleteCallback = void (*)(BatchLoadHandle handle, 
                                            BatchLoadResult const* result, 
                                            void* user_data);

/**
 * Resource state change callback.
 * Called when a resource's state changes.
 */
using ResourceStateCallback = void (*)(ResourceId id, 
                                        ResourceStateEvent event, 
                                        void* user_data);

/**
 * Extended load request options.
 */
struct LoadOptions {
  LoadPriority priority = LoadPriority::Normal;
  CallbackThreadStrategy callbackThread = CallbackThreadStrategy::MainThread;
  bool preloadDependencies = false;  // If true, only load dependencies first
  void* user_data = nullptr;
};

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

  //============================================================================
  // Basic Load Operations
  //============================================================================

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
   * Request async load with extended options.
   * Supports priority, callback thread strategy, and dependency preloading.
   * Thread-safe.
   * 
   * @param path Resource file path
   * @param type Resource type
   * @param on_done Completion callback
   * @param options Extended load options
   * @return LoadRequestId for status tracking
   */
  virtual LoadRequestId RequestLoadAsyncEx(char const* path, ResourceType type,
                                           LoadCompleteCallback on_done,
                                           LoadOptions const& options) = 0;

  /**
   * Request batch async load.
   * Loads multiple resources in parallel with a single completion callback.
   * Thread-safe.
   * 
   * @param requests Array of load request info
   * @param count Number of requests
   * @param on_done Batch completion callback
   * @param user_data User data for callback
   * @param options Default options for all requests
   * @return BatchLoadRequestId for batch status tracking
   */
  virtual BatchLoadRequestId RequestLoadBatchAsync(
      LoadRequestInfo const* requests, std::size_t count,
      BatchLoadCompleteCallback on_done, void* user_data,
      LoadOptions const& options) = 0;

  /**
   * Get batch load result.
   * Thread-safe.
   * 
   * @param id Batch load request ID
   * @param out_result Output result structure
   * @return true if batch ID is valid
   */
  virtual bool GetBatchLoadResult(BatchLoadRequestId id, BatchLoadResult& out_result) const = 0;

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
   * Cancel batch load request.
   * Callback will still be called with partial results.
   * Thread-safe.
   * 
   * @param id Batch load request ID
   */
  virtual void CancelBatchLoad(BatchLoadRequestId id) = 0;

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

  //============================================================================
  // Recursive Dependency State Query
  //============================================================================

  /**
   * Get recursive load state for a resource and all its dependencies.
   * Thread-safe.
   * 
   * @param id Resource ID
   * @return Recursive load state
   */
  virtual RecursiveLoadState GetRecursiveLoadState(ResourceId id) const = 0;

  /**
   * Get recursive load state by LoadRequestId.
   * Thread-safe.
   * 
   * @param id Load request ID
   * @return Recursive load state
   */
  virtual RecursiveLoadState GetRecursiveLoadStateByRequestId(LoadRequestId id) const = 0;

  /**
   * Check if resource and all dependencies are ready.
   * Thread-safe.
   * 
   * @param id Resource ID
   * @return true if resource and all dependencies are loaded
   */
  virtual bool IsResourceReady(ResourceId id) const = 0;

  /**
   * Check if resource and all dependencies are ready (by request ID).
   * Thread-safe.
   * 
   * @param id Load request ID
   * @return true if resource and all dependencies are loaded
   */
  virtual bool IsResourceReadyByRequestId(LoadRequestId id) const = 0;

  //============================================================================
  // Resource State Events
  //============================================================================

  /**
   * Subscribe to resource state changes for a specific resource.
   * Thread-safe.
   * 
   * @param id Resource ID to monitor
   * @param callback State change callback
   * @param user_data User data for callback
   * @return Subscription handle (use UnsubscribeResourceState to unsubscribe)
   */
  virtual void* SubscribeResourceState(ResourceId id, 
                                        ResourceStateCallback callback, 
                                        void* user_data) = 0;

  /**
   * Subscribe to all resource state changes globally.
   * Thread-safe.
   * 
   * @param callback State change callback
   * @param user_data User data for callback
   * @return Subscription handle
   */
  virtual void* SubscribeGlobalResourceState(ResourceStateCallback callback, 
                                              void* user_data) = 0;

  /**
   * Unsubscribe from resource state changes.
   * Thread-safe.
   * 
   * @param subscription_handle Handle returned from Subscribe*
   */
  virtual void UnsubscribeResourceState(void* subscription_handle) = 0;

  //============================================================================
  // Dependency Management
  //============================================================================

  /**
   * Preload dependencies for a resource without loading the resource itself.
   * Useful for warming up caches or preparing for level transitions.
   * Thread-safe.
   * 
   * @param id Resource ID whose dependencies to preload
   * @param on_done Completion callback (called when all dependencies are loaded)
   * @param user_data User data for callback
   * @return LoadRequestId for tracking (or nullptr on error)
   */
  virtual LoadRequestId PreloadDependencies(ResourceId id,
                                            LoadCompleteCallback on_done,
                                            void* user_data) = 0;

  /**
   * Get dependency tree for a resource.
   * Thread-safe.
   * 
   * @param id Resource ID
   * @param out_dependencies Output vector to receive all dependency IDs
   * @param max_depth Maximum depth to traverse (0 = unlimited)
   * @return true if resource found
   */
  virtual bool GetDependencyTree(ResourceId id, 
                                  std::vector<ResourceId>& out_dependencies,
                                  std::size_t max_depth = 0) const = 0;

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

  /**
   * Remove asset folder from manifest. Folder and all descendant folders are removed;
   * resources under that path are moved to the folder's parent path.
   * @param repositoryName Repository that contains the folder
   * @param assetPath Full folder path to remove (e.g. "A/B/C")
   * @return true if any change was made and saved
   */
  virtual bool RemoveAssetFolder(char const* repositoryName, char const* assetPath) = 0;

  //============================================================================
  // Memory Management
  //============================================================================

  /**
   * Get total memory usage by all cached resources.
   * Thread-safe.
   * 
   * @return Total memory usage in bytes
   */
  virtual std::size_t GetTotalMemoryUsage() const = 0;

  /**
   * Get memory usage for a specific resource.
   * Thread-safe.
   * 
   * @param id Resource ID
   * @return Memory usage in bytes, or 0 if not cached
   */
  virtual std::size_t GetResourceMemoryUsage(ResourceId id) const = 0;

  /**
   * Set memory budget for resource cache.
   * When budget is exceeded, LRU resources will be unloaded.
   * Thread-safe.
   * 
   * @param budget_bytes Memory budget in bytes (0 = unlimited)
   */
  virtual void SetMemoryBudget(std::size_t budget_bytes) = 0;

  /**
   * Get current memory budget.
   * Thread-safe.
   * 
   * @return Memory budget in bytes (0 = unlimited)
   */
  virtual std::size_t GetMemoryBudget() const = 0;

  /**
   * Force garbage collection to free memory.
   * Unloads resources with zero refcount and LRU resources if over budget.
   * Thread-safe.
   * 
   * @return Number of resources unloaded
   */
  virtual std::size_t ForceGarbageCollect() = 0;
};

/** Global accessor; provided by Subsystems or singleton. Caller does not own pointer. */
IResourceManager* GetResourceManager();

}  // namespace resource
}  // namespace te

#endif  // TE_RESOURCE_RESOURCE_MANAGER_H
