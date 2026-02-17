/**
 * @file EditorCamera.h
 * @brief Editor camera controller interface (ABI IEditorCamera).
 */
#ifndef TE_EDITOR_EDITOR_CAMERA_H
#define TE_EDITOR_EDITOR_CAMERA_H

#include <te/editor/EditorTypes.h>
#include <te/core/math.h>
#include <functional>

namespace te {
namespace editor {

/**
 * @brief Camera bookmark for saving/restoring camera positions.
 */
struct CameraBookmark {
  te::math::Vec3 position;
  te::math::Vec3 rotation;  // Euler angles in radians
  te::math::Vec3 target;    // Look-at point for orbit mode
  float distance;           // Distance from target for orbit
  char name[32] = "";
};

/**
 * @brief Editor camera controller interface.
 * 
 * Provides camera navigation for the viewport including fly mode,
 * orbit mode, zoom, and camera bookmarks.
 */
class IEditorCamera {
public:
  virtual ~IEditorCamera() = default;
  
  // === Viewport Setup ===
  
  /**
   * @brief Set the viewport dimensions.
   * @param w Viewport width in pixels
   * @param h Viewport height in pixels
   */
  virtual void SetViewportSize(int w, int h) = 0;
  
  /**
   * @brief Get viewport width.
   */
  virtual int GetViewportWidth() const = 0;
  
  /**
   * @brief Get viewport height.
   */
  virtual int GetViewportHeight() const = 0;
  
  // === Input Handling ===
  
  /**
   * @brief Process input for camera movement.
   * @param deltaTime Frame delta time in seconds
   */
  virtual void OnInput(float deltaTime) = 0;
  
  /**
   * @brief Handle mouse button press.
   * @param button Mouse button (0=left, 1=right, 2=middle)
   * @param x Mouse X position
   * @param y Mouse Y position
   */
  virtual void OnMouseDown(int button, int x, int y) = 0;
  
  /**
   * @brief Handle mouse button release.
   * @param button Mouse button
   */
  virtual void OnMouseUp(int button) = 0;
  
  /**
   * @brief Handle mouse movement.
   * @param x Mouse X position
   * @param y Mouse Y position
   */
  virtual void OnMouseMove(int x, int y) = 0;
  
  /**
   * @brief Handle mouse wheel scroll.
   * @param delta Scroll delta
   */
  virtual void OnMouseWheel(float delta) = 0;
  
  /**
   * @brief Handle key press.
   * @param key Key code
   */
  virtual void OnKeyDown(int key) = 0;
  
  /**
   * @brief Handle key release.
   * @param key Key code
   */
  virtual void OnKeyUp(int key) = 0;
  
  // === Matrices ===
  
  /**
   * @brief Get the view matrix.
   */
  virtual te::math::Mat4 GetViewMatrix() const = 0;
  
  /**
   * @brief Get the projection matrix.
   */
  virtual te::math::Mat4 GetProjectionMatrix() const = 0;
  
  /**
   * @brief Get combined view-projection matrix.
   */
  virtual te::math::Mat4 GetViewProjectionMatrix() const = 0;
  
  // === Camera Properties ===
  
  /**
   * @brief Get camera position in world space.
   */
  virtual te::math::Vec3 GetPosition() const = 0;
  
  /**
   * @brief Set camera position.
   */
  virtual void SetPosition(te::math::Vec3 const& pos) = 0;
  
  /**
   * @brief Get camera forward direction.
   */
  virtual te::math::Vec3 GetForward() const = 0;
  
  /**
   * @brief Get camera right direction.
   */
  virtual te::math::Vec3 GetRight() const = 0;
  
  /**
   * @brief Get camera up direction.
   */
  virtual te::math::Vec3 GetUp() const = 0;
  
  /**
   * @brief Get camera yaw angle in radians.
   */
  virtual float GetYaw() const = 0;
  
  /**
   * @brief Get camera pitch angle in radians.
   */
  virtual float GetPitch() const = 0;
  
  /**
   * @brief Set camera rotation from yaw and pitch.
   */
  virtual void SetRotation(float yaw, float pitch) = 0;
  
