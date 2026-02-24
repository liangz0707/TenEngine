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

// Note: FrameTimingStats, RenderStats, SceneStats, PhysicsStats,
// ViewportStatistics, and StatsDisplaySettings are defined in EditorTypes.h

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
  virtual void AddDrawCall(int triangles, int vertices) = 0;

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
  virtual int GetDrawCalls() const = 0;

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
   * @brief Set position (x, y screen coordinates).
   */
  virtual void SetPosition(float x, float y) = 0;

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
