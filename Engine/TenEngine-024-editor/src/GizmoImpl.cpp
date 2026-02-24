/**
 * @file GizmoImpl.cpp
 * @brief Gizmo transformation tool implementation (024-Editor).
 */
#include <te/editor/Gizmo.h>
#include <te/editor/EntityAdapter.h>
#include <te/entity/Entity.h>
#include <te/scene/SceneTypes.h>
#include <te/core/math.h>
#include <cmath>

namespace te {
namespace editor {

class GizmoImpl : public IGizmo {
public:
  GizmoImpl()
    : m_mode(GizmoMode::Translate)
    , m_space(GizmoSpace::World)
    , m_target(nullptr)
    , m_hovered(false)
    , m_active(false)
    , m_size(100.0f)
    , m_viewportWidth(800)
    , m_viewportHeight(600)
    , m_activeAxis(0)
  {
    m_hoverState = {};
    m_initialTransform = {};
  }

  // === Mode Control ===
  
  void SetMode(GizmoMode mode) override {
    m_mode = mode;
  }
  
  GizmoMode GetMode() const override {
    return m_mode;
  }
  
  void SetSpace(GizmoSpace space) override {
    m_space = space;
  }
  
  GizmoSpace GetSpace() const override {
    return m_space;
  }
  
  // === Target ===
  
  void SetTarget(IEntity* entity) override {
    m_target = entity;
    if (entity && entity->GetEntity()) {
      m_initialTransform = entity->GetEntity()->GetLocalTransform();
    }
  }
  
  IEntity* GetTarget() const override {
    return m_target;
  }
  
  // === Input Handling ===
  
  bool OnMouseDown(int x, int y) override {
    if (!m_target || !m_hovered) return false;
    
    m_active = true;
    m_dragStartX = x;
    m_dragStartY = y;
    
    if (m_target && m_target->GetEntity()) {
      m_initialTransform = m_target->GetEntity()->GetLocalTransform();
    }
    
    return true;
  }
  
  void OnMouseMove(int x, int y, float dx, float dy) override {
    if (!m_target) {
      m_hovered = false;
      return;
    }

    if (m_active && m_target && m_target->GetEntity()) {
      // Use the provided delta values for smoother transformation
      ApplyTransformWithDelta(dx, dy);
    } else {
      UpdateHoverState(x, y);
    }
  }
  
  void OnMouseUp() override {
    m_active = false;
    m_activeAxis = 0;
  }
  
  // === State Query ===
  
  bool IsHovered() const override {
    return m_hovered;
  }
  
  bool IsActive() const override {
    return m_active;
  }
  
  GizmoHoverState GetHoverState() const override {
    return m_hoverState;
  }
  
  // === Rendering ===

  void OnDraw() override {
    // TODO: Gizmo rendering integration with Pipeline module
    // This requires:
    // 1. Access to IRenderPipeline or debug renderer interface
    // 2. Drawing colored lines for each axis (X=red, Y=green, Z=blue)
    // 3. Drawing rotation rings for rotate mode
    // 4. Drawing scale boxes for scale mode
    // 5. Highlight hovered axes
    //
    // Current implementation is a placeholder - the transform logic
    // in ApplyTransformWithDelta works, but visual gizmo rendering
    // requires Pipeline integration.
  }
  
  void SetSize(float size) override {
    m_size = size;
  }
  
  float GetSize() const override {
    return m_size;
  }
  
  // === Camera Context ===
  
  void SetCameraMatrices(te::core::Matrix4 const& view, te::core::Matrix4 const& projection) override {
    m_view = view;
    m_projection = projection;
    // TODO: Implement Matrix4 multiplication and inverse in te::core::math
    // m_viewProjection = m_projection * m_view;
    // m_inverseViewProjection = te::core::Inverse(m_viewProjection);
    // For now, store view and projection separately for WorldToScreen
    (void)m_viewProjection;
    (void)m_inverseViewProjection;
  }
  