  // === Projection Settings ===
  
  /**
   * @brief Set field of view (in degrees).
   */
  virtual void SetFOV(float fov) = 0;
  
  /**
   * @brief Get field of view (in degrees).
   */
  virtual float GetFOV() const = 0;
  
  /**
   * @brief Set near clip plane distance.
   */
  virtual void SetNearClip(float nearClip) = 0;
  
  /**
   * @brief Get near clip plane distance.
   */
  virtual float GetNearClip() const = 0;
  
  /**
   * @brief Set far clip plane distance.
   */
  virtual void SetFarClip(float farClip) = 0;
  
  /**
   * @brief Get far clip plane distance.
   */
  virtual float GetFarClip() const = 0;
  
  // === Navigation ===
  
  /**
   * @brief Focus camera on a point in world space.
   * @param point World position to focus on
   */
  virtual void FocusOn(te::math::Vec3 const& point) = 0;
  
  /**
   * @brief Reset camera to default view.
   */
  virtual void ResetView() = 0;
  
  /**
   * @brief Frame selected objects in view.
   */
  virtual void FrameSelection() = 0;
  
  /**
   * @brief Set navigation mode.
   */
  virtual void SetNavigationMode(CameraNavigationMode mode) = 0;
  
  /**
   * @brief Get current navigation mode.
   */
  virtual CameraNavigationMode GetNavigationMode() const = 0;
  
  // === Movement Speed ===
  
  /**
   * @brief Set camera movement speed.
   */
  virtual void SetMoveSpeed(float speed) = 0;
  
  /**
   * @brief Get camera movement speed.
   */
  virtual float GetMoveSpeed() const = 0;
  
  /**
   * @brief Set camera rotation sensitivity.
   */
  virtual void SetRotationSpeed(float speed) = 0;
  
  /**
   * @brief Get camera rotation sensitivity.
   */
  virtual float GetRotationSpeed() const = 0;
  
  // === Orbit Mode ===
  
  /**
   * @brief Set orbit target point.
   */
  virtual void SetOrbitTarget(te::math::Vec3 const& target) = 0;
  
  /**
   * @brief Get orbit target point.
   */
  virtual te::math::Vec3 GetOrbitTarget() const = 0;
  
  /**
   * @brief Set orbit distance.
   */
  virtual void SetOrbitDistance(float distance) = 0;
  
  /**
   * @brief Get orbit distance.
   */
  virtual float GetOrbitDistance() const = 0;
  
  // === Bookmarks ===
  
  /**
   * @brief Save current camera state to bookmark slot.
   * @param slot Bookmark slot index (0-9)
   */
  virtual void SaveBookmark(int slot) = 0;
  
  /**
   * @brief Load camera state from bookmark slot.
   * @param slot Bookmark slot index (0-9)
   * @return true if bookmark existed
   */
  virtual bool LoadBookmark(int slot) = 0;
  
  /**
   * @brief Check if bookmark slot has a saved state.
   */
  virtual bool HasBookmark(int slot) const = 0;
  
  /**
   * @brief Get bookmark at slot.
   */
  virtual CameraBookmark const* GetBookmark(int slot) const = 0;
  
  // === Ray Casting ===
  
  /**
   * @brief Create a ray from screen point.
   * @param x Screen X position
   * @param y Screen Y position
   * @return Ray in world space
   */
  virtual te::math::Ray ScreenPointToRay(int x, int y) const = 0;
  
  /**
   * @brief Convert screen point to world position.
   * @param x Screen X position
   * @param y Screen Y position
   * @return World position on near plane
   */
  virtual te::math::Vec3 ScreenToWorld(int x, int y) const = 0;
  
  /**
   * @brief Convert world position to screen coordinates.
   * @param worldPos World position
   * @return Screen coordinates (x, y)
   */
  virtual te::math::Vec2 WorldToScreen(te::math::Vec3 const& worldPos) const = 0;
};

/**
 * @brief Factory function to create an editor camera.
 */
IEditorCamera* CreateEditorCamera();

}  // namespace editor
}  // namespace te

#endif  // TE_EDITOR_EDITOR_CAMERA_H
