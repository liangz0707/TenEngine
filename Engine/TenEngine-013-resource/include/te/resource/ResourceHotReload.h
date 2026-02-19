/**
 * @file ResourceHotReload.h
 * @brief Hot reload system for development-time resource updates.
 * 
 * Provides functionality for:
 * - File system monitoring
 * - Automatic resource reloading
 * - Hot reload events and callbacks
 * - Shader-specific hot reload support
 */
#ifndef TE_RESOURCE_RESOURCE_HOT_RELOAD_H
#define TE_RESOURCE_RESOURCE_HOT_RELOAD_H

#include <te/resource/ResourceId.h>
#include <te/resource/ResourceTypes.h>
#include <te/resource/ResourceEvent.h>
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <mutex>
#include <unordered_set>
#include <chrono>

namespace te {
namespace resource {

class IResourceManager;
class IResource;

/**
 * Hot reload callback type.
 * Called when a resource is about to be reloaded or has been reloaded.
 */
using HotReloadCallback = void (*)(ResourceId id, 
                                    IResource* oldResource,
                                    IResource* newResource,
                                    void* user_data);

/**
 * File change event type.
 */
enum class FileChangeType {
  Created,      // File was created
  Modified,     // File was modified
  Deleted,      // File was deleted
  Renamed       // File was renamed (old name provided in event)
};

/**
 * File change event data.
 */
struct FileChangeEvent {
  std::string path;             // File path that changed
  std::string oldPath;          // Old path (for rename events)
  FileChangeType changeType;    // Type of change
  std::chrono::system_clock::time_point timestamp;  // When the change occurred
  
  FileChangeEvent() : changeType(FileChangeType::Modified) {}
};

/**
 * File change callback type.
 */
using FileChangeCallback = void (*)(FileChangeEvent const& event, void* user_data);

/**
 * Hot reload configuration.
 */
struct HotReloadConfig {
  bool enabled = true;                    // Master enable switch
  bool watchSourceFiles = true;           // Watch source files (.png, .obj, etc.)
  bool watchAssetFiles = true;            // Watch asset files (.texture, .mesh, etc.)
  bool watchShaderFiles = true;           // Watch shader files (.shader, .hlsl, etc.)
  bool autoReload = true;                 // Automatically reload changed resources
  bool reloadDependencies = true;         // Also reload dependent resources
  std::chrono::milliseconds debounceTime{500};  // Debounce time for file changes
  std::vector<std::string> watchPaths;    // Additional paths to watch
  std::vector<std::string> excludePatterns;  // Patterns to exclude (glob)
};

/**
 * Hot reload subscription handle.
 */
using HotReloadSubscriptionHandle = void*;

/**
 * File watcher interface.
 * Monitors file system for changes.
 */
class IFileWatcher {
 public:
  virtual ~IFileWatcher() = default;
  
  /**
   * Add a path to watch.
   * @param path Directory or file path
   * @param recursive Watch subdirectories
   * @return true if path was added
   */
  virtual bool AddWatchPath(std::string const& path, bool recursive = true) = 0;
  
  /**
   * Remove a watched path.
   * @param path Path to remove
   */
  virtual void RemoveWatchPath(std::string const& path) = 0;
  
  /**
   * Subscribe to file change events.
   * @param callback Callback function
   * @param userData User data
   * @return Subscription handle
   */
  virtual HotReloadSubscriptionHandle SubscribeToFileChanges(
      FileChangeCallback callback, void* userData) = 0;
  
  /**
   * Unsubscribe from file change events.
   * @param handle Subscription handle
   */
  virtual void UnsubscribeFromFileChanges(HotReloadSubscriptionHandle handle) = 0;
  
  /**
   * Start watching (typically starts a background thread).
   */
  virtual void Start() = 0;
  
  /**
   * Stop watching.
   */
  virtual void Stop() = 0;
  
  /**
   * Check if currently watching.
   */
  virtual bool IsWatching() const = 0;
  
  /**
   * Process pending file change events.
   * Should be called on the main thread.
   */
  virtual void ProcessPendingEvents() = 0;
  
  /**
   * Get number of pending file change events.
   */
  virtual std::size_t GetPendingEventCount() const = 0;
};

/**
 * Hot reload manager interface.
 * Manages hot reload functionality.
 */
class IHotReloadManager {
 public:
  virtual ~IHotReloadManager() = default;
  
  //==========================================================================
  // Configuration
  //==========================================================================
  
  /**
   * Set hot reload configuration.
   * @param config Configuration
   */
  virtual void SetConfig(HotReloadConfig const& config) = 0;
  
  /**
   * Get current configuration.
   * @return Current configuration
   */
  virtual HotReloadConfig GetConfig() const = 0;
  
  /**
   * Enable or disable hot reload.
   * @param enabled Enable state
   */
  virtual void SetEnabled(bool enabled) = 0;
  
  /**
   * Check if hot reload is enabled.
   * @return true if enabled
   */
  virtual bool IsEnabled() const = 0;
  
  //==========================================================================
  // File Watching
  //==========================================================================
  
  /**
   * Get the file watcher instance.
   * @return File watcher interface
   */
  virtual IFileWatcher* GetFileWatcher() = 0;
  
