/**
 * @file ResourceTypes.h
 * @brief Resource type enumeration (contract: specs/_contracts/013-resource-ABI.md).
 */
#ifndef TE_RESOURCE_RESOURCE_TYPES_H
#define TE_RESOURCE_RESOURCE_TYPES_H

#include <cstdint>
#include <cstddef>

namespace te {
namespace resource {

enum class ResourceType {
  Texture,
  Mesh,
  Material,
  Model,
  Effect,
  Terrain,
  Shader,
  Audio,
  Level,   // Level/关卡资源，029-World 持有并加载
  Custom,
  // Reserve for extension
  _Count
};

/** Loading status enumeration. */
enum class LoadStatus {
  Pending,     // Request created, not yet started
  Loading,     // Currently loading
  Completed,   // Load completed successfully
  Failed,      // Load failed
  Cancelled    // Load was cancelled
};

/** Loading result enumeration. */
enum class LoadResult {
  Ok,          // Load succeeded
  NotFound,    // Resource file not found
  Error,        // Load error occurred
  Cancelled     // Load was cancelled
};

/**
 * Loading priority enumeration.
 * Higher priority tasks are processed first in the loading queue.
 */
enum class LoadPriority : int {
  Background = 0,    // Background loading (e.g., prefetching)
  Low = 25,          // Low priority
  Normal = 50,       // Normal priority (default)
  High = 75,         // High priority
  Critical = 100     // Critical priority (e.g., required for current frame)
};

/**
 * Callback thread strategy enumeration.
 * Specifies which thread the completion callback should be invoked on.
 */
enum class CallbackThreadStrategy : int {
  MainThread,      // Callback on main thread (default, safe for UI updates)
  CallingThread,   // Callback on the thread that submitted the request
  AnyWorker        // Callback on any worker thread (fastest, but not UI-safe)
};

/**
 * Recursive load state enumeration.
 * Used to query the state of a resource and all its dependencies.
 */
enum class RecursiveLoadState {
  NotLoaded,       // Resource not loaded
  Loading,         // Resource or dependencies are being loaded
  PartiallyReady,  // Resource loaded but some dependencies not ready
  Ready,           // Resource and all dependencies loaded successfully
  Failed,          // Resource or one of its dependencies failed to load
  Cancelled        // Load was cancelled
};

/**
 * Resource state event enumeration.
 * Used for resource state change notifications.
 */
enum class ResourceStateEvent {
  Created,         // Resource instance created
  Loading,         // Resource started loading
  Loaded,          // Resource loaded (may have pending dependencies)
  DependenciesReady,  // All dependencies loaded
  DeviceReady,     // GPU resources ready (after EnsureDeviceResources)
  Unloading,       // Resource is being unloaded
  Unloaded,        // Resource has been unloaded
  Reloaded,        // Resource has been hot-reloaded
  Error            // Error occurred during load/unload
};

/**
 * Batch load result structure.
 * Contains results for a batch load operation.
 */
struct BatchLoadResult {
  std::size_t totalCount = 0;
  std::size_t successCount = 0;
  std::size_t failedCount = 0;
  std::size_t cancelledCount = 0;
  LoadResult overallResult = LoadResult::Ok;
};

/**
 * Resource load request info for batch operations.
 */
struct LoadRequestInfo {
  const char* path = nullptr;
  ResourceType type = ResourceType::Custom;
  LoadPriority priority = LoadPriority::Normal;
  void* user_data = nullptr;
};

}  // namespace resource
}  // namespace te

#endif  // TE_RESOURCE_RESOURCE_TYPES_H
