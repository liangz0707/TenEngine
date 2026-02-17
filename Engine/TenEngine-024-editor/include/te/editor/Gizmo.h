/**
 * @file Gizmo.h
 * @brief Gizmo transformation tool interface (ABI IGizmo).
 */
#ifndef TE_EDITOR_GIZMO_H
#define TE_EDITOR_GIZMO_H

#include <te/editor/EditorTypes.h>
#include <te/core/math.h>

namespace te {
namespace editor {

class IEntity;

/**
 * @brief Gizmo axis hover state.
 */
struct GizmoHoverState {
  bool hoveringX = false;
  bool hoveringY = false;
  bool hoveringZ = false;
  bool hoveringXY = false;
  bool hoveringXZ = false;
  bool hoveringYZ = false;
  bool hoveringXYZ = false;  // Center (all axes)
};

/**
 * @brief Gizmo operation result.
 */
struct GizmoOperation {
  bool active = false;
  GizmoMode mode = GizmoMode::Translate;
  te::math::Vec3 delta;
  te::math::Vec3 newValue;
  GizmoHoverState hover;
};

/**
 * @brief Gizmo transformation tool interface.
 * 
 * Provides visual manipulation tools for translating, rotating, and scaling
 * selected entities in the viewport.
 */
class IGizmo {
public:
  virtual ~IGizmo() = default;
  
  // === Mode Control ===
  
  /**
   * @brief Set the gizmo transformation mode.
   * @param mode Translate, Rotate, or Scale
   */
  virtual void SetMode(GizmoMode mode) = 0;
  
  /**
   * @brief Get the current gizmo mode.
   */
  virtual GizmoMode GetMode() const = 0;
  
  /**
   * @brief Set the coordinate space for transformations.
   * @param space World or Local
   */
  virtual void SetSpace(GizmoSpace space) = 0;
  
  /**
   * @brief Get the current coordinate space.
   */
  virtual GizmoSpace GetSpace() const = 0;
  
  // === Target ===
  
  /**
   * @brief Set the entity to transform.
   * @param entity Target entity (nullptr to hide gizmo)
   */
  virtual void SetTarget(IEntity* entity) = 0;
  
  /**
   * @brief Get the current target entity.
   */
  virtual IEntity* GetTarget() const = 0;
  
  // === Input Handling ===
  
  /**
   * @brief Handle mouse button press.
   * @param x Mouse X position in viewport
   * @param y Mouse Y position in viewport
   * @return true if gizmo was hit and started dragging
   */
  virtual bool OnMouseDown(int x, int y) = 0;
  
  /**
   * @brief Handle mouse movement.
   * @param x Mouse X position
   * @param y Mouse Y position
   * @param dx Mouse delta X
   * @param dy Mouse delta Y
   */
  virtual void OnMouseMove(int x, int y, float dx, float dy) = 0;
  
  /**
   * @brief Handle mouse button release.
   */
  virtual void OnMouseUp() = 0;
  
  // === State Query ===
  
  /**
   * @brief Check if the mouse is hovering over the gizmo.
   */
  virtual bool IsHovered() const = 0;
  
  /**
   * @brief Check if the gizmo is actively being dragged.
   */
  virtual bool IsActive() const = 0;
  
  /**
   * @brief Get current hover state (which axes are hovered).
   */
  virtual GizmoHoverState GetHoverState() const = 0;
  
  // === Rendering ===
  
  /**
   * @brief Draw the gizmo (called each frame).
   * @note Rendering implementation depends on Pipeline module
   */
  virtual void OnDraw() = 0;
  
  /**
   * @brief Set the gizmo size in pixels.
   * @param size Size in pixels
   */
  virtual void SetSize(float size) = 0;
  
  /**
   * @brief Get the gizmo size in pixels.
   */
  virtual float GetSize() const = 0;
  
  // === Camera Context ===
  
  /**
   * @brief Set the view-projection matrix for gizmo calculation.
   * @param view View matrix
   * @param projection Projection matrix
   */
  virtual void SetCameraMatrices(te::math::Mat4 const& view, te::math::Mat4 const& projection) = 0;
  
  /**
   * @brief Set the viewport dimensions.
   * @param width Viewport width
   * @param height Viewport height
   */
  virtual void SetViewportSize(int width, int height) = 0;
};

/**
 * @brief Factory function to create a gizmo instance.
 */
IGizmo* CreateGizmo();

}  // namespace editor
}  // namespace te

#endif  // TE_EDITOR_GIZMO_H
