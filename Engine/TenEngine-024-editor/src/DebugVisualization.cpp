/**
 * @file DebugVisualization.cpp
 * @brief Debug visualization implementation (024-Editor).
 */
#include <te/editor/DebugVisualization.h>
#include <te/core/log.h>
#include <imgui.h>
#include <vector>
#include <chrono>
#include <algorithm>

namespace te {
namespace editor {

struct DebugPrimitive {
  enum Type { Line, Box, Sphere, Cylinder, Capsule, Arrow, Text };
  Type type;
  math::Vec3 start;
  math::Vec3 end;
  math::Vec3 color;
  float param1 = 0.0f;  // radius, halfExtent, etc.
  float param2 = 0.0f;
  const char* text = nullptr;
  std::chrono::steady_clock::time_point createTime;
  float lifetime = 0.0f;
};

class DebugVisualizationImpl : public IDebugVisualization {
public:
  DebugVisualizationImpl()
    : m_flags(DebugVisFlags::None)
  {
  }

  // === Global Flags ===

  void SetFlags(DebugVisFlags flags) override {
    m_flags = flags;
  }

  DebugVisFlags GetFlags() const override {
    return m_flags;
  }

  void EnableVisualization(DebugVisFlags flag, bool enable) override {
    if (enable) {
      m_flags = m_flags | flag;
    } else {
      m_flags = static_cast<DebugVisFlags>(
        static_cast<uint32_t>(m_flags) & ~static_cast<uint32_t>(flag));
    }
  }

  bool IsVisualizationEnabled(DebugVisFlags flag) const override {
    return HasFlag(m_flags, flag);
  }

  void ToggleVisualization(DebugVisFlags flag) override {
    EnableVisualization(flag, !IsVisualizationEnabled(flag));
  }

  // === Collision Settings ===

  void SetCollisionSettings(CollisionVisSettings const& settings) override {
    m_collisionSettings = settings;
  }

  CollisionVisSettings const& GetCollisionSettings() const override {
    return m_collisionSettings;
  }

  // === Navigation Settings ===

  void SetNavigationSettings(NavigationVisSettings const& settings) override {
    m_navigationSettings = settings;
  }

  NavigationVisSettings const& GetNavigationSettings() const override {
    return m_navigationSettings;
  }

  // === Rendering ===

  void OnDraw() override {
    // TODO: Integrate with Pipeline module for actual debug rendering
    // This method should submit debug primitives to the render pipeline
    // using a debug draw interface. The pipeline will then render them
    // as overlay geometry in the viewport.

    auto now = std::chrono::steady_clock::now();

    // Draw primitives that haven't expired
    for (auto& primitive : m_primitives) {
      if (primitive.lifetime > 0.0f) {
        auto age = std::chrono::duration<float>(now - primitive.createTime).count();
        if (age > primitive.lifetime) {
          primitive.lifetime = 0.0f;  // Mark for removal
          continue;
        }
      }

      // TODO: Integrate with Pipeline to actually render primitives
      DrawPrimitive(primitive);
    }

    // Remove expired primitives
    m_primitives.erase(
      std::remove_if(m_primitives.begin(), m_primitives.end(),
                     [](DebugPrimitive const& p) { return p.lifetime == 0.0f; }),
      m_primitives.end());
  }

  void DrawEntityCollision(uint64_t entityId) override {
    // TODO: Integrate with physics system (014-Physics) to query and draw collision shapes
    (void)entityId;
  }

  void DrawNavigationMesh() override {
    // TODO: Integrate with navigation system to draw nav mesh
  }

  void DrawEntityBounds(uint64_t entityId, math::Vec3 const& color) override {
    // TODO: Get entity bounds from scene system and draw bounding box
    (void)entityId;
    (void)color;
  }

  // === Debug Draw Primitives ===

  void DrawLine(math::Vec3 const& start, math::Vec3 const& end,
                math::Vec3 const& color, float lifetime) override {
    DebugPrimitive p;
    p.type = DebugPrimitive::Line;
    p.start = start;
    p.end = end;
    p.color = color;
    p.lifetime = lifetime;
    p.createTime = std::chrono::steady_clock::now();
    m_primitives.push_back(p);
  }

  void DrawBox(math::Vec3 const& center, math::Vec3 const& halfExtents,
               math::Vec3 const& color, float lifetime) override {
    DebugPrimitive p;
    p.type = DebugPrimitive::Box;
    p.start = center;
    p.param1 = halfExtents.x;
    p.param2 = halfExtents.y;  // Store Z in end.x
    p.end.x = halfExtents.z;
    p.color = color;
    p.lifetime = lifetime;
    p.createTime = std::chrono::steady_clock::now();
    m_primitives.push_back(p);
  }

  void DrawSphere(math::Vec3 const& center, float radius,
                  math::Vec3 const& color, float lifetime) override {
    DebugPrimitive p;
    p.type = DebugPrimitive::Sphere;
    p.start = center;
    p.param1 = radius;
    p.color = color;
    p.lifetime = lifetime;
    p.createTime = std::chrono::steady_clock::now();
    m_primitives.push_back(p);
  }

