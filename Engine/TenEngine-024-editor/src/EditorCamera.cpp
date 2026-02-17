/**
 * @file EditorCamera.cpp
 * @brief Editor camera controller implementation (024-Editor).
 */
#include <te/editor/EditorCamera.h>
#include <te/core/math.h>
#include <cmath>
#include <cstring>

namespace te {
namespace editor {

class EditorCameraImpl : public IEditorCamera {
public:
  EditorCameraImpl()
    : m_position(0.0f, 5.0f, -10.0f)
    , m_yaw(0.0f)
    , m_pitch(-0.3f)
    , m_fov(60.0f)
    , m_nearClip(0.1f)
    , m_farClip(1000.0f)
    , m_viewportWidth(1280)
    , m_viewportHeight(720)
    , m_moveSpeed(10.0f)
    , m_rotationSpeed(0.003f)
    , m_navigationMode(CameraNavigationMode::Fly)
    , m_orbitTarget(0.0f)
    , m_orbitDistance(10.0f)
    , m_rightMouseDown(false)
    , m_leftMouseDown(false)
    , m_middleMouseDown(false)
    , m_altDown(false)
    , m_lastMouseX(0)
    , m_lastMouseY(0)
  {
    std::memset(m_keysDown, 0, sizeof(m_keysDown));
    std::memset(m_bookmarks, 0, sizeof(m_bookmarks));
    UpdateVectors();
  }

  // === Viewport Setup ===
  
  void SetViewportSize(int w, int h) override {
    m_viewportWidth = w;
    m_viewportHeight = h;
  }
  
  int GetViewportWidth() const override { return m_viewportWidth; }
  int GetViewportHeight() const override { return m_viewportHeight; }
  
  // === Input Handling ===
  
  void OnInput(float deltaTime) override {
    // Fly mode movement when right mouse is held
    if (m_rightMouseDown && m_navigationMode == CameraNavigationMode::Fly) {
      float speed = m_moveSpeed * deltaTime;
      
      // Shift to speed up
      if (m_keysDown[340]) speed *= 3.0f;  // GLFW_KEY_LEFT_SHIFT
      
      te::math::Vec3 movement(0.0f);
      
      if (m_keysDown[87]) movement += m_forward * speed;      // W
      if (m_keysDown[83]) movement -= m_forward * speed;      // S
      if (m_keysDown[65]) movement -= m_right * speed;        // A
      if (m_keysDown[68]) movement += m_right * speed;        // D
      if (m_keysDown[81]) movement -= te::math::Vec3(0, 1, 0) * speed;  // Q
      if (m_keysDown[69]) movement += te::math::Vec3(0, 1, 0) * speed;  // E
      
      m_position = m_position + movement;
    }
  }
  
  void OnMouseDown(int button, int x, int y) override {
    m_lastMouseX = x;
    m_lastMouseY = y;
    
    if (button == 0) m_leftMouseDown = true;
    if (button == 1) m_rightMouseDown = true;
    if (button == 2) m_middleMouseDown = true;
  }
  
  void OnMouseUp(int button) override {
    if (button == 0) m_leftMouseDown = false;
    if (button == 1) m_rightMouseDown = false;
    if (button == 2) m_middleMouseDown = false;
  }
  
  void OnMouseMove(int x, int y) override {
    int dx = x - m_lastMouseX;
    int dy = y - m_lastMouseY;
    
    m_lastMouseX = x;
    m_lastMouseY = y;
    
    // Orbit mode (Alt + left drag)
    if (m_leftMouseDown && m_altDown) {
      OrbitCamera(static_cast<float>(dx), static_cast<float>(dy));
    }
    // Fly mode rotation (right drag)
    else if (m_rightMouseDown) {
      RotateCamera(static_cast<float>(dx), static_cast<float>(dy));
    }
    // Pan mode (middle drag)
    else if (m_middleMouseDown) {
      PanCamera(static_cast<float>(dx), static_cast<float>(dy));
    }
  }
  
  void OnMouseWheel(float delta) override {
    // Zoom by moving camera forward or adjusting orbit distance
    if (m_navigationMode == CameraNavigationMode::Orbit || m_altDown) {
      m_orbitDistance -= delta * 0.5f;
      m_orbitDistance = std::max(0.1f, m_orbitDistance);
      UpdatePositionFromOrbit();
    } else {
      // Move camera forward
      m_position = m_position + m_forward * delta;
    }
  }
  
