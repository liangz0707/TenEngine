/**
 * @file ViewportStats.h
 * @brief Viewport statistics overlay interface.
 */
#ifndef TE_EDITOR_VIEWPORT_STATS_H
#define TE_EDITOR_VIEWPORT_STATS_H

#include <te/editor/EditorTypes.h>
#include <te/core/math.h>
#include <cstdint>

namespace te {
namespace editor {

/**
 * @brief Frame timing statistics.
 */
struct FrameTimingStats {
  float cpuTimeMs = 0.0f;
  float gpuTimeMs = 0.0f;
  float totalFrameTimeMs = 0.0f;
  float fps = 0.0f;
  
  float cpuTimeHistory[60] = {};
  float gpuTimeHistory[60] = {};
  int historyIndex = 0;
};

/**
 * @brief Rendering statistics.
 */
struct RenderStats {
  uint32_t drawCalls = 0;
  uint32_t triangles = 0;
  uint32_t vertices = 0;
  uint32_t textureBindings = 0;
  uint32_t shaderBindings = 0;
  uint32_t bufferBindings = 0;
  
  // Memory
  uint64_t vramUsed = 0;
  uint64_t vramBudget = 0;
};

/**
 * @brief Scene statistics.
 */
struct SceneStats {
  uint32_t totalEntities = 0;
  uint32_t visibleEntities = 0;
  uint32_t culledEntities = 0;
  uint32_t dynamicEntities = 0;
  uint32_t staticEntities = 0;
  
  uint32_t totalLights = 0;
  uint32_t visibleLights = 0;
  uint32_t shadowCastingLights = 0;
  
  uint32_t totalCameras = 0;
  uint32_t totalMeshes = 0;
};

/**
 * @brief Physics statistics.
 */
struct PhysicsStats {
  uint32_t activeBodies = 0;
  uint32_t sleepingBodies = 0;
  uint32_t totalColliders = 0;
  uint32_t activeContacts = 0;
  float physicsTimeMs = 0.0f;
};

/**
 * @brief Complete viewport statistics.
 */
struct ViewportStatistics {
  FrameTimingStats timing;
  RenderStats render;
  SceneStats scene;
  PhysicsStats physics;
  
  uint64_t frameNumber = 0;
  double totalTime = 0.0;
};

/**
 * @brief Stats display settings.
 */
struct StatsDisplaySettings {
  bool showFPS = true;
  bool showFrameTime = true;
  bool showDrawCalls = true;
  bool showTriangleCount = true;
  bool showVertexCount = false;
  bool showMemoryUsage = true;
  bool showEntityCount = true;
  bool showLightCount = true;
  bool showPhysicsStats = false;
  bool showGraphs = true;
  
  // Position
  enum class Position { TopLeft, TopRight, BottomLeft, BottomRight };
  Position position = Position::TopLeft;
  
  // Style
  float opacity = 0.85f;
  bool showBackground = true;
  float fontSize = 14.0f;
};

/**
 * @brief Viewport statistics overlay interface.
 * 
 * Provides real-time statistics display in the viewport,
 * similar to Unity/Unreal stats overlay.
 */
class IViewportStats {
public:
  virtual ~IViewportStats() = default;
  
  // === Data Collection ===
  
  /**
   * @brief Begin a new frame.
   */
  virtual void BeginFrame() = 0;
  
  /**
   * @brief End the current frame.
   */
  virtual void EndFrame() = 0;
  
  /**
   * @brief Update timing stats.
   */
  virtual void UpdateTiming(float cpuTimeMs, float gpuTimeMs) = 0;
  
  /**
   * @brief Update render stats.
   */
  virtual void UpdateRenderStats(RenderStats const& stats) = 0;
  
  /**
   * @brief Update scene stats.
   */
  virtual void UpdateSceneStats(SceneStats const& stats) = 0;
  
  /**
   * @brief Update physics stats.
   */
  virtual void UpdatePhysicsStats(PhysicsStats const& stats) = 0;
  
  /**
   * @brief Add draw call.
   */
  virtual void AddDrawCall(uint32_t triangles, uint32_t vertices) = 0;
  
  /**
   * @brief Reset all stats.
   */
  virtual void ResetStats() = 0;
  
  // === Data Access ===
  
  /**
   * @brief Get current statistics.
   */
  virtual ViewportStatistics const& GetStats() const = 0;
  
  /**
   * @brief Get FPS.
   */
  virtual float GetFPS() const = 0;
  
  /**
   * @brief Get frame time.
   */
  virtual float GetFrameTimeMs() const = 0;
  
  /**
   * @brief Get draw call count.
   */
  virtual uint32_t GetDrawCalls() const = 0;
  
  // === Display Settings ===
  
  /**
   * @brief Set display settings.
   */
  virtual void SetDisplaySettings(StatsDisplaySettings const& settings) = 0;
  
  /**
   * @brief Get display settings.
   */
  virtual StatsDisplaySettings const& GetDisplaySettings() const = 0;
  
  /**
   * @brief Toggle stats visibility.
   */
  virtual void SetVisible(bool visible) = 0;
  
  /**
   * @brief Check if visible.
   */
  virtual bool IsVisible() const = 0;
  
  /**
   * @brief Set position.
   */
  virtual void SetPosition(StatsDisplaySettings::Position position) = 0;
  
  // === Rendering ===
  
  /**
   * @brief Draw the stats overlay.
   * Called by viewport renderer.
   */
  virtual void OnDraw() = 0;
  
  /**
   * @brief Draw timing graph.
   */
  virtual void DrawTimingGraph() = 0;
};

/**
 * @brief Factory function to create viewport stats.
 */
IViewportStats* CreateViewportStats();

}  // namespace editor
}  // namespace te

#endif  // TE_EDITOR_VIEWPORT_STATS_H