  void SetViewportSize(int width, int height) override {
    m_viewportWidth = width;
    m_viewportHeight = height;
  }

private:
  void UpdateHoverState(int x, int y) {
    m_hoverState = {};
    m_hovered = false;

    if (!m_target || !m_target->GetEntity()) return;

    te::scene::Transform targetTransform = m_target->GetEntity()->GetLocalTransform();
    te::core::Vector3 targetPos = targetTransform.position;
    te::core::Vector2 screenPos = WorldToScreen(targetPos);

    float dist = std::sqrt((x - screenPos.x) * (x - screenPos.x) +
                           (y - screenPos.y) * (y - screenPos.y));

    // Simple proximity check for hover
    if (dist < m_size) {
      m_hovered = true;

      // Determine which axis is closest based on relative position
      float relX = static_cast<float>(x) - screenPos.x;
      float relY = static_cast<float>(y) - screenPos.y;

      float absX = std::abs(relX);
      float absY = std::abs(relY);

      float threshold = m_size * 0.3f;

      if (m_mode == GizmoMode::Translate || m_mode == GizmoMode::Scale) {
        if (absX > absY && absX > threshold) {
          m_hoverState.hoveringX = true;
        } else if (absY > absX && absY > threshold) {
          m_hoverState.hoveringY = true;
        } else if (dist < m_size * 0.3f) {
          m_hoverState.hoveringXYZ = true;
        }
      } else if (m_mode == GizmoMode::Rotate) {
        // Rotation gizmo is a ring
        if (dist < m_size * 0.7f && dist > m_size * 0.3f) {
          m_hoverState.hoveringXYZ = true;
        }
      }
    }
  }
  
  void ApplyTransform(int x, int y) {
    if (!m_target || !m_target->GetEntity()) return;

    te::entity::Entity* entity = m_target->GetEntity();
    te::scene::Transform transform = m_initialTransform;

    float dx = static_cast<float>(x - m_dragStartX);
    float dy = static_cast<float>(y - m_dragStartY);

    // Convert screen delta to world delta (simplified)
    float worldScale = 0.01f; // Pixels to world units

    switch (m_mode) {
      case GizmoMode::Translate: {
        te::core::Vector3 delta{0.0f, 0.0f, 0.0f};

        if (m_hoverState.hoveringX || m_hoverState.hoveringXYZ) {
          delta.x = dx * worldScale;
        }
        if (m_hoverState.hoveringY || m_hoverState.hoveringXYZ) {
          delta.y = -dy * worldScale;
        }
        if (m_hoverState.hoveringZ) {
          delta.z = (dx + dy) * worldScale * 0.5f;
        }

        transform.position.x = m_initialTransform.position.x + delta.x;
        transform.position.y = m_initialTransform.position.y + delta.y;
        transform.position.z = m_initialTransform.position.z + delta.z;
        break;
      }

      case GizmoMode::Rotate: {
        float angle = (dx + dy) * 0.5f * 0.01f; // Radians

        // TODO: Implement QuatFromAxisAngle in te::core::math
        // For now, use simplified Euler rotation
        te::core::Quaternion rot{0.0f, 0.0f, 0.0f, 1.0f};
        if (m_hoverState.hoveringX) {
          // Rotation around X axis
          float halfAngle = angle * 0.5f;
          rot.x = std::sin(halfAngle);
          rot.w = std::cos(halfAngle);
        } else if (m_hoverState.hoveringY) {
          // Rotation around Y axis
          float halfAngle = angle * 0.5f;
          rot.y = std::sin(halfAngle);
          rot.w = std::cos(halfAngle);
        } else if (m_hoverState.hoveringZ || m_hoverState.hoveringXYZ) {
          // Rotation around Z axis
          float halfAngle = angle * 0.5f;
          rot.z = std::sin(halfAngle);
          rot.w = std::cos(halfAngle);
        }

        // TODO: Implement quaternion multiplication
        // For now, just apply the rotation directly (simplified)
        transform.rotation = rot;
        break;
      }

      case GizmoMode::Scale: {
        float scaleFactor = 1.0f + (dx + dy) * 0.005f;
        scaleFactor = std::max(0.001f, scaleFactor);

        if (m_hoverState.hoveringX) {
          transform.scale.x = m_initialTransform.scale.x * scaleFactor;
        } else if (m_hoverState.hoveringY) {
          transform.scale.y = m_initialTransform.scale.y * scaleFactor;
        } else if (m_hoverState.hoveringZ) {
          transform.scale.z = m_initialTransform.scale.z * scaleFactor;
        } else if (m_hoverState.hoveringXYZ) {
          transform.scale.x = m_initialTransform.scale.x * scaleFactor;
          transform.scale.y = m_initialTransform.scale.y * scaleFactor;
          transform.scale.z = m_initialTransform.scale.z * scaleFactor;
        }
        break;
      }
    }

    entity->SetLocalTransform(transform);
  }