  void OnKeyDown(int key) override {
    if (key >= 0 && key < 512) {
      m_keysDown[key] = true;
    }
    
    // Check for Alt
    if (key == 342 || key == 346) {  // GLFW_KEY_LEFT_ALT / RIGHT_ALT
      m_altDown = true;
    }
  }
  
  void OnKeyUp(int key) override {
    if (key >= 0 && key < 512) {
      m_keysDown[key] = false;
    }
    
    // Check for Alt release
    if (key == 342 || key == 346) {
      m_altDown = false;
    }
  }
  
  // === Matrices ===
  
  te::math::Mat4 GetViewMatrix() const override {
    te::math::Vec3 target = m_position + m_forward;
    return te::math::LookAt(m_position, target, te::math::Vec3(0, 1, 0));
  }
  
  te::math::Mat4 GetProjectionMatrix() const override {
    float aspect = static_cast<float>(m_viewportWidth) / 
                   static_cast<float>(std::max(1, m_viewportHeight));
    return te::math::Perspective(te::math::ToRadians(m_fov), aspect, m_nearClip, m_farClip);
  }
  
  te::math::Mat4 GetViewProjectionMatrix() const override {
    return GetProjectionMatrix() * GetViewMatrix();
  }
  
  // === Camera Properties ===
  
  te::math::Vec3 GetPosition() const override { return m_position; }
  
  void SetPosition(te::math::Vec3 const& pos) override {
    m_position = pos;
    UpdateVectors();
  }
  
  te::math::Vec3 GetForward() const override { return m_forward; }
  te::math::Vec3 GetRight() const override { return m_right; }
  te::math::Vec3 GetUp() const override { return m_up; }
  
  float GetYaw() const override { return m_yaw; }
  float GetPitch() const override { return m_pitch; }
  
  void SetRotation(float yaw, float pitch) override {
    m_yaw = yaw;
    m_pitch = std::clamp(pitch, -te::math::PIDIV2 + 0.01f, te::math::PIDIV2 - 0.01f);
    UpdateVectors();
  }
  
  // === Projection Settings ===
  
  void SetFOV(float fov) override { m_fov = fov; }
  float GetFOV() const override { return m_fov; }
  
  void SetNearClip(float nearClip) override { m_nearClip = nearClip; }
  float GetNearClip() const override { return m_nearClip; }
  
  void SetFarClip(float farClip) override { m_farClip = farClip; }
  float GetFarClip() const override { return m_farClip; }
  
  // === Navigation ===
  
  void FocusOn(te::math::Vec3 const& point) override {
    m_orbitTarget = point;
    
    // Calculate distance from current position
    te::math::Vec3 toTarget = m_position - point;
    m_orbitDistance = te::math::Length(toTarget);
    if (m_orbitDistance < 0.1f) m_orbitDistance = 5.0f;
    
    // Update yaw and pitch to look at target
    te::math::Vec3 dir = te::math::Normalize(toTarget);
    m_yaw = std::atan2(dir.x, dir.z);
    m_pitch = std::asin(-dir.y);
    
    UpdateVectors();
    UpdatePositionFromOrbit();
  }
  
  void ResetView() override {
    m_position = te::math::Vec3(0.0f, 5.0f, -10.0f);
    m_yaw = 0.0f;
    m_pitch = -0.3f;
    m_orbitTarget = te::math::Vec3(0.0f);
    m_orbitDistance = 10.0f;
    UpdateVectors();
  }
  
  void FrameSelection() override {
    // Placeholder - would need selection manager integration
    FocusOn(te::math::Vec3(0.0f));
  }
  
  void SetNavigationMode(CameraNavigationMode mode) override {
    m_navigationMode = mode;
  }
  
  CameraNavigationMode GetNavigationMode() const override {
    return m_navigationMode;
  }
  
  // === Movement Speed ===
  
  void SetMoveSpeed(float speed) override { m_moveSpeed = speed; }
  float GetMoveSpeed() const override { return m_moveSpeed; }
  
  void SetRotationSpeed(float speed) override { m_rotationSpeed = speed; }
  float GetRotationSpeed() const override { return m_rotationSpeed; }
  
  // === Orbit Mode ===
  