  /**
   * Add a path to watch for hot reload.
   * @param path Directory path
   * @param recursive Watch subdirectories
   */
  virtual void AddWatchPath(std::string const& path, bool recursive = true) = 0;
  
  /**
   * Remove a watched path.
   * @param path Path to remove
   */
  virtual void RemoveWatchPath(std::string const& path) = 0;
  
  /**
   * Watch the asset root directory.
   * Uses the asset root from ResourceManager.
   */
  virtual void WatchAssetRoot() = 0;
  
  //==========================================================================
  // Manual Reload
  //==========================================================================
  
  /**
   * Reload a specific resource.
   * @param manager Resource manager
   * @param resourceId Resource ID to reload
   * @param force If true, reload even if not hot-reloadable
   * @return true if reload was triggered
   */
  virtual bool ReloadResource(IResourceManager* manager,
                              ResourceId resourceId,
                              bool force = false) = 0;
  
  /**
   * Reload a resource by path.
   * @param manager Resource manager
   * @param path Resource path
   * @param force If true, reload even if not hot-reloadable
   * @return true if reload was triggered
   */
  virtual bool ReloadResourceByPath(IResourceManager* manager,
                                    std::string const& path,
                                    bool force = false) = 0;
  
  /**
   * Reload all loaded resources.
   * @param manager Resource manager
   * @return Number of resources reloaded
   */
  virtual std::size_t ReloadAllResources(IResourceManager* manager) = 0;
  
  /**
   * Reload all resources of a specific type.
   * @param manager Resource manager
   * @param type Resource type
   * @return Number of resources reloaded
   */
  virtual std::size_t ReloadResourcesByType(IResourceManager* manager,
                                            ResourceType type) = 0;
  
  /**
   * Reload all dependent resources of a resource.
   * @param manager Resource manager
   * @param resourceId Resource whose dependents to reload
   * @return Number of resources reloaded
   */
  virtual std::size_t ReloadDependentResources(IResourceManager* manager,
                                               ResourceId resourceId) = 0;
  
  //==========================================================================
  // Subscriptions
  //==========================================================================
  
  /**
   * Subscribe to hot reload events.
   * @param callback Callback function
   * @param userData User data
   * @return Subscription handle
   */
  virtual HotReloadSubscriptionHandle SubscribeToHotReload(
      HotReloadCallback callback, void* userData) = 0;
  
  /**
   * Unsubscribe from hot reload events.
   * @param handle Subscription handle
   */
  virtual void UnsubscribeFromHotReload(HotReloadSubscriptionHandle handle) = 0;
  
  //==========================================================================
  // Processing
  //==========================================================================
  
  /**
   * Process pending hot reload operations.
   * Should be called on the main thread.
   */
  virtual void ProcessPendingReloads(IResourceManager* manager) = 0;
  
  /**
   * Get number of pending reload operations.
   */
  virtual std::size_t GetPendingReloadCount() const = 0;
  
  //==========================================================================
  // Resource Support
  //==========================================================================
  
  /**
   * Check if a resource type supports hot reload.
   * @param type Resource type
   * @return true if hot reload is supported
   */
  virtual bool IsHotReloadable(ResourceType type) const = 0;
  
  /**
   * Register a resource type as hot-reloadable.
   * @param type Resource type
   * @param supported Whether hot reload is supported
   */
  virtual void SetHotReloadable(ResourceType type, bool supported) = 0;
  
  //==========================================================================
  // Statistics
  //==========================================================================
  
  /**
   * Get total number of hot reloads performed.
   */
  virtual std::size_t GetTotalReloadCount() const = 0;
  
  /**
   * Get number of failed hot reloads.
   */
  virtual std::size_t GetFailedReloadCount() const = 0;
  
  /**
   * Get last hot reload timestamp.
   */
  virtual std::chrono::system_clock::time_point GetLastReloadTime() const = 0;
};

/**
 * Get the global hot reload manager.
 */
IHotReloadManager* GetHotReloadManager();

/**
 * RAII helper to temporarily disable hot reload.
 */
class ScopedHotReloadDisable {
 public:
  ScopedHotReloadDisable();
  ~ScopedHotReloadDisable();
  
 private:
  bool wasEnabled_;
};

/**
 * Interface for resources that support custom hot reload behavior.
 */
class IHotReloadableResource {
 public:
  virtual ~IHotReloadableResource() = default;
  
  /**
   * Called before the resource is reloaded.
   * @return true to proceed with reload, false to cancel
   */
  virtual bool OnPreReload() { return true; }
  
  /**
   * Called after the resource has been reloaded.
   * @param success Whether the reload succeeded
   */
  virtual void OnPostReload(bool success) { (void)success; }
  
  /**
   * Get the file paths that should be watched for this resource.
   * @param outPaths Output vector of paths
   */
  virtual void GetWatchPaths(std::vector<std::string>& outPaths) const {
    (void)outPaths;
  }
  
  /**
   * Check if the resource can be hot reloaded.
   * @return true if hot reload is supported
   */
  virtual bool CanHotReload() const { return true; }
};

}  // namespace resource
}  // namespace te

#endif  // TE_RESOURCE_RESOURCE_HOT_RELOAD_H
