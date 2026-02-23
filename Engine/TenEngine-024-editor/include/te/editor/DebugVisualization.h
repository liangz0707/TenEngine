/**
 * @file DebugVisualization.h
 * @brief Debug visualization interface for collision, navigation, etc.
 */
#ifndef TE_EDITOR_DEBUG_VISUALIZATION_H
#define TE_EDITOR_DEBUG_VISUALIZATION_H

#include <te/editor/EditorTypes.h>
#include <te/core/math.h>
#include <cstdint>

namespace te {
namespace editor {

// Math type aliases for convenience
namespace math {
using Vec3 = te::core::Vector3;
}

/**
 * @brief Debug visualization flags.
 */
enum class DebugVisFlags : uint32_t {
  None = 0,
  Collision = 1 << 0,           ///< Show collision shapes
  Navigation = 1 << 1,          ///< Show navigation mesh
  Wireframe = 1 << 2,           ///< Show wireframe overlay
  Normals = 1 << 3,             ///< Show vertex normals
  Bounds = 1 << 4,              ///< Show bounding boxes
  Overdraw = 1 << 5,            ///< Show overdraw visualization
  LightComplexity = 1 << 6,     ///< Show light complexity
  ShaderComplexity = 1 << 7,    ///< Show shader complexity
  Occlusion = 1 << 8,           ///< Show occlusion culling
  Physics = 1 << 9,             ///< Show physics debug
  AI = 1 << 10,                 ///< Show AI debug
  All = 0xFFFFFFFF
};

inline DebugVisFlags operator|(DebugVisFlags a, DebugVisFlags b) {
  return static_cast<DebugVisFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline DebugVisFlags operator&(DebugVisFlags a, DebugVisFlags b) {
  return static_cast<DebugVisFlags>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

inline bool HasFlag(DebugVisFlags flags, DebugVisFlags flag) {
  return (static_cast<uint32_t>(flags) & static_cast<uint32_t>(flag)) != 0;
}

/**
 * @brief Collision visualization settings.
 */
struct CollisionVisSettings {
  bool showColliders = true;
  bool showTriggers = true;
  bool showStatic = true;
  bool showDynamic = true;
  bool showContacts = true;
  bool showSleeping = false;
  
  math::Vec3 colliderColor = math::Vec3(0.0f, 1.0f, 0.0f);
  math::Vec3 triggerColor = math::Vec3(1.0f, 1.0f, 0.0f);
  math::Vec3 contactColor = math::Vec3(1.0f, 0.0f, 0.0f);
  
  float opacity = 0.5f;
  float lineWidth = 2.0f;
};

/**
 * @brief Navigation visualization settings.
 */
struct NavigationVisSettings {
  bool showMesh = true;
  bool showLinks = true;
  bool showObstacles = true;
  bool showPath = true;
  bool showLabels = false;
  
  math::Vec3 walkableColor = math::Vec3(0.0f, 0.5f, 1.0f);
  math::Vec3 obstacleColor = math::Vec3(1.0f, 0.0f, 0.0f);
  math::Vec3 linkColor = math::Vec3(1.0f, 0.5f, 0.0f);
  math::Vec3 pathColor = math::Vec3(0.0f, 1.0f, 0.0f);
  
  float opacity = 0.7f;
};

/**
 * @brief Debug visualization system interface.
 * 
 * Provides visualization overlays for debugging collision,
 * navigation, and other systems.
 */
class IDebugVisualization {
public:
  virtual ~IDebugVisualization() = default;
  
  // === Global Flags ===
  
  /**
   * @brief Set active visualization flags.
   */
  virtual void SetFlags(DebugVisFlags flags) = 0;
  
  /**
   * @brief Get active visualization flags.
   */
  virtual DebugVisFlags GetFlags() const = 0;
  
  /**
   * @brief Enable/disable a specific visualization.
   */
  virtual void EnableVisualization(DebugVisFlags flag, bool enable) = 0;
  
  /**
   * @brief Check if a visualization is enabled.
   */
  virtual bool IsVisualizationEnabled(DebugVisFlags flag) const = 0;
  
  /**
   * @brief Toggle a visualization.
   */
  virtual void ToggleVisualization(DebugVisFlags flag) = 0;
  
  // === Collision Settings ===
  
  /**
   * @brief Set collision visualization settings.
   */
  virtual void SetCollisionSettings(CollisionVisSettings const& settings) = 0;
  
  /**
   * @brief Get collision visualization settings.
   */
  virtual CollisionVisSettings const& GetCollisionSettings() const = 0;
  
  // === Navigation Settings ===
  
  /**
   * @brief Set navigation visualization settings.
   */
  virtual void SetNavigationSettings(NavigationVisSettings const& settings) = 0;
  
  /**
   * @brief Get navigation visualization settings.
   */
  virtual NavigationVisSettings const& GetNavigationSettings() const = 0;
  
  // === Rendering ===
  
  /**
   * @brief Draw active visualizations.
   * Called by the viewport renderer.
   */
  virtual void OnDraw() = 0;
  
  /**
   * @brief Draw collision debug for a specific entity.
   */
  virtual void DrawEntityCollision(uint64_t entityId) = 0;
  
  /**
   * @brief Draw navigation mesh.
   */
  virtual void DrawNavigationMesh() = 0;
  
  /**
   * @brief Draw bounds for a specific entity.
   */
  virtual void DrawEntityBounds(uint64_t entityId, math::Vec3 const& color) = 0;
  
  // === Debug Draw Primitives ===
  
  /**
   * @brief Draw a debug line.
   */
  virtual void DrawLine(math::Vec3 const& start, math::Vec3 const& end, 
                        math::Vec3 const& color, float lifetime = 0.0f) = 0;
  
  /**
   * @brief Draw a debug box.
   */
  virtual void DrawBox(math::Vec3 const& center, math::Vec3 const& halfExtents,
                       math::Vec3 const& color, float lifetime = 0.0f) = 0;
  
  /**
   * @brief Draw a debug sphere.
   */
  virtual void DrawSphere(math::Vec3 const& center, float radius,
                          math::Vec3 const& color, float lifetime = 0.0f) = 0;
  
  /**
   * @brief Draw a debug cylinder.
   */
  virtual void DrawCylinder(math::Vec3 const& center, float radius, float height,
                            math::Vec3 const& color, float lifetime = 0.0f) = 0;
  
  /**
   * @brief Draw a debug capsule.
   */
  virtual void DrawCapsule(math::Vec3 const& center, float radius, float height,
                           math::Vec3 const& color, float lifetime = 0.0f) = 0;
  
  /**
   * @brief Draw a debug arrow.
   */
  virtual void DrawArrow(math::Vec3 const& start, math::Vec3 const& end,
                         math::Vec3 const& color, float lifetime = 0.0f) = 0;
  
  /**
   * @brief Draw debug text in world space.
   */
  virtual void DrawText(math::Vec3 const& position, const char* text,
                        math::Vec3 const& color, float lifetime = 0.0f) = 0;
  
  // === Persistence ===
  
  /**
   * @brief Clear all debug drawings.
   */
  virtual void ClearDrawings() = 0;
  
  /**
   * @brief Clear drawings older than a certain time.
   */
  virtual void ClearOldDrawings(float maxAge) = 0;
  
  // === UI ===
  
  /**
   * @brief Draw visualization settings UI.
   */
  virtual void OnDrawUI() = 0;
};

/**
 * @brief Factory function to create debug visualization system.
 */
IDebugVisualization* CreateDebugVisualization();

}  // namespace editor
}  // namespace te

#endif  // TE_EDITOR_DEBUG_VISUALIZATION_H
