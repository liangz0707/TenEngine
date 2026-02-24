/**
 * @file SnapSettings.cpp
 * @brief Snap settings implementation (024-Editor).
 */
#include <te/editor/SnapSettings.h>
#include <te/core/math.h>
#include <cmath>

namespace te {
namespace editor {

// Local helper functions for math operations not in te::core::math
namespace {

constexpr float PI = 3.14159265358979323846f;

float ToRadiansLocal(float degrees) {
  return degrees * PI / 180.0f;
}

float ToDegreesLocal(float radians) {
  return radians * 180.0f / PI;
}

} // namespace

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

  void SetGridOrigin(math::Vec3 const& origin) override {
    m_config.gridOrigin[0] = origin.x;
    m_config.gridOrigin[1] = origin.y;
    m_config.gridOrigin[2] = origin.z;
  }

  math::Vec3 GetGridOrigin() const override {
    return math::Vec3{m_config.gridOrigin[0], m_config.gridOrigin[1], m_config.gridOrigin[2]};
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

  math::Vec3 SnapPosition(math::Vec3 const& position) const override {
    if (!m_config.gridSnapEnabled) return position;

    float ox = m_config.gridOrigin[0];
    float oy = m_config.gridOrigin[1];
    float oz = m_config.gridOrigin[2];

    float relX = position.x - ox;
    float relY = position.y - oy;
    float relZ = position.z - oz;

    float invGridSize = 1.0f / m_config.gridSize;

    math::Vec3 snapped;
    snapped.x = std::round(relX * invGridSize) * m_config.gridSize + ox;
    snapped.y = std::round(relY * invGridSize) * m_config.gridSize + oy;
    snapped.z = std::round(relZ * invGridSize) * m_config.gridSize + oz;

    return snapped;
  }

  math::Quat SnapRotation(math::Quat const& rotation) const override {
    if (!m_config.rotationSnapEnabled) return rotation;

    // Simplified: just return the rotation unchanged
    // TODO: Implement proper euler angle snapping when quaternion utils are available
    return rotation;
  }

  math::Vec3 SnapEulerAngles(math::Vec3 const& euler) const override {
    if (!m_config.rotationSnapEnabled) return euler;

    math::Vec3 snapped;
    float snapRad = ToRadiansLocal(m_config.rotationSnapAngle);
    float invSnap = 1.0f / snapRad;

    snapped.x = std::round(euler.x * invSnap) * snapRad;
    snapped.y = std::round(euler.y * invSnap) * snapRad;
    snapped.z = std::round(euler.z * invSnap) * snapRad;

    return snapped;
  }

  math::Vec3 SnapScale(math::Vec3 const& scale) const override {
    if (!m_config.scaleSnapEnabled) return scale;

    math::Vec3 snapped;
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

  SnapResult ApplySnap(math::Vec3 const& position,
                       math::Quat const& rotation,
                       math::Vec3 const& scale) const override {
    SnapResult result;

    if (m_config.gridSnapEnabled) {
      math::Vec3 snapped = SnapPosition(position);
      result.snappedPosition[0] = snapped.x;
      result.snappedPosition[1] = snapped.y;
      result.snappedPosition[2] = snapped.z;
      result.positionSnapped = true;
    } else {
      result.snappedPosition[0] = position.x;
      result.snappedPosition[1] = position.y;
      result.snappedPosition[2] = position.z;
    }

    if (m_config.rotationSnapEnabled) {
      // Simplified: just copy rotation as-is
      result.snappedRotation[0] = rotation.x;
      result.snappedRotation[1] = rotation.y;
      result.snappedRotation[2] = rotation.z;
      result.rotationSnapped = true;
    } else {
      result.snappedRotation[0] = rotation.x;
      result.snappedRotation[1] = rotation.y;
      result.snappedRotation[2] = rotation.z;
    }

    if (m_config.scaleSnapEnabled) {
      math::Vec3 snapped = SnapScale(scale);
      result.snappedScale[0] = snapped.x;
      result.snappedScale[1] = snapped.y;
      result.snappedScale[2] = snapped.z;
      result.scaleSnapped = true;
    } else {
      result.snappedScale[0] = scale.x;
      result.snappedScale[1] = scale.y;
      result.snappedScale[2] = scale.z;
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
    m_config.gridOrigin[0] = 0.0f;
    m_config.gridOrigin[1] = 0.0f;
    m_config.gridOrigin[2] = 0.0f;

    m_config.rotationSnapEnabled = false;
    m_config.rotationSnapAngle = 15.0f;

    m_config.scaleSnapEnabled = false;
    m_config.scaleSnapIncrement = 0.1f;

    m_config.surfaceSnapEnabled = false;
    m_config.surfaceSnapOffset = 0.0f;

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