  void SetOrbitTarget(te::math::Vec3 const& target) override {
    m_orbitTarget = target;
    UpdatePositionFromOrbit();
  }
  
  te::math::Vec3 GetOrbitTarget() const override { return m_orbitTarget; }
  
  void SetOrbitDistance(float distance) override {
    m_orbitDistance = std::max(0.1f, distance);
    UpdatePositionFromOrbit();
  }
  
  float GetOrbitDistance() const override { return m_orbitDistance; }
  
  // === Bookmarks ===
  
  void SaveBookmark(int slot) override {
    if (slot < 0 || slot >= 10) return;
    
    m_bookmarks[slot].position = m_position;
    m_bookmarks[slot].rotation = te::math::Vec3(m_yaw, m_pitch, 0.0f);
    m_bookmarks[slot].target = m_orbitTarget;
    m_bookmarks[slot].distance = m_orbitDistance;
    m_bookmarks[slot].name[0] = '\0';
  }
  
  bool LoadBookmark(int slot) override {
    if (slot < 0 || slot >= 10) return false;
    if (m_bookmarks[slot].name[0] == '\0' && 
        m_bookmarks[slot].position.x == 0.0f &&
        m_bookmarks[slot].position.y == 0.0f &&
        m_bookmarks[slot].position.z == 0.0f) {
      return false;  // Bookmark not set
    }
    
    m_position = m_bookmarks[slot].position;
    m_yaw = m_bookmarks[slot].rotation.x;
    m_pitch = m_bookmarks[slot].rotation.y;
    m_orbitTarget = m_bookmarks[slot].target;
    m_orbitDistance = m_bookmarks[slot].distance;
    UpdateVectors();
    return true;
  }
  
  bool HasBookmark(int slot) const override {
    if (slot < 0 || slot >= 10) return false;
    return m_bookmarks[slot].name[0] != '\0' || 
           m_bookmarks[slot].position.x != 0.0f ||
           m_bookmarks[slot].position.y != 0.0f ||
           m_bookmarks[slot].position.z != 0.0f;
  }
  
  CameraBookmark const* GetBookmark(int slot) const override {
    if (slot < 0 || slot >= 10) return nullptr;
    return &m_bookmarks[slot];
  }
  
  // === Ray Casting ===
  
  te::math::Ray ScreenPointToRay(int x, int y) const override {
    float ndcX = (2.0f * static_cast<float>(x)) / static_cast<float>(m_viewportWidth) - 1.0f;
    float ndcY = 1.0f - (2.0f * static_cast<float>(y)) / static_cast<float>(m_viewportHeight);
    
    te::math::Mat4 invProj = te::math::Inverse(GetProjectionMatrix());
    te::math::Mat4 invView = te::math::Inverse(GetViewMatrix());
    
    te::math::Vec4 rayClip(ndcX, ndcY, -1.0f, 1.0f);
    te::math::Vec4 rayEye = invProj * rayClip;
    rayEye = te::math::Vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);
    te::math::Vec4 rayWorld = invView * rayEye;
    
    te::math::Vec3 direction = te::math::Normalize(te::math::Vec3(rayWorld.x, rayWorld.y, rayWorld.z));
    
    te::math::Ray ray;
    ray.origin = m_position;
    ray.direction = direction;
    return ray;
  }
  
  te::math::Vec3 ScreenToWorld(int x, int y) const override {
    float ndcX = (2.0f * static_cast<float>(x)) / static_cast<float>(m_viewportWidth) - 1.0f;
    float ndcY = 1.0f - (2.0f * static_cast<float>(y)) / static_cast<float>(m_viewportHeight);
    
    te::math::Mat4 invProj = te::math::Inverse(GetProjectionMatrix());
    te::math::Mat4 invView = te::math::Inverse(GetViewMatrix());
    
    te::math::Vec4 rayClip(ndcX, ndcY, 0.0f, 1.0f);
    te::math::Vec4 rayEye = invProj * rayClip;
    rayEye.z = -1.0f;
    te::math::Vec4 rayWorld = invView * rayEye;
    
    return te::math::Vec3(rayWorld.x, rayWorld.y, rayWorld.z);
  }
  
