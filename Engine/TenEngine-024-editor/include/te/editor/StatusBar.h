/**
 * @file StatusBar.h
 * @brief Status bar interface (ABI IStatusBar).
 */
#ifndef TE_EDITOR_STATUS_BAR_H
#define TE_EDITOR_STATUS_BAR_H

#include <te/editor/EditorTypes.h>
#include <functional>

namespace te {
namespace editor {

/**
 * @brief Background task info for status bar display.
 */
struct BackgroundTask {
  const char* name = nullptr;
  float progress = 0.0f;  ///< 0.0 to 1.0, -1 for indeterminate
  bool active = false;
  int id = 0;
};

/**
 * @brief Status bar interface.
 * 
 * Displays editor status information including current level name,
 * selection count, FPS, memory usage, and background task progress.
 */
class IStatusBar {
public:
  virtual ~IStatusBar() = default;
  
  // === Drawing ===
  
  /**
   * @brief Draw the status bar.
   */
  virtual void OnDraw() = 0;
  
  // === Level Info ===
  
  /**
   * @brief Set the current level name.
   */
  virtual void SetLevelName(const char* name) = 0;
  
  /**
   * @brief Get the current level name.
   */
  virtual const char* GetLevelName() const = 0;
  
  // === Selection Info ===
  
  /**
   * @brief Set the selection count.
   */
  virtual void SetSelectionCount(size_t count) = 0;
  
  /**
   * @brief Get the selection count.
   */
  virtual size_t GetSelectionCount() const = 0;
  
  // === Performance Stats ===
  
  /**
   * @brief Set FPS display.
   */
  virtual void SetFPS(float fps) = 0;
  
  /**
   * @brief Set frame time display.
   */
  virtual void SetFrameTime(float ms) = 0;
  
  /**
   * @brief Set memory usage display (in MB).
   */
  virtual void SetMemoryUsage(size_t bytes) = 0;
  
  /**
   * @brief Update all frame stats at once.
   */
  virtual void SetFrameStats(FrameStats const& stats) = 0;
  
  // === Background Tasks ===
  
  /**
   * @brief Add or update a background task.
   * @return Task ID for future updates
   */
  virtual int AddBackgroundTask(const char* name, float progress = -1.0f) = 0;
  
  /**
   * @brief Update background task progress.
   */
  virtual void UpdateBackgroundTask(int taskId, float progress) = 0;
  
  /**
   * @brief Remove a background task.
   */
  virtual void RemoveBackgroundTask(int taskId) = 0;
  
  /**
   * @brief Get all background tasks.
   */
  virtual std::vector<BackgroundTask> const& GetBackgroundTasks() const = 0;
  
  // === Status Messages ===
  
  /**
   * @brief Set a temporary status message.
   * @param message Status message text
   * @param duration Display duration in seconds (0 for default)
   */
  virtual void SetStatusMessage(const char* message, float duration = 0.0f) = 0;
  
  /**
   * @brief Clear the status message.
   */
  virtual void ClearStatusMessage() = 0;
  
  // === Visibility ===
  
  /**
   * @brief Show/hide the status bar.
   */
  virtual void SetVisible(bool visible) = 0;
  
  /**
   * @brief Check if status bar is visible.
   */
  virtual bool IsVisible() const = 0;
};

/**
 * @brief Factory function to create status bar.
 */
IStatusBar* CreateStatusBar();

}  // namespace editor
}  // namespace te

#endif  // TE_EDITOR_STATUS_BAR_H
