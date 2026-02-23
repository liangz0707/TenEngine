/**
 * @file SnapSettings.h
 * @brief Snap settings interface for grid and transform snapping.
 */
#ifndef TE_EDITOR_SNAP_SETTINGS_H
#define TE_EDITOR_SNAP_SETTINGS_H

#include <te/editor/EditorTypes.h>
#include <te/core/math.h>

namespace te {
namespace editor {

// Math type aliases for convenience
namespace math {
using Vec3 = te::core::Vector3;
using Quat = te::core::Quaternion;
}

// Note: SnapConfig and SnapResult are defined in EditorTypes.h

/**
 * @brief Snap settings manager interface.
 * 
 * Manages grid and transform snapping settings for the editor.
 */
class ISnapSettings {
public:
  virtual ~ISnapSettings() = default;
  
  // === Grid Snap ===
  
  /**
   * @brief Enable/disable grid snapping.
   */
  virtual void SetGridSnapEnabled(bool enabled) = 0;
  
  /**
   * @brief Check if grid snapping is enabled.
   */
  virtual bool IsGridSnapEnabled() const = 0;
  
  /**
   * @brief Set grid size.
   */
  virtual void SetGridSize(float size) = 0;
  
  /**
   * @brief Get grid size.
   */
  virtual float GetGridSize() const = 0;
  
  /**
   * @brief Set grid origin.
   */
  virtual void SetGridOrigin(math::Vec3 const& origin) = 0;
  
  /**
   * @brief Get grid origin.
   */
  virtual math::Vec3 GetGridOrigin() const = 0;
  
  // === Rotation Snap ===
  
  /**
   * @brief Enable/disable rotation snapping.
   */
  virtual void SetRotationSnapEnabled(bool enabled) = 0;
  
  /**
   * @brief Check if rotation snapping is enabled.
   */
  virtual bool IsRotationSnapEnabled() const = 0;
  
  /**
   * @brief Set rotation snap angle (in degrees).
   */
  virtual void SetRotationSnapAngle(float angle) = 0;
  
  /**
   * @brief Get rotation snap angle (in degrees).
   */
  virtual float GetRotationSnapAngle() const = 0;
  
  // === Scale Snap ===
  
  /**
   * @brief Enable/disable scale snapping.
   */
  virtual void SetScaleSnapEnabled(bool enabled) = 0;
  
  /**
   * @brief Check if scale snapping is enabled.
   */
  virtual bool IsScaleSnapEnabled() const = 0;
  
  /**
   * @brief Set scale snap increment.
   */
  virtual void SetScaleSnapIncrement(float increment) = 0;
  
  /**
   * @brief Get scale snap increment.
   */
  virtual float GetScaleSnapIncrement() const = 0;
  
  // === Surface Snap ===
  
  /**
   * @brief Enable/disable surface snapping.
   */
  virtual void SetSurfaceSnapEnabled(bool enabled) = 0;
  
  /**
   * @brief Check if surface snapping is enabled.
   */
  virtual bool IsSurfaceSnapEnabled() const = 0;
  
  /**
   * @brief Set surface snap offset.
   */
  virtual void SetSurfaceSnapOffset(float offset) = 0;
  
  /**
   * @brief Get surface snap offset.
   */
  virtual float GetSurfaceSnapOffset() const = 0;
  
  // === Vertex/Edge Snap ===
  
  /**
   * @brief Enable/disable vertex snapping.
   */
  virtual void SetVertexSnapEnabled(bool enabled) = 0;
  
  /**
   * @brief Check if vertex snapping is enabled.
   */
  virtual bool IsVertexSnapEnabled() const = 0;
  
  /**
   * @brief Enable/disable edge snapping.
   */
  virtual void SetEdgeSnapEnabled(bool enabled) = 0;
  
  /**
   * @brief Check if edge snapping is enabled.
   */
  virtual bool IsEdgeSnapEnabled() const = 0;
  
  // === Snap Operations ===
  
  /**
   * @brief Snap a position to grid.
   */
  virtual math::Vec3 SnapPosition(math::Vec3 const& position) const = 0;
  
  /**
   * @brief Snap a rotation to nearest snap angle.
   */
  virtual math::Quat SnapRotation(math::Quat const& rotation) const = 0;
  
  /**
   * @brief Snap euler angles to nearest snap angle.
   */
  virtual math::Vec3 SnapEulerAngles(math::Vec3 const& euler) const = 0;
  
  /**
   * @brief Snap a scale to nearest increment.
   */
  virtual math::Vec3 SnapScale(math::Vec3 const& scale) const = 0;
  
  /**
   * @brief Apply all snapping to a transform.
   */
  virtual SnapResult ApplySnap(math::Vec3 const& position,
                               math::Quat const& rotation,
                               math::Vec3 const& scale) const = 0;
  
  // === Configuration ===
  
  /**
   * @brief Get the full snap configuration.
   */
  virtual SnapConfig GetConfig() const = 0;
  
  /**
   * @brief Set the full snap configuration.
   */
  virtual void SetConfig(SnapConfig const& config) = 0;
  
  /**
   * @brief Reset to default settings.
   */
  virtual void ResetToDefaults() = 0;
  
  // === Toggle All ===
  
  /**
   * @brief Toggle all snapping on/off.
   */
  virtual void ToggleAllSnap(bool enabled) = 0;
  
  /**
   * @brief Check if any snapping is enabled.
   */
  virtual bool IsAnySnapEnabled() const = 0;
};

/**
 * @brief Factory function to create snap settings.
 */
ISnapSettings* CreateSnapSettings();

}  // namespace editor
}  // namespace te

#endif  // TE_EDITOR_SNAP_SETTINGS_H
