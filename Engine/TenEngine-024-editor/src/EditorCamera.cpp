/**
 * @file EditorCamera.cpp
 * @brief Editor camera controller implementation (024-Editor).
 */
#include <te/editor/EditorCamera.h>
#include <te/core/math.h>
#include <algorithm>
#include <cmath>
#include <cstring>

namespace te {
namespace editor {

// Helper math functions (not in core::math)
namespace {

constexpr float PIDIV2 = 1.57079632679489661923f;

float ToRadians(float degrees) {
  return degrees * 0.017453292519943295769f;
}

math::Mat4 LookAt(math::Vec3 const& eye, math::Vec3 const& target, math::Vec3 const& up) {
  math::Mat4 result{};
  math::Vec3 f = Normalize(math::Vec3{target.x - eye.x, target.y - eye.y, target.z - eye.z});
  math::Vec3 s = Normalize(Cross(f, up));
  math::Vec3 u = Cross(s, f);

  result.m[0][0] = s.x; result.m[1][0] = s.y; result.m[2][0] = s.z; result.m[3][0] = 0;
  result.m[0][1] = u.x; result.m[1][1] = u.y; result.m[2][1] = u.z; result.m[3][1] = 0;
  result.m[0][2] = -f.x; result.m[1][2] = -f.y; result.m[2][2] = -f.z; result.m[3][2] = 0;
  result.m[0][3] = 0; result.m[1][3] = 0; result.m[2][3] = 0; result.m[3][3] = 1;

  // Translation
  result.m[3][0] = -(s.x * eye.x + s.y * eye.y + s.z * eye.z);
  result.m[3][1] = -(u.x * eye.x + u.y * eye.y + u.z * eye.z);
  result.m[3][2] = f.x * eye.x + f.y * eye.y + f.z * eye.z;

  return result;
}

math::Mat4 Perspective(float fovy, float aspect, float nearZ, float farZ) {
  math::Mat4 result{};
  float tanHalfFovy = std::tan(fovy * 0.5f);
  result.m[0][0] = 1.0f / (aspect * tanHalfFovy);
  result.m[1][1] = 1.0f / tanHalfFovy;
  result.m[2][2] = farZ / (nearZ - farZ);
  result.m[2][3] = -1.0f;
  result.m[3][2] = -(farZ * nearZ) / (farZ - nearZ);
  return result;
}

math::Mat4 Multiply(math::Mat4 const& a, math::Mat4 const& b) {
  math::Mat4 result{};
  for (int col = 0; col < 4; ++col) {
    for (int row = 0; row < 4; ++row) {
      result.m[col][row] = 0;
      for (int k = 0; k < 4; ++k) {
        result.m[col][row] += a.m[k][row] * b.m[col][k];
      }
    }
  }
  return result;
}

} // namespace

class EditorCameraImpl : public IEditorCamera {
public:
  EditorCameraImpl()
    : m_position{0.0f, 5.0f, -10.0f}
    , m_forward{0.0f, 0.0f, 1.0f}
    , m_right{1.0f, 0.0f, 0.0f}
    , m_up{0.0f, 1.0f, 0.0f}
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
    , m_orbitTarget{0.0f, 0.0f, 0.0f}
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
    if (m_rightMouseDown && m_navigationMode == CameraNavigationMode::Fly) {
      float speed = m_moveSpeed * deltaTime;
      if (m_keysDown[340]) speed *= 3.0f;  // GLFW_KEY_LEFT_SHIFT

      math::Vec3 movement{0.0f, 0.0f, 0.0f};

      if (m_keysDown[87]) { movement.x += m_forward.x * speed; movement.y += m_forward.y * speed; movement.z += m_forward.z * speed; }  // W
      if (m_keysDown[83]) { movement.x -= m_forward.x * speed; movement.y -= m_forward.y * speed; movement.z -= m_forward.z * speed; }  // S
      if (m_keysDown[65]) { movement.x -= m_right.x * speed; movement.y -= m_right.y * speed; movement.z -= m_right.z * speed; }        // A
      if (m_keysDown[68]) { movement.x += m_right.x * speed; movement.y += m_right.y * speed; movement.z += m_right.z * speed; }        // D
      if (m_keysDown[81]) { movement.y -= speed; }  // Q
      if (m_keysDown[69]) { movement.y += speed; }  // E

      m_position.x += movement.x;
      m_position.y += movement.y;
      m_position.z += movement.z;
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

    if (m_leftMouseDown && m_altDown) {
      OrbitCamera(static_cast<float>(dx), static_cast<float>(dy));
    } else if (m_rightMouseDown) {
      RotateCamera(static_cast<float>(dx), static_cast<float>(dy));
    } else if (m_middleMouseDown) {
      PanCamera(static_cast<float>(dx), static_cast<float>(dy));
    }
  }

