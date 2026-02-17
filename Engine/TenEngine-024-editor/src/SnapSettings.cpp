/**
 * @file SnapSettings.cpp
 * @brief Snap settings implementation (024-Editor).
 */
#include <te/editor/SnapSettings.h>
#include <te/core/math.h>
#include <cmath>

namespace te {
namespace editor {

class SnapSettingsImpl : public ISnapSettings {
public:
  SnapSettingsImpl() {
    ResetToDefaults();
  }

  // === Grid Snap ===
  
  void SetGridSnapEnabled(bool enabled) override {
    m_config.gridSnapEnabled = enabled;
  }
  
  bool IsGridSnapEnabled() const override {
    return m_config.gridSnapEnabled;
  }
  
  void SetGridSize(float size) override {
    m_config.gridSize = std::max(0.001f, size);
  }
  
  float GetGridSize() const override {
    return m_config.gridSize;
  }
  
  void SetGridOrigin(te::math::Vec3 const& origin) override {
    m_config.gridOrigin = origin;
  }
  
  te::math::Vec3 GetGridOrigin() const override {
    return m_config.gridOrigin;
  }
  
  // === Rotation Snap ===
  
  void SetRotationSnapEnabled(bool enabled) override {
    m_config.rotationSnapEnabled = enabled;
  }
  
  bool IsRotationSnapEnabled() const override {
    return m_config.rotationSnapEnabled;
  }
  
  void SetRotationSnapAngle(float angle) override {
    m_config.rotationSnapAngle = std::max(1.0f, std::min(90.0f, angle));
  }
  
  float GetRotationSnapAngle() const override {
    return m_config.rotationSnapAngle;
  }
  
  // === Scale Snap ===
  
  void SetScaleSnapEnabled(bool enabled) override {
    m_config.scaleSnapEnabled = enabled;
  }
  
  bool IsScaleSnapEnabled() const override {
    return m_config.scaleSnapEnabled;
  }
  
  void SetScaleSnapIncrement(float increment) override {
    m_config.scaleSnapIncrement = std::max(0.001f, increment);
  }
  
  float GetScaleSnapIncrement() const override {
    return m_config.scaleSnapIncrement;
  }
  
  // === Surface Snap ===
  
  void SetSurfaceSnapEnabled(bool enabled) override {
    m_config.surfaceSnapEnabled = enabled;
  }
  
  bool IsSurfaceSnapEnabled() const override {
    return m_config.surfaceSnapEnabled;
  }
  
  void SetSurfaceSnapOffset(float offset) override {
    m_config.surfaceSnapOffset = offset;
  }
  
  float GetSurfaceSnapOffset() const override {
    return m_config.surfaceSnapOffset;
  }
  
  // === Vertex/Edge Snap ===
  
  void SetVertexSnapEnabled(bool enabled) override {
    m_config.vertexSnapEnabled = enabled;
  }
  
  bool IsVertexSnapEnabled() const override {
    return m_config.vertexSnapEnabled;
  }
  
  void SetEdgeSnapEnabled(bool enabled) override {
    m_config.edgeSnapEnabled = enabled;
  }
  
  bool IsEdgeSnapEnabled() const override {
    return m_config.edgeSnapEnabled;
  }
  
  // === Snap Operations ===
  
  te::math::Vec3 SnapPosition(te::math::Vec3 const& position) const override {
    if (!m_config.gridSnapEnabled) return position;
    
    te::math::Vec3 relative = position - m_config.gridOrigin;
    te::math::Vec3 snapped;
    
    float invGridSize = 1.0f / m_config.gridSize;
    snapped.x = std::round(relative.x * invGridSize) * m_config.gridSize;
    snapped.y = std::round(relative.y * invGridSize) * m_config.gridSize;
    snapped.z = std::round(relative.z * invGridSize) * m_config.gridSize;
    
    return snapped + m_config.gridOrigin;
  }
  
  te::math::Quat SnapRotation(te::math::Quat const& rotation) const override {
    if (!m_config.rotationSnapEnabled) return rotation;
    
    // Convert to euler angles
    te::math::Vec3 euler = te::math::ToEulerAngles(rotation);
    te::math::Vec3 snappedEuler = SnapEulerAngles(euler);
    
    // Convert back to quaternion
    return te::math::FromEulerAngles(snappedEuler);
  }
  
