/**
 * @file ResourceDebug.h
 * @brief Debug and profiling tools for the resource system.
 * 
 * Provides functionality for:
 * - Resource loading performance profiling
 * - Dependency graph visualization
 * - Resource leak detection
 * - Memory usage tracking
 * - Debug logging
 */
#ifndef TE_RESOURCE_RESOURCE_DEBUG_H
#define TE_RESOURCE_RESOURCE_DEBUG_H

#include <te/resource/ResourceId.h>
#include <te/resource/ResourceTypes.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <memory>
#include <functional>

namespace te {
namespace resource {

class IResourceManager;
class IResource;

/**
 * Debug log level.
 */
enum class ResourceDebugLogLevel {
  None = 0,
  Error = 1,
  Warning = 2,
  Info = 3,
  Debug = 4,
  Trace = 5
};

/**
 * Resource profiling data.
 */
struct ResourceProfileData {
  ResourceId resourceId;
  std::string path;
  ResourceType type;
  
  // Timing data (in milliseconds)
  double loadTimeMs = 0.0;           // Time to load the resource
  double dependencyLoadTimeMs = 0.0; // Time to load dependencies
  double gpuUploadTimeMs = 0.0;      // Time to upload to GPU
  
  // Size data
  std::size_t fileSize = 0;          // File size on disk
  std::size_t memorySize = 0;        // Memory usage
  
  // Counters
  std::size_t dependencyCount = 0;   // Number of dependencies
  std::size_t loadCount = 0;         // Number of times loaded
  
  // Timestamps
  std::chrono::system_clock::time_point loadStartTime;
  std::chrono::system_clock::time_point loadEndTime;
  
  ResourceProfileData() = default;
};

/**
 * Resource system statistics.
 */
struct ResourceSystemStats {
  // Cache statistics
  std::size_t totalCachedResources = 0;
  std::size_t totalMemoryUsage = 0;
  std::size_t peakMemoryUsage = 0;
  
  // Load statistics
  std::size_t totalLoads = 0;
  std::size_t successfulLoads = 0;
  std::size_t failedLoads = 0;
  std::size_t cacheHits = 0;
  std::size_t cacheMisses = 0;
  
  // Timing statistics
  double averageLoadTimeMs = 0.0;
  double maxLoadTimeMs = 0.0;
  double totalLoadTimeMs = 0.0;
  
  // Async statistics
  std::size_t pendingAsyncLoads = 0;
  std::size_t activeAsyncLoads = 0;
  
  // Dependency statistics
  std::size_t totalDependencies = 0;
  std::size_t maxDependencyDepth = 0;
  std::size_t circularDependenciesDetected = 0;
  
  // Hot reload statistics (if enabled)
  std::size_t hotReloadsTriggered = 0;
  std::size_t hotReloadsSucceeded = 0;
  std::size_t hotReloadsFailed = 0;
  
  // Time since last reset
  std::chrono::system_clock::time_point statsStartTime;
};

/**
 * Resource leak information.
 */
struct ResourceLeakInfo {
  ResourceId resourceId;
  std::string path;
  ResourceType type;
  std::size_t refCount;
  std::chrono::system_clock::time_point loadTime;
  std::string stackTrace;       // Stack trace when resource was loaded
  std::vector<std::string> tagHistory;  // Tags at time of leak detection
};

/**
 * Dependency graph node for visualization.
 */
struct DependencyGraphNode {
  ResourceId resourceId;
  std::string path;
  ResourceType type;
  std::size_t memoryUsage;
  bool isLoaded;
  std::vector<ResourceId> dependencies;
  std::vector<ResourceId> dependents;  // Resources that depend on this one
};

/**
 * Dependency graph data for visualization.
 */
struct DependencyGraphData {
  std::vector<DependencyGraphNode> nodes;
  std::vector<std::pair<ResourceId, ResourceId>> edges;  // (from, to)
  std::size_t maxDepth;
  std::size_t totalNodes;
  std::size_t totalEdges;
};

/**
 * Debug log callback type.
 */
using ResourceDebugLogCallback = void (*)(ResourceDebugLogLevel level,
                                          std::string const& message,
                                          void* user_data);

/**
 * Resource profiler interface.
 */
class IResourceProfiler {
 public:
  virtual ~IResourceProfiler() = default;
  
  //==========================================================================
  // Profiling Control
  //==========================================================================
  
  /**
   * Enable or disable profiling.
   */
  virtual void SetEnabled(bool enabled) = 0;
  
