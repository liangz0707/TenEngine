/**
 * @file ResourceStreaming.h
 * @brief Streaming system for LOD management and on-demand loading.
 * 
 * Provides functionality for:
 * - LOD-based streaming
 * - Distance-based priority calculation
 * - Memory-aware streaming decisions
 * - Streaming callbacks and events
 */
#ifndef TE_RESOURCE_RESOURCE_STREAMING_H
#define TE_RESOURCE_RESOURCE_STREAMING_H

#include <te/resource/ResourceId.h>
#include <te/resource/ResourceTypes.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>
#include <chrono>

namespace te {
namespace resource {

class IResourceManager;
class IResource;

/**
 * Streaming LOD level.
 */
enum class StreamingLODLevel : int {
  Lowest = 0,     // Lowest quality (or unloaded)
  Low = 1,
  Medium = 2,
  High = 3,
  Highest = 4,    // Highest quality
  Full = 5,       // Full resolution (no LOD reduction)
  
  // Special values
  Invalid = -1,
  NotLoaded = -2
};

/**
 * Streaming policy type.
 */
enum class StreamingPolicy {
  Distance,       // Based on distance from viewer
  ScreenSize,     // Based on screen space size
  Memory,         // Based on available memory
  Manual,         // Manually controlled
  Hybrid          // Combination of above
};

/**
 * Streaming state for a resource.
 */
struct StreamingState {
  ResourceId resourceId;
  StreamingLODLevel currentLOD = StreamingLODLevel::NotLoaded;
  StreamingLODLevel targetLOD = StreamingLODLevel::Highest;
  float priority = 0.0f;
  float distance = 0.0f;
  float screenSize = 0.0f;
  std::size_t memorySize = 0;
  bool isStreaming = false;
  std::chrono::steady_clock::time_point lastAccessTime;
  std::chrono::steady_clock::time_point lastUpdateTime;
};

/**
 * Streaming configuration.
 */
struct StreamingConfig {
  StreamingPolicy policy = StreamingPolicy::Hybrid;
  
  // Distance-based settings
  float lod0Distance = 100.0f;     // Distance for LOD 0 (lowest)
  float lod1Distance = 200.0f;     // Distance for LOD 1
  float lod2Distance = 400.0f;     // Distance for LOD 2
  float lod3Distance = 800.0f;     // Distance for LOD 3
  float lod4Distance = 1600.0f;    // Distance for LOD 4 (highest)
  float unloadDistance = 2000.0f;  // Distance beyond which to unload
  
  // Screen size settings
  float minScreenSize = 0.01f;     // Minimum screen size to keep loaded
  float lod0ScreenSize = 0.05f;    // Screen size for LOD 0
  float lod4ScreenSize = 0.5f;     // Screen size for LOD 4
  
  // Memory settings
  std::size_t maxStreamingMemory = 512 * 1024 * 1024;  // 512MB
  float memoryThreshold = 0.9f;    // Start unloading at 90% capacity
  
  // Update settings
  float updateInterval = 0.1f;     // Seconds between streaming updates
  std::size_t maxOperationsPerUpdate = 10;  // Max load/unload per update
  
  // Async settings
  bool useAsyncLoading = true;
  LoadPriority streamingPriority = LoadPriority::Low;
};

/**
 * Streaming event type.
 */
enum class StreamingEvent {
  LODChanged,      // LOD level changed
  LoadingStarted,  // Started loading
  LoadingComplete, // Completed loading
  Unloading,       // About to unload
  Unloaded,        // Fully unloaded
  PriorityChanged, // Priority updated
  Error            // Error occurred
};

/**
 * Streaming event data.
 */
struct StreamingEventData {
  ResourceId resourceId;
  StreamingEvent event;
  StreamingLODLevel oldLOD;
  StreamingLODLevel newLOD;
  std::string message;
};

/**
 * Streaming callback type.
 */
using StreamingEventCallback = void (*)(StreamingEventData const& event, void* user_data);

/**
 * Streaming manager interface.
 */
class IStreamingManager {
 public:
  virtual ~IStreamingManager() = default;
  
  //==========================================================================
  // Configuration
  //==========================================================================
  
  /**
   * Set streaming configuration.
   */
  virtual void SetConfig(StreamingConfig const& config) = 0;
  
  /**
   * Get current configuration.
   */
  virtual StreamingConfig GetConfig() const = 0;
  
  /**
   * Set streaming policy.
   */
  virtual void SetPolicy(StreamingPolicy policy) = 0;
  
  /**
   * Get current policy.
   */
  virtual StreamingPolicy GetPolicy() const = 0;
  
  //==========================================================================
  // Resource Registration
  //==========================================================================
  
  /**
   * Register a streamable resource.
   * @param resourceId Resource ID
   * @param lodLevels Available LOD levels (e.g., {LOD0, LOD1, LOD2})
   * @param memorySizes Memory size for each LOD level
   */
  virtual void RegisterStreamable(ResourceId resourceId,
                                   std::vector<StreamingLODLevel> const& lodLevels,
                                   std::vector<std::size_t> const& memorySizes) = 0;
  