  void ApplyTransformWithDelta(float dx, float dy) {
    if (!m_target || !m_target->GetEntity()) return;

    te::entity::Entity* entity = m_target->GetEntity();
    te::scene::Transform transform = entity->GetLocalTransform();

    // Convert screen delta to world delta
    float worldScale = 0.01f;

    switch (m_mode) {
      case GizmoMode::Translate: {
        te::core::Vector3 delta{0.0f, 0.0f, 0.0f};

        if (m_hoverState.hoveringX || m_hoverState.hoveringXYZ) {
          delta.x = dx * worldScale;
        }
        if (m_hoverState.hoveringY || m_hoverState.hoveringXYZ) {
          delta.y = -dy * worldScale;
        }
        if (m_hoverState.hoveringZ) {
          delta.z = (dx + dy) * worldScale * 0.5f;
        }

        transform.position.x += delta.x;
        transform.position.y += delta.y;
        transform.position.z += delta.z;
        break;
      }

      case GizmoMode::Rotate: {
        float angle = (dx + dy) * 0.5f * 0.01f;

        // TODO: Implement QuatFromAxisAngle in te::core::math
        te::core::Quaternion rot{0.0f, 0.0f, 0.0f, 1.0f};
        if (m_hoverState.hoveringX) {
          float halfAngle = angle * 0.5f;
          rot.x = std::sin(halfAngle);
          rot.w = std::cos(halfAngle);
        } else if (m_hoverState.hoveringY) {
          float halfAngle = angle * 0.5f;
          rot.y = std::sin(halfAngle);
          rot.w = std::cos(halfAngle);
        } else {
          float halfAngle = angle * 0.5f;
          rot.z = std::sin(halfAngle);
          rot.w = std::cos(halfAngle);
        }

        // TODO: Implement quaternion multiplication for proper rotation composition
        // For now, apply rotation directly (simplified behavior)
        transform.rotation = rot;
        break;
      }

      case GizmoMode::Scale: {
        float scaleFactor = 1.0f + (dx + dy) * 0.005f;
        scaleFactor = std::max(0.001f, scaleFactor);

        if (m_hoverState.hoveringX) {
          transform.scale.x *= scaleFactor;
        } else if (m_hoverState.hoveringY) {
          transform.scale.y *= scaleFactor;
        } else if (m_hoverState.hoveringZ) {
          transform.scale.z *= scaleFactor;
        } else {
          transform.scale.x *= scaleFactor;
          transform.scale.y *= scaleFactor;
          transform.scale.z *= scaleFactor;
        }
        break;
      }
    }

    entity->SetLocalTransform(transform);
  }
  
  te::core::Vector2 WorldToScreen(te::core::Vector3 const& worldPos) const {
    // TODO: Implement proper WorldToScreen once Matrix4 multiplication is available
    // For now, return a simple projection based on view matrix
    // Simplified: just use view matrix translation component
    (void)worldPos;

    // Placeholder: return center of viewport
    return te::core::Vector2{
      static_cast<float>(m_viewportWidth) * 0.5f,
      static_cast<float>(m_viewportHeight) * 0.5f
    };
  }
  
  // State
  GizmoMode m_mode;
  GizmoSpace m_space;
  IEntity* m_target;
  bool m_hovered;
  bool m_active;
  float m_size;
  int m_viewportWidth;
  int m_viewportHeight;
  int m_activeAxis;
  
  // Hover state
  GizmoHoverState m_hoverState;
  
  // Drag state
  int m_dragStartX = 0;
  int m_dragStartY = 0;
  te::scene::Transform m_initialTransform;
  
  // Camera matrices
  te::core::Matrix4 m_view;
  te::core::Matrix4 m_projection;
  te::core::Matrix4 m_viewProjection;
  te::core::Matrix4 m_inverseViewProjection;
};

IGizmo* CreateGizmo() {
  return new GizmoImpl();
}

}  // namespace editor
}  // namespace te
