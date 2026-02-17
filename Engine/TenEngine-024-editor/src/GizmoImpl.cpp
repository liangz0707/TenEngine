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
    (void)dx;
    (void)dy;
    
    if (!m_target) {
      m_hovered = false;
      return;
    }
    
    if (m_active && m_target && m_target->GetEntity()) {
      ApplyTransform(x, y);
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
    // Rendering depends on Pipeline module
    // This is a placeholder - actual rendering should be implemented
    // when Pipeline integration is available
  }
  
  void SetSize(float size) override {
    m_size = size;
  }
  
  float GetSize() const override {
    return m_size;
  }
  
  // === Camera Context ===
  
  void SetCameraMatrices(te::math::Mat4 const& view, te::math::Mat4 const& projection) override {
    m_view = view;
    m_projection = projection;
    m_viewProjection = m_projection * m_view;
    // Calculate inverse for unprojection
    m_inverseViewProjection = te::math::Inverse(m_viewProjection);
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
    
    te::math::Vec3 targetPos = m_target->GetEntity()->GetLocalTransform().position;
    te::math::Vec2 screenPos = WorldToScreen(targetPos);
    
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
        te::math::Vec3 delta(0.0f);
        
        if (m_hoverState.hoveringX || m_hoverState.hoveringXYZ) {
          delta.x = dx * worldScale;
        }
        if (m_hoverState.hoveringY || m_hoverState.hoveringXYZ) {
          delta.y = -dy * worldScale;
        }
        if (m_hoverState.hoveringZ) {
          delta.z = (dx + dy) * worldScale * 0.5f;
        }
        
        transform.position = m_initialTransform.position + delta;
        break;
      }
      
      case GizmoMode::Rotate: {
        float angle = (dx + dy) * 0.5f * 0.01f; // Radians
        
        if (m_hoverState.hoveringX) {
          te::math::Quat rot = te::math::QuatFromAxisAngle(
            te::math::Vec3(1.0f, 0.0f, 0.0f), angle);
          transform.rotation = rot * m_initialTransform.rotation;
        } else if (m_hoverState.hoveringY) {
          te::math::Quat rot = te::math::QuatFromAxisAngle(
            te::math::Vec3(0.0f, 1.0f, 0.0f), angle);
          transform.rotation = rot * m_initialTransform.rotation;
        } else if (m_hoverState.hoveringZ || m_hoverState.hoveringXYZ) {
          te::math::Quat rot = te::math::QuatFromAxisAngle(
            te::math::Vec3(0.0f, 0.0f, 1.0f), angle);
          transform.rotation = rot * m_initialTransform.rotation;
        }
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
          transform.scale = m_initialTransform.scale * scaleFactor;
        }
        break;
      }
    }
    
    entity->SetLocalTransform(transform);
  }
  
  te::math::Vec2 WorldToScreen(te::math::Vec3 const& worldPos) const {
    te::math::Vec4 clipPos = m_viewProjection * te::math::Vec4(worldPos, 1.0f);
    
    if (std::abs(clipPos.w) < 0.0001f) {
      return te::math::Vec2(0.0f, 0.0f);
    }
    
    te::math::Vec3 ndcPos = te::math::Vec3(clipPos.x, clipPos.y, clipPos.z) / clipPos.w;
    
    return te::math::Vec2(
      (ndcPos.x + 1.0f) * 0.5f * static_cast<float>(m_viewportWidth),
      (1.0f - ndcPos.y) * 0.5f * static_cast<float>(m_viewportHeight)
    );
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
  te::math::Mat4 m_view;
  te::math::Mat4 m_projection;
  te::math::Mat4 m_viewProjection;
  te::math::Mat4 m_inverseViewProjection;
};

IGizmo* CreateGizmo() {
  return new GizmoImpl();
}

}  // namespace editor
}  // namespace te