  void OnMouseWheel(float delta) override {
    if (m_navigationMode == CameraNavigationMode::Orbit || m_altDown) {
      m_orbitDistance -= delta * 0.5f;
      m_orbitDistance = std::max(0.1f, m_orbitDistance);
      UpdatePositionFromOrbit();
    } else {
      m_position.x += m_forward.x * delta;
      m_position.y += m_forward.y * delta;
      m_position.z += m_forward.z * delta;
    }
  }

  void OnKeyDown(int key) override {
    if (key >= 0 && key < 512) m_keysDown[key] = true;
    if (key == 342 || key == 346) m_altDown = true;
  }

  void OnKeyUp(int key) override {
    if (key >= 0 && key < 512) m_keysDown[key] = false;
    if (key == 342 || key == 346) m_altDown = false;
  }

  // === Matrices ===

  math::Mat4 GetViewMatrix() const override {
    math::Vec3 target{m_position.x + m_forward.x, m_position.y + m_forward.y, m_position.z + m_forward.z};
    return LookAt(m_position, target, math::Vec3{0.0f, 1.0f, 0.0f});
  }

  math::Mat4 GetProjectionMatrix() const override {
    float aspect = static_cast<float>(m_viewportWidth) / static_cast<float>(std::max(1, m_viewportHeight));
    return Perspective(ToRadians(m_fov), aspect, m_nearClip, m_farClip);
  }

  math::Mat4 GetViewProjectionMatrix() const override {
    return Multiply(GetProjectionMatrix(), GetViewMatrix());
  }

  // === Camera Properties ===

  math::Vec3 GetPosition() const override { return m_position; }

  void SetPosition(math::Vec3 const& pos) override {
    m_position = pos;
    UpdateVectors();
  }

  math::Vec3 GetForward() const override { return m_forward; }
  math::Vec3 GetRight() const override { return m_right; }
  math::Vec3 GetUp() const override { return m_up; }

  float GetYaw() const override { return m_yaw; }
  float GetPitch() const override { return m_pitch; }