  /**
   * Check if profiling is enabled.
   */
  virtual bool IsEnabled() const = 0;
  
  /**
   * Reset all profiling data.
   */
  virtual void Reset() = 0;
  
  //==========================================================================
  // Data Collection
  //==========================================================================
  
  /**
   * Begin profiling a resource load.
   * Called by ResourceManager at the start of a load operation.
   */
  virtual void BeginLoad(ResourceId id, std::string const& path, ResourceType type) = 0;
  
  /**
   * End profiling a resource load.
   * Called by ResourceManager at the end of a load operation.
   */
  virtual void EndLoad(ResourceId id, bool success) = 0;
  
  /**
   * Record dependency load time.
   */
  virtual void RecordDependencyLoadTime(ResourceId id, double timeMs) = 0;
  
  /**
   * Record GPU upload time.
   */
  virtual void RecordGpuUploadTime(ResourceId id, double timeMs) = 0;
  
  /**
   * Record cache hit.
   */
  virtual void RecordCacheHit(ResourceId id) = 0;
  
  /**
   * Record cache miss.
   */
  virtual void RecordCacheMiss(ResourceId id) = 0;
  
  //==========================================================================
  // Data Retrieval
  //==========================================================================
  
  /**
   * Get profile data for a specific resource.
   */
  virtual bool GetProfileData(ResourceId id, ResourceProfileData& outData) const = 0;
  
  /**
   * Get all profile data.
   */
  virtual void GetAllProfileData(std::vector<ResourceProfileData>& outData) const = 0;
  
  /**
   * Get system-wide statistics.
   */
  virtual ResourceSystemStats GetSystemStats() const = 0;
  
  /**
   * Get top N slowest loading resources.
   */
  virtual void GetSlowestLoads(std::size_t count, 
                                std::vector<ResourceProfileData>& outData) const = 0;
  
  /**
   * Get top N largest resources by memory.
   */
  virtual void GetLargestResources(std::size_t count,
                                    std::vector<ResourceProfileData>& outData) const = 0;
};

/**
 * Resource leak detector interface.
 */
class IResourceLeakDetector {
 public:
  virtual ~IResourceLeakDetector() = default;
  
  //==========================================================================
  // Detection Control
  //==========================================================================
  
  /**
   * Enable or disable leak detection.
   */
  virtual void SetEnabled(bool enabled) = 0;
  
  /**
   * Check if leak detection is enabled.
   */
  virtual bool IsEnabled() const = 0;
  
  /**
   * Enable stack trace capture.
   * Capturing stack traces has performance overhead.
   */
  virtual void SetCaptureStackTrace(bool capture) = 0;
  
  //==========================================================================
  // Detection Operations
  //==========================================================================
  
  /**
   * Detect potential resource leaks.
   * A leak is a resource that has been loaded but not properly released.
   * @param manager Resource manager
   * @param outLeaks Output vector of leak info
   * @return Number of potential leaks found
   */
  virtual std::size_t DetectLeaks(IResourceManager* manager,
                                   std::vector<ResourceLeakInfo>& outLeaks) = 0;
  
  /**
   * Check for specific types of leaks.
   * @param manager Resource manager
   * @param type Resource type to check
   * @param outLeaks Output vector of leak info
   * @return Number of potential leaks found
   */
  virtual std::size_t DetectLeaksByType(IResourceManager* manager,
                                         ResourceType type,
                                         std::vector<ResourceLeakInfo>& outLeaks) = 0;
  
  /**
   * Get all tracked resources.
   */
  virtual void GetTrackedResources(std::vector<ResourceLeakInfo>& outInfo) const = 0;
  
  /**
   * Mark current state as baseline (resources loaded now won't be reported as leaks).
   */
  virtual void MarkBaseline() = 0;
  
  /**
   * Clear the baseline.
   */
  virtual void ClearBaseline() = 0;
};

/**
 * Resource debug visualizer interface.
 */
class IResourceDebugVisualizer {
 public:
  virtual ~IResourceDebugVisualizer() = default;
  
  //==========================================================================
  // Dependency Graph
  //==========================================================================
  
  /**
   * Get dependency graph data for visualization.
   */
  virtual DependencyGraphData GetDependencyGraph(IResourceManager* manager) = 0;
  
  /**
   * Get dependency graph for a specific resource.
   */
  virtual DependencyGraphData GetResourceDependencyGraph(IResourceManager* manager,
                                                          ResourceId rootId,
                                                          std::size_t maxDepth = 10) = 0;
  