  /**
   * Unregister a streamable resource.
   */
  virtual void UnregisterStreamable(ResourceId resourceId) = 0;
  
  /**
   * Check if resource is registered for streaming.
   */
  virtual bool IsStreamable(ResourceId resourceId) const = 0;
  
  /**
   * Get streaming state for a resource.
   */
  virtual bool GetStreamingState(ResourceId resourceId, StreamingState& outState) const = 0;
  
  //==========================================================================
  // LOD Management
  //==========================================================================
  
  /**
   * Get current LOD level for a resource.
   */
  virtual StreamingLODLevel GetCurrentLOD(ResourceId resourceId) const = 0;
  
  /**
   * Get target LOD level for a resource.
   */
  virtual StreamingLODLevel GetTargetLOD(ResourceId resourceId) const = 0;
  
  /**
   * Force a specific LOD level.
   * @param resourceId Resource ID
   * @param lod LOD level
   * @param immediate If true, load immediately; otherwise queue
   */
  virtual void ForceLOD(ResourceId resourceId, StreamingLODLevel lod, bool immediate = false) = 0;
  
  /**
   * Clear forced LOD and return to automatic streaming.
   */
  virtual void ClearForcedLOD(ResourceId resourceId) = 0;
  
  //==========================================================================
  // Priority Management
  //==========================================================================
  
  /**
   * Set manual priority for a resource.
   */
  virtual void SetManualPriority(ResourceId resourceId, float priority) = 0;
  
  /**
   * Clear manual priority.
   */
  virtual void ClearManualPriority(ResourceId resourceId) = 0;
  
  /**
   * Get calculated priority for a resource.
   */
  virtual float GetPriority(ResourceId resourceId) const = 0;
  
  //==========================================================================
  // Viewer/Context Management
  //==========================================================================
  
  /**
   * Set viewer position for distance-based streaming.
   * @param x Viewer X position
   * @param y Viewer Y position
   * @param z Viewer Z position
   */
  virtual void SetViewerPosition(float x, float y, float z) = 0;
  
  /**
   * Set resource positions for distance calculations.
   * @param positions Map of resource ID to (x, y, z) position
   */
  virtual void SetResourcePositions(
      std::unordered_map<ResourceId, std::tuple<float, float, float>> const& positions) = 0;
  
  /**
   * Update position for a single resource.
   */
  virtual void UpdateResourcePosition(ResourceId resourceId, float x, float y, float z) = 0;
  
  /**
   * Set screen size for a resource (for screen-size-based policy).
   */
  virtual void SetResourceScreenSize(ResourceId resourceId, float screenSize) = 0;
  
  //==========================================================================
  // Streaming Operations
  //==========================================================================
  
  /**
   * Update streaming system.
   * Should be called periodically (e.g., once per frame).
   * @param deltaTime Time since last update
   */
  virtual void Update(float deltaTime) = 0;
  
  /**
   * Force immediate update of all streaming decisions.
   */
  virtual void ForceUpdate() = 0;
  
  /**
   * Suspend streaming (e.g., during loading screens).
   */
  virtual void Suspend() = 0;
  
  /**
   * Resume streaming.
   */
  virtual void Resume() = 0;
  
  /**
   * Check if streaming is suspended.
   */
  virtual bool IsSuspended() const = 0;
  
  //==========================================================================
  // Callbacks
  //==========================================================================
  
  /**
   * Subscribe to streaming events.
   */
  virtual void* SubscribeToEvents(StreamingEventCallback callback, void* user_data) = 0;
  
  /**
   * Unsubscribe from events.
   */
  virtual void Unsubscribe(void* subscription) = 0;
  
  //==========================================================================
  // Statistics
  //==========================================================================
  
  /**
   * Get total streaming memory usage.
   */
  virtual std::size_t GetTotalMemoryUsage() const = 0;
  
  /**
   * Get number of registered streamables.
   */
  virtual std::size_t GetStreamableCount() const = 0;
  
  /**
   * Get number of resources currently streaming (loading/unloading).
   */
  virtual std::size_t GetActiveStreamingCount() const = 0;
  
  /**
   * Get number of fully loaded resources.
   */
  virtual std::size_t GetFullyLoadedCount() const = 0;
  
  /**
   * Get number of partially loaded resources.
   */
  virtual std::size_t GetPartiallyLoadedCount() const = 0;
  
  /**
   * Get number of unloaded resources.
   */
  virtual std::size_t GetUnloadedCount() const = 0;
};

/**
 * Get the global streaming manager.
 */
IStreamingManager* GetStreamingManager();

}  // namespace resource
}  // namespace te

#endif  // TE_RESOURCE_RESOURCE_STREAMING_H