  te::math::Vec3 SnapEulerAngles(te::math::Vec3 const& euler) const override {
    if (!m_config.rotationSnapEnabled) return euler;
    
    te::math::Vec3 snapped;
    float snapRad = te::math::ToRadians(m_config.rotationSnapAngle);
    float invSnap = 1.0f / snapRad;
    
    // Convert to degrees for snapping, then back to radians
    snapped.x = std::round(euler.x * invSnap) * snapRad;
    snapped.y = std::round(euler.y * invSnap) * snapRad;
    snapped.z = std::round(euler.z * invSnap) * snapRad;
    
    return snapped;
  }
  
  te::math::Vec3 SnapScale(te::math::Vec3 const& scale) const override {
    if (!m_config.scaleSnapEnabled) return scale;
    
    te::math::Vec3 snapped;
    float invIncrement = 1.0f / m_config.scaleSnapIncrement;
    
    snapped.x = std::round(scale.x * invIncrement) * m_config.scaleSnapIncrement;
    snapped.y = std::round(scale.y * invIncrement) * m_config.scaleSnapIncrement;
    snapped.z = std::round(scale.z * invIncrement) * m_config.scaleSnapIncrement;
    
    // Ensure minimum scale
    snapped.x = std::max(0.001f, snapped.x);
    snapped.y = std::max(0.001f, snapped.y);
    snapped.z = std::max(0.001f, snapped.z);
    
    return snapped;
  }
  
  SnapResult ApplySnap(te::math::Vec3 const& position,
                       te::math::Quat const& rotation,
                       te::math::Vec3 const& scale) const override {
    SnapResult result;
    
    if (m_config.gridSnapEnabled) {
      result.snappedPosition = SnapPosition(position);
      result.positionSnapped = true;
    } else {
      result.snappedPosition = position;
    }
    
    if (m_config.rotationSnapEnabled) {
      result.snappedRotation = SnapRotation(rotation);
      result.rotationSnapped = true;
    } else {
      result.snappedRotation = rotation;
    }
    
    if (m_config.scaleSnapEnabled) {
      result.snappedScale = SnapScale(scale);
      result.scaleSnapped = true;
    } else {
      result.snappedScale = scale;
    }
    
    return result;
  }
  
  // === Configuration ===
  
  SnapConfig GetConfig() const override {
    return m_config;
  }
  
  void SetConfig(SnapConfig const& config) override {
    m_config = config;
  }
  
  void ResetToDefaults() override {
    m_config.gridSnapEnabled = false;
    m_config.gridSize = 1.0f;
    m_config.gridOrigin = te::math::Vec3(0.0f);
    
    m_config.rotationSnapEnabled = false;
    m_config.rotationSnapAngle = 15.0f;
    
    m_config.scaleSnapEnabled = false;
    m_config.scaleSnapIncrement = 0.1f;
    
    m_config.surfaceSnapEnabled = false;
    m_config.surfaceSnapOffset = 0.0f;
    m_config.snapToBackface = false;
    
    m_config.vertexSnapEnabled = false;
    m_config.vertexSnapDistance = 0.1f;
    
    m_config.edgeSnapEnabled = false;
    m_config.edgeSnapDistance = 0.1f;
    
    m_config.pivotSnapEnabled = false;
  }
  
  // === Toggle All ===
  
  void ToggleAllSnap(bool enabled) override {
    m_config.gridSnapEnabled = enabled;
    m_config.rotationSnapEnabled = enabled;
    m_config.scaleSnapEnabled = enabled;
    // Don't toggle surface/vertex/edge snap - those are more specialized
  }
  
  bool IsAnySnapEnabled() const override {
    return m_config.gridSnapEnabled ||
           m_config.rotationSnapEnabled ||
           m_config.scaleSnapEnabled ||
           m_config.surfaceSnapEnabled ||
           m_config.vertexSnapEnabled ||
           m_config.edgeSnapEnabled;
  }

private:
  SnapConfig m_config;
};

ISnapSettings* CreateSnapSettings() {
  return new SnapSettingsImpl();
}

}  // namespace editor
}  // namespace te