  /**
   * Export dependency graph to DOT format (Graphviz).
   */
  virtual std::string ExportDependencyGraphDot(IResourceManager* manager) = 0;
  
  /**
   * Export dependency graph to JSON format.
   */
  virtual std::string ExportDependencyGraphJson(IResourceManager* manager) = 0;
  
  //==========================================================================
  // Memory Visualization
  //==========================================================================
  
  /**
   * Get memory usage breakdown by resource type.
   */
  virtual void GetMemoryByType(IResourceManager* manager,
                                std::unordered_map<ResourceType, std::size_t>& outData) = 0;
  
  /**
   * Get memory usage breakdown by repository.
   */
  virtual void GetMemoryByRepository(IResourceManager* manager,
                                      std::unordered_map<std::string, std::size_t>& outData) = 0;
};

/**
 * Resource debug manager interface.
 * Main interface for all debug functionality.
 */
class IResourceDebugManager {
 public:
  virtual ~IResourceDebugManager() = default;
  
  //==========================================================================
  // Component Access
  //==========================================================================
  
  /**
   * Get the profiler instance.
   */
  virtual IResourceProfiler* GetProfiler() = 0;
  
  /**
   * Get the leak detector instance.
   */
  virtual IResourceLeakDetector* GetLeakDetector() = 0;
  
  /**
   * Get the debug visualizer instance.
   */
  virtual IResourceDebugVisualizer* GetVisualizer() = 0;
  
  //==========================================================================
  // Logging
  //==========================================================================
  
  /**
   * Set debug log level.
   */
  virtual void SetLogLevel(ResourceDebugLogLevel level) = 0;
  
  /**
   * Get current debug log level.
   */
  virtual ResourceDebugLogLevel GetLogLevel() const = 0;
  
  /**
   * Subscribe to debug log messages.
   */
  virtual void* SubscribeToLog(ResourceDebugLogCallback callback, 
                                void* user_data) = 0;
  
  /**
   * Unsubscribe from debug log messages.
   */
  virtual void UnsubscribeFromLog(void* subscription) = 0;
  
  /**
   * Log a debug message.
   */
  virtual void Log(ResourceDebugLogLevel level, std::string const& message) = 0;
  
  //==========================================================================
  // Utility
  //==========================================================================
  
  /**
   * Dump debug info to a file.
   * @param manager Resource manager
   * @param filePath Output file path
   * @return true on success
   */
  virtual bool DumpDebugInfo(IResourceManager* manager, 
                             std::string const& filePath) = 0;
  
  /**
   * Generate a report of the resource system state.
   */
  virtual std::string GenerateReport(IResourceManager* manager) = 0;
  
  //==========================================================================
  // Breakpoints
  //==========================================================================
  
  /**
   * Set a breakpoint for a specific resource.
   * The debugger will break when this resource is loaded.
   */
  virtual void SetLoadBreakpoint(ResourceId id, bool enabled = true) = 0;
  
  /**
   * Clear a load breakpoint.
   */
  virtual void ClearLoadBreakpoint(ResourceId id) = 0;
  
  /**
   * Set a breakpoint for a specific resource type.
   */
  virtual void SetTypeBreakpoint(ResourceType type, bool enabled = true) = 0;
  
  /**
   * Clear a type breakpoint.
   */
  virtual void ClearTypeBreakpoint(ResourceType type) = 0;
  
  /**
   * Check if a breakpoint should trigger.
   */
  virtual bool ShouldBreakOnLoad(ResourceId id, ResourceType type) = 0;
};

/**
 * Get the global resource debug manager.
 */
IResourceDebugManager* GetResourceDebugManager();

/**
 * Scoped profiler helper.
 * Automatically records load time.
 */
class ScopedResourceProfiler {
 public:
  ScopedResourceProfiler(IResourceProfiler* profiler, 
                         ResourceId id, 
                         std::string const& path,
                         ResourceType type)
    : profiler_(profiler)
    , id_(id)
    , success_(false) {
    if (profiler_) {
      profiler_->BeginLoad(id, path, type);
    }
  }
  
  ~ScopedResourceProfiler() {
    if (profiler_) {
      profiler_->EndLoad(id_, success_);
    }
  }
  
  void SetSuccess(bool success) { success_ = success; }
  
 private:
  IResourceProfiler* profiler_;
  ResourceId id_;
  bool success_;
};

}  // namespace resource
}  // namespace te

#endif  // TE_RESOURCE_RESOURCE_DEBUG_H