  void SetRotation(float yaw, float pitch) override {
    m_yaw = yaw;
    m_pitch = std::clamp(pitch, -PIDIV2 + 0.01f, PIDIV2 - 0.01f);
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

  void FocusOn(math::Vec3 const& point) override {
    m_orbitTarget = point;
    math::Vec3 toTarget{m_position.x - point.x, m_position.y - point.y, m_position.z - point.z};
    m_orbitDistance = Length(toTarget);
    if (m_orbitDistance < 0.1f) m_orbitDistance = 5.0f;

    math::Vec3 dir = Normalize(toTarget);
    m_yaw = std::atan2(dir.x, dir.z);
    m_pitch = std::asin(-dir.y);

    UpdateVectors();
    UpdatePositionFromOrbit();
  }

  void ResetView() override {
    m_position = {0.0f, 5.0f, -10.0f};
    m_yaw = 0.0f;
    m_pitch = -0.3f;
    m_orbitTarget = {0.0f, 0.0f, 0.0f};
    m_orbitDistance = 10.0f;
    UpdateVectors();
  }

  void FrameSelection() override {
    FocusOn(math::Vec3{0.0f, 0.0f, 0.0f});
  }

  void SetNavigationMode(CameraNavigationMode mode) override { m_navigationMode = mode; }
  CameraNavigationMode GetNavigationMode() const override { return m_navigationMode; }

  // === Movement Speed ===

  void SetMoveSpeed(float speed) override { m_moveSpeed = speed; }
  float GetMoveSpeed() const override { return m_moveSpeed; }

  void SetRotationSpeed(float speed) override { m_rotationSpeed = speed; }
  float GetRotationSpeed() const override { return m_rotationSpeed; }

  // === Orbit Mode ===

  void SetOrbitTarget(math::Vec3 const& target) override {
    m_orbitTarget = target;
    UpdatePositionFromOrbit();
  }

  math::Vec3 GetOrbitTarget() const override { return m_orbitTarget; }

  void SetOrbitDistance(float distance) override {
    m_orbitDistance = std::max(0.1f, distance);
    UpdatePositionFromOrbit();
  }

  float GetOrbitDistance() const override { return m_orbitDistance; }

  // === Bookmarks ===

  void SaveBookmark(int slot) override {
    if (slot < 0 || slot >= 10) return;
    m_bookmarks[slot].position[0] = m_position.x;
    m_bookmarks[slot].position[1] = m_position.y;
    m_bookmarks[slot].position[2] = m_position.z;
    m_bookmarks[slot].rotation[0] = m_yaw;
    m_bookmarks[slot].rotation[1] = m_pitch;
    m_bookmarks[slot].rotation[2] = 0.0f;
    m_bookmarks[slot].rotation[3] = 1.0f;
    m_bookmarks[slot].target[0] = m_orbitTarget.x;
    m_bookmarks[slot].target[1] = m_orbitTarget.y;
    m_bookmarks[slot].target[2] = m_orbitTarget.z;
    m_bookmarks[slot].distance = m_orbitDistance;
    m_bookmarks[slot].name.clear();
  }

  bool LoadBookmark(int slot) override {
    if (slot < 0 || slot >= 10) return false;
    if (m_bookmarks[slot].name.empty() &&
        m_bookmarks[slot].position[0] == 0.0f &&
        m_bookmarks[slot].position[1] == 0.0f &&
        m_bookmarks[slot].position[2] == 0.0f) {
      return false;
    }

    m_position.x = m_bookmarks[slot].position[0];
    m_position.y = m_bookmarks[slot].position[1];
    m_position.z = m_bookmarks[slot].position[2];
    m_yaw = m_bookmarks[slot].rotation[0];
    m_pitch = m_bookmarks[slot].rotation[1];
    m_orbitTarget.x = m_bookmarks[slot].target[0];
    m_orbitTarget.y = m_bookmarks[slot].target[1];
    m_orbitTarget.z = m_bookmarks[slot].target[2];
    m_orbitDistance = m_bookmarks[slot].distance;
    UpdateVectors();
    return true;
  }

  bool HasBookmark(int slot) const override {
    if (slot < 0 || slot >= 10) return false;
    return !m_bookmarks[slot].name.empty() ||
           m_bookmarks[slot].position[0] != 0.0f ||
           m_bookmarks[slot].position[1] != 0.0f ||
           m_bookmarks[slot].position[2] != 0.0f;
  }

  CameraBookmark const* GetBookmark(int slot) const override {
    if (slot < 0 || slot >= 10) return nullptr;
    return &m_bookmarks[slot];
  }

  // === Ray Casting ===

  math::Ray ScreenPointToRay(int x, int y) const override {
    math::Ray ray;
    ray.origin = m_position;

    // Simplified: just use forward direction
    float ndcX = (2.0f * static_cast<float>(x)) / static_cast<float>(m_viewportWidth) - 1.0f;
    float ndcY = 1.0f - (2.0f * static_cast<float>(y)) / static_cast<float>(m_viewportHeight);

    ray.direction = Normalize(math::Vec3{
      m_forward.x + m_right.x * ndcX + m_up.x * ndcY,
      m_forward.y + m_right.y * ndcX + m_up.y * ndcY,
      m_forward.z + m_right.z * ndcX + m_up.z * ndcY
    });
    return ray;
  }

  math::Vec3 ScreenToWorld(int x, int y) const override {
    // Simplified: return point on near plane
    float ndcX = (2.0f * static_cast<float>(x)) / static_cast<float>(m_viewportWidth) - 1.0f;
    float ndcY = 1.0f - (2.0f * static_cast<float>(y)) / static_cast<float>(m_viewportHeight);

    return math::Vec3{
      m_position.x + m_forward.x * m_nearClip + m_right.x * ndcX + m_up.x * ndcY,
      m_position.y + m_forward.y * m_nearClip + m_right.y * ndcX + m_up.y * ndcY,
      m_position.z + m_forward.z * m_nearClip + m_right.z * ndcX + m_up.z * ndcY
    };
  }

  math::Vec2 WorldToScreen(math::Vec3 const& worldPos) const override {
    // Simplified projection
    math::Vec3 rel{worldPos.x - m_position.x, worldPos.y - m_position.y, worldPos.z - m_position.z};
    float z = rel.x * m_forward.x + rel.y * m_forward.y + rel.z * m_forward.z;
    if (std::abs(z) < 0.0001f) return {0.0f, 0.0f};

    float x = rel.x * m_right.x + rel.y * m_right.y + rel.z * m_right.z;
    float y = rel.x * m_up.x + rel.y * m_up.y + rel.z * m_up.z;

    float fovRad = ToRadians(m_fov);
    float scale = static_cast<float>(m_viewportHeight) / (2.0f * std::tan(fovRad * 0.5f));

    return math::Vec2{
      static_cast<float>(m_viewportWidth) * 0.5f + x * scale / z,
      static_cast<float>(m_viewportHeight) * 0.5f - y * scale / z
    };
  }

private:
  void UpdateVectors() {
    float cosPitch = std::cos(m_pitch);
    float sinPitch = std::sin(m_pitch);
    float cosYaw = std::cos(m_yaw);
    float sinYaw = std::sin(m_yaw);

    m_forward.x = cosPitch * sinYaw;
    m_forward.y = sinPitch;
    m_forward.z = cosPitch * cosYaw;
    m_forward = Normalize(m_forward);

    m_right.x = cosYaw;
    m_right.y = 0.0f;
    m_right.z = -sinYaw;
    m_right = Normalize(m_right);

    m_up = Cross(m_forward, m_right);
    m_up = Normalize(m_up);
  }

  void RotateCamera(float dx, float dy) {
    m_yaw += dx * m_rotationSpeed;
    m_pitch += dy * m_rotationSpeed;
    m_pitch = std::clamp(m_pitch, -PIDIV2 + 0.01f, PIDIV2 - 0.01f);
    UpdateVectors();
  }

  void OrbitCamera(float dx, float dy) {
    m_yaw += dx * m_rotationSpeed;
    m_pitch += dy * m_rotationSpeed;
    m_pitch = std::clamp(m_pitch, -PIDIV2 + 0.01f, PIDIV2 - 0.01f);
    UpdateVectors();
    UpdatePositionFromOrbit();
  }

  void PanCamera(float dx, float dy) {
    float panSpeed = 0.01f * m_orbitDistance;

    m_position.x -= m_right.x * dx * panSpeed;
    m_position.y -= m_right.y * dx * panSpeed;
    m_position.z -= m_right.z * dx * panSpeed;
    m_position.x += m_up.x * dy * panSpeed;
    m_position.y += m_up.y * dy * panSpeed;
    m_position.z += m_up.z * dy * panSpeed;

    m_orbitTarget.x -= m_right.x * dx * panSpeed;
    m_orbitTarget.y -= m_right.y * dx * panSpeed;
    m_orbitTarget.z -= m_right.z * dx * panSpeed;
    m_orbitTarget.x += m_up.x * dy * panSpeed;
    m_orbitTarget.y += m_up.y * dy * panSpeed;
    m_orbitTarget.z += m_up.z * dy * panSpeed;
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
  math::Vec3 m_position;
  math::Vec3 m_forward;
  math::Vec3 m_right;
  math::Vec3 m_up;
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
  math::Vec3 m_orbitTarget;
  float m_orbitDistance;

  // Input state
  bool m_rightMouseDown;
  bool m_leftMouseDown;
  bool m_middleMouseDown;
  bool m_altDown;
  int m_lastMouseX;
  int m_lastMouseY;
  bool m_keysDown[512];

  // Bookmarks
  CameraBookmark m_bookmarks[10];
};

IEditorCamera* CreateEditorCamera() {
  return new EditorCameraImpl();
}

}  // namespace editor
}  // namespace te