  void DrawCylinder(math::Vec3 const& center, float radius, float height,
                    math::Vec3 const& color, float lifetime) override {
    DebugPrimitive p;
    p.type = DebugPrimitive::Cylinder;
    p.start = center;
    p.param1 = radius;
    p.param2 = height;
    p.color = color;
    p.lifetime = lifetime;
    p.createTime = std::chrono::steady_clock::now();
    m_primitives.push_back(p);
  }

  void DrawCapsule(math::Vec3 const& center, float radius, float height,
                   math::Vec3 const& color, float lifetime) override {
    DebugPrimitive p;
    p.type = DebugPrimitive::Capsule;
    p.start = center;
    p.param1 = radius;
    p.param2 = height;
    p.color = color;
    p.lifetime = lifetime;
    p.createTime = std::chrono::steady_clock::now();
    m_primitives.push_back(p);
  }

  void DrawArrow(math::Vec3 const& start, math::Vec3 const& end,
                 math::Vec3 const& color, float lifetime) override {
    DebugPrimitive p;
    p.type = DebugPrimitive::Arrow;
    p.start = start;
    p.end = end;
    p.color = color;
    p.lifetime = lifetime;
    p.createTime = std::chrono::steady_clock::now();
    m_primitives.push_back(p);
  }

  void DrawText(math::Vec3 const& position, const char* text,
                math::Vec3 const& color, float lifetime) override {
    DebugPrimitive p;
    p.type = DebugPrimitive::Text;
    p.start = position;
    p.text = text;
    p.color = color;
    p.lifetime = lifetime;
    p.createTime = std::chrono::steady_clock::now();
    m_primitives.push_back(p);
  }

  // === Persistence ===

  void ClearDrawings() override {
    m_primitives.clear();
  }

  void ClearOldDrawings(float maxAge) override {
    auto now = std::chrono::steady_clock::now();
    m_primitives.erase(
      std::remove_if(m_primitives.begin(), m_primitives.end(),
                     [&now, maxAge](DebugPrimitive const& p) {
                       auto age = std::chrono::duration<float>(now - p.createTime).count();
                       return age > maxAge;
                     }),
      m_primitives.end());
  }

  // === UI ===

  void OnDrawUI() override {
    ImGui::Text("Debug Visualization");
    ImGui::Separator();

    // Collision
    if (ImGui::CollapsingHeader("Collision", ImGuiTreeNodeFlags_DefaultOpen)) {
      ImGui::Checkbox("Show Colliders", &m_collisionSettings.showColliders);
      ImGui::Checkbox("Show Triggers", &m_collisionSettings.showTriggers);
      ImGui::Checkbox("Show Contacts", &m_collisionSettings.showContacts);

      ImGui::ColorEdit3("Collider Color", m_collisionSettings.colliderColor);
      ImGui::ColorEdit3("Trigger Color", m_collisionSettings.triggerColor);
      ImGui::SliderFloat("Opacity", &m_collisionSettings.opacity, 0.0f, 1.0f);
    }

    // Navigation
    if (ImGui::CollapsingHeader("Navigation")) {
      ImGui::Checkbox("Show Mesh", &m_navigationSettings.showMesh);
      ImGui::Checkbox("Show Links", &m_navigationSettings.showLinks);
      ImGui::Checkbox("Show Path", &m_navigationSettings.showPath);

      ImGui::ColorEdit3("Walkable Color", m_navigationSettings.walkableColor);
      ImGui::SliderFloat("Opacity##Nav", &m_navigationSettings.opacity, 0.0f, 1.0f);
    }

    // Quick toggles
    ImGui::Separator();
    ImGui::Text("Quick Toggles:");

    bool collision = IsVisualizationEnabled(DebugVisFlags::Collision);
    if (ImGui::Checkbox("Collision", &collision)) {
      EnableVisualization(DebugVisFlags::Collision, collision);
    }

    bool navigation = IsVisualizationEnabled(DebugVisFlags::Navigation);
    if (ImGui::Checkbox("Navigation", &navigation)) {
      EnableVisualization(DebugVisFlags::Navigation, navigation);
    }

    bool bounds = IsVisualizationEnabled(DebugVisFlags::Bounds);
    if (ImGui::Checkbox("Bounds", &bounds)) {
      EnableVisualization(DebugVisFlags::Bounds, bounds);
    }

    bool wireframe = IsVisualizationEnabled(DebugVisFlags::Wireframe);
    if (ImGui::Checkbox("Wireframe", &wireframe)) {
      EnableVisualization(DebugVisFlags::Wireframe, wireframe);
    }

    // Clear button
    ImGui::Separator();
    if (ImGui::Button("Clear All Drawings")) {
      ClearDrawings();
    }
  }

private:
  void DrawPrimitive(DebugPrimitive const& p) {
    // TODO: Integrate with Pipeline module for actual rendering
    (void)p;
  }

private:
  DebugVisFlags m_flags;
  CollisionVisSettings m_collisionSettings;
  NavigationVisSettings m_navigationSettings;
  std::vector<DebugPrimitive> m_primitives;
};

IDebugVisualization* CreateDebugVisualization() {
  return new DebugVisualizationImpl();
}

}  // namespace editor
}  // namespace te
