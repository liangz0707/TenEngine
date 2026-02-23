/**
 * @file StatisticsPanel.h
 * @brief Scene statistics panel interface (ABI IStatisticsPanel).
 */
#ifndef TE_EDITOR_STATISTICS_PANEL_H
#define TE_EDITOR_STATISTICS_PANEL_H

#include <te/editor/EditorTypes.h>
#include <te/core/math.h>
#include <functional>
#include <vector>

namespace te {
namespace editor {

// Math type aliases for convenience
namespace math {
using Vec3 = te::core::Vector3;
}

// Note: ComponentStats is defined in EditorTypes.h

/**
 * @brief Scene statistics panel interface.
 *
 * Displays statistics about the current scene including entity counts,
 * component breakdown, and scene bounds.
 */
class IStatisticsPanel {
public:
  virtual ~IStatisticsPanel() = default;

  // === Drawing ===

  /**
   * @brief Draw the statistics panel.
   */
  virtual void OnDraw() = 0;

  // === Entity Stats ===

  /**
   * @brief Set total entity count.
   */
  virtual void SetEntityCount(int count) = 0;

  /**
   * @brief Get total entity count.
   */
  virtual int GetEntityCount() const = 0;

  /**
   * @brief Set root entity count (top-level in hierarchy).
   */
  virtual void SetRootEntityCount(int count) = 0;

  /**
   * @brief Get root entity count.
   */
  virtual int GetRootEntityCount() const = 0;

  /**
   * @brief Set active entity count.
   */
  virtual void SetActiveEntityCount(int count) = 0;

  /**
   * @brief Get active entity count.
   */
  virtual int GetActiveEntityCount() const = 0;

  // === Component Stats ===

  /**
   * @brief Set component statistics.
   */
  virtual void SetComponentStats(std::vector<ComponentStats> const& stats) = 0;

  /**
   * @brief Get component statistics.
   */
  virtual std::vector<ComponentStats> const& GetComponentStats() const = 0;

  /**
   * @brief Set count for a specific component type.
   */
  virtual void SetComponentCount(const char* typeName, int count, int enabledCount = -1) = 0;

  /**
   * @brief Get count for a specific component type.
   */
  virtual int GetComponentCount(const char* typeName) const = 0;

  // === Scene Bounds ===

  /**
   * @brief Set scene bounding box.
   */
  virtual void SetSceneBounds(math::Vec3 const& min, math::Vec3 const& max) = 0;

  /**
   * @brief Get scene bounding box.
   */
  virtual void GetSceneBounds(math::Vec3& min, math::Vec3& max) const = 0;

  /**
   * @brief Get scene center point.
   */
  virtual math::Vec3 GetSceneCenter() const = 0;

  /**
   * @brief Get scene extents (half-size).
   */
  virtual math::Vec3 GetSceneExtents() const = 0;

  // === Batch Update ===

  /**
   * @brief Update all stats at once.
   */
  virtual void UpdateStats(SceneStats const& stats) = 0;
  
  /**
   * @brief Get all stats.
   */
  virtual SceneStats GetStats() const = 0;
  
  // === Refresh ===
  
  /**
   * @brief Request refresh of statistics (for async update).
   */
  virtual void RequestRefresh() = 0;
  
  /**
   * @brief Set callback for refresh requests.
   */
  virtual void SetOnRefreshRequested(std::function<void()> callback) = 0;
  
  /**
   * @brief Set auto-refresh interval (0 = manual only).
   */
  virtual void SetAutoRefreshInterval(float seconds) = 0;
  
  /**
   * @brief Get auto-refresh interval.
   */
  virtual float GetAutoRefreshInterval() const = 0;
  
  // === Visibility ===
  
  /**
   * @brief Show/hide the statistics panel.
   */
  virtual void SetVisible(bool visible) = 0;
  
  /**
   * @brief Check if statistics panel is visible.
   */
  virtual bool IsVisible() const = 0;
};

/**
 * @brief Factory function to create statistics panel.
 */
IStatisticsPanel* CreateStatisticsPanel();

}  // namespace editor
}  // namespace te

#endif  // TE_EDITOR_STATISTICS_PANEL_H
