/**
 * @file ProfilerPanel.h
 * @brief Performance profiler panel interface (ABI IProfilerPanel).
 */
#ifndef TE_EDITOR_PROFILER_PANEL_H
#define TE_EDITOR_PROFILER_PANEL_H

#include <te/editor/EditorTypes.h>
#include <functional>
#include <vector>

namespace te {
namespace editor {

// Note: ProfilerScope and FrameTimingEntry are defined in EditorTypes.h

/**
 * @brief Performance profiler panel interface.
 *
 * Displays real-time performance metrics including CPU/GPU frame times,
 * draw call statistics, and a timeline view of profiler scopes.
 */
class IProfilerPanel {
public:
  virtual ~IProfilerPanel() = default;

  // === Drawing ===

  /**
   * @brief Draw the profiler panel.
   */
  virtual void OnDraw() = 0;

  // === Frame Stats ===

  /**
   * @brief Update with current frame statistics.
   */
  virtual void UpdateFrameStats(FrameStats const& stats) = 0;
  
  /**
   * @brief Get current frame statistics.
   */
  virtual FrameStats GetCurrentStats() const = 0;
  
  /**
   * @brief Get average frame time over recent frames.
   */
  virtual float GetAverageFrameTime() const = 0;
  
  /**
   * @brief Get min/max frame time over recent frames.
   */
  virtual void GetFrameTimeRange(float& min, float& max) const = 0;
  
  // === Profiling Scopes ===
  
  /**
   * @brief Begin a profiling scope.
   */
  virtual void BeginScope(const char* name, uint32_t color = 0xFFFFFFFF) = 0;
  
  /**
   * @brief End the current profiling scope.
   */
  virtual void EndScope() = 0;
  
  /**
   * @brief Clear all scope data for current frame.
   */
  virtual void ClearScopes() = 0;
  
  /**
   * @brief Get all scopes for current frame.
   */
  virtual std::vector<ProfilerScope> const& GetScopes() const = 0;
  
  // === History ===
  
  /**
   * @brief Set the number of frames to keep in history.
   */
  virtual void SetHistoryLength(int frames) = 0;
  
  /**
   * @brief Get the history length.
   */
  virtual int GetHistoryLength() const = 0;
  
  /**
   * @brief Get frame timing history.
   */
  virtual std::vector<FrameTimingEntry> const& GetHistory() const = 0;
  
  // === Display Options ===
  
  /**
   * @brief Set the frame time graph height.
   */
  virtual void SetGraphHeight(float height) = 0;
  
  /**
   * @brief Get the frame time graph height.
   */
  virtual float GetGraphHeight() const = 0;
  
  /**
   * @brief Enable/disable timeline view.
   */
  virtual void SetShowTimeline(bool show) = 0;
  
  /**
   * @brief Check if timeline view is enabled.
   */
  virtual bool IsShowTimelineEnabled() const = 0;
  
  /**
   * @brief Set the frame time warning threshold (ms).
   */
  virtual void SetWarningThreshold(float ms) = 0;
  
  /**
   * @brief Get the frame time warning threshold.
   */
  virtual float GetWarningThreshold() const = 0;
  
  // === Pause/Resume ===
  
  /**
   * @brief Pause profiling data collection.
   */
  virtual void Pause() = 0;
  
  /**
   * @brief Resume profiling data collection.
   */
  virtual void Resume() = 0;
  
  /**
   * @brief Check if profiling is paused.
   */
  virtual bool IsPaused() const = 0;
  
  // === Visibility ===
  
  /**
   * @brief Show/hide the profiler panel.
   */
  virtual void SetVisible(bool visible) = 0;
  
  /**
   * @brief Check if profiler panel is visible.
   */
  virtual bool IsVisible() const = 0;
};

/**
 * @brief Factory function to create profiler panel.
 */
IProfilerPanel* CreateProfilerPanel();

}  // namespace editor
}  // namespace te

#endif  // TE_EDITOR_PROFILER_PANEL_H