  te::math::Vec2 WorldToScreen(te::math::Vec3 const& worldPos) const override {
    te::math::Vec4 clipPos = GetViewProjectionMatrix() * te::math::Vec4(worldPos, 1.0f);
    
    if (std::abs(clipPos.w) < 0.0001f) {
      return te::math::Vec2(0.0f, 0.0f);
    }
    
    te::math::Vec3 ndcPos = te::math::Vec3(clipPos.x, clipPos.y, clipPos.z) / clipPos.w;
    
    return te::math::Vec2(
      (ndcPos.x + 1.0f) * 0.5f * static_cast<float>(m_viewportWidth),
      (1.0f - ndcPos.y) * 0.5f * static_cast<float>(m_viewportHeight)
    );
  }

private:
  void UpdateVectors() {
    // Calculate forward vector from yaw and pitch
    float cosPitch = std::cos(m_pitch);
    float sinPitch = std::sin(m_pitch);
    float cosYaw = std::cos(m_yaw);
    float sinYaw = std::sin(m_yaw);
    
    m_forward.x = cosPitch * sinYaw;
    m_forward.y = sinPitch;
    m_forward.z = cosPitch * cosYaw;
    m_forward = te::math::Normalize(m_forward);
    
    // Right vector (assuming world up is Y)
    m_right.x = cosYaw;
    m_right.y = 0.0f;
    m_right.z = -sinYaw;
    m_right = te::math::Normalize(m_right);
    
    // Up vector
    m_up = te::math::Cross(m_forward, m_right);
    m_up = te::math::Normalize(m_up);
  }
  
  void RotateCamera(float dx, float dy) {
    m_yaw += dx * m_rotationSpeed;
    m_pitch += dy * m_rotationSpeed;
    
    // Clamp pitch
    m_pitch = std::clamp(m_pitch, -te::math::PIDIV2 + 0.01f, te::math::PIDIV2 - 0.01f);
    
    UpdateVectors();
  }
  
  void OrbitCamera(float dx, float dy) {
    m_yaw += dx * m_rotationSpeed;
    m_pitch += dy * m_rotationSpeed;
    
    // Clamp pitch
    m_pitch = std::clamp(m_pitch, -te::math::PIDIV2 + 0.01f, te::math::PIDIV2 - 0.01f);
    
    UpdateVectors();
    UpdatePositionFromOrbit();
  }
  
  void PanCamera(float dx, float dy) {
    float panSpeed = 0.01f * m_orbitDistance;
    
    m_position = m_position - m_right * dx * panSpeed;
    m_position = m_position + m_up * dy * panSpeed;
    m_orbitTarget = m_orbitTarget - m_right * dx * panSpeed;
    m_orbitTarget = m_orbitTarget + m_up * dy * panSpeed;
  }
  
  void UpdatePositionFromOrbit() {
    float cosPitch = std::cos(m_pitch);
    float sinPitch = std::sin(m_pitch);
    float cosYaw = std::cos(m_yaw);
    float sinYaw = std::sin(m_yaw);
    
    m_position.x = m_orbitTarget.x + m_orbitDistance * cosPitch * sinYaw;
    m_position.y = m_orbitTarget.y + m_orbitDistance * sinPitch;
    m_position.z = m_orbitTarget.z + m_orbitDistance * cosPitch * cosYaw;
    
    UpdateVectors();
  }
  
  // Camera state
  te::math::Vec3 m_position;
  te::math::Vec3 m_forward;
  te::math::Vec3 m_right;
  te::math::Vec3 m_up;
  float m_yaw;
  float m_pitch;
  
  // Projection
  float m_fov;
  float m_nearClip;
  float m_farClip;
  
  // Viewport
  int m_viewportWidth;
  int m_viewportHeight;
  
  // Speed
  float m_moveSpeed;
  float m_rotationSpeed;
  
  // Navigation
  CameraNavigationMode m_navigationMode;
  te::math::Vec3 m_orbitTarget;
  float m_orbitDistance;
  
  // Input state
  bool m_rightMouseDown;
  bool m_leftMouseDown;
  bool m_middleMouseDown;
  bool m_altDown;
  int m_lastMouseX;
  int m_lastMouseY;
  bool m_keysDown[512];
  
  // Bookmarks (Ctrl+1-9)
  CameraBookmark m_bookmarks[10];
};

IEditorCamera* CreateEditorCamera() {
  return new EditorCameraImpl();
}

}  // namespace editor
}  // namespace te
