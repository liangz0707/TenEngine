/**
 * @file PropertyPanelImpl.cpp
 * @brief Property panel (024-Editor) - Transform and Component display/editing.
 */
#include <te/editor/PropertyPanel.h>
#include <te/editor/UndoSystem.h>
#include <te/entity/Entity.h>
#include <te/entity/EntityId.h>
#include <te/entity/EntityManager.h>
#include <te/entity/ComponentRegistry.h>
#include <te/scene/SceneTypes.h>
#include <te/world/ModelComponent.h>
#include <te/world/LightComponent.h>
#include <te/world/CameraComponent.h>
#include <te/resource/ResourceManager.h>
#include <te/core/log.h>
#include <imgui.h>
#include <vector>
#include <cstring>

namespace te {
namespace editor {

class PropertyPanelImpl : public IPropertyPanel {
public:
  explicit PropertyPanelImpl(IUndoSystem* undo) : m_undo(undo) {}

  void Undo() override { if (m_undo) m_undo->Undo(); }
  void Redo() override { if (m_undo) m_undo->Redo(); }
  bool CanUndo() const override { return m_undo && m_undo->CanUndo(); }
  bool CanRedo() const override { return m_undo && m_undo->CanRedo(); }

  void SetSelection(std::vector<te::entity::EntityId> const& ids) override { m_selection = ids; }

  void OnDraw() override {
    if (m_selection.empty()) {
      ImGui::TextDisabled("No entity selected");
      return;
    }
    te::entity::Entity* entity = te::entity::EntityManager::GetInstance().GetEntity(m_selection[0]);
    if (!entity) {
      ImGui::TextDisabled("Invalid entity");
      return;
    }

    char const* name = entity->GetName();
    ImGui::Text("Entity: %s", name && name[0] ? name : "(unnamed)");
    ImGui::Separator();

    // Transform section
    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
      DrawTransform(entity);
    }

    ImGui::Separator();

    // Components section
    ImGui::Text("Components");
    DrawAllComponents(entity);

    // Add Component button
    ImGui::Separator();
    DrawAddComponentMenu(entity);
  }

private:
  void DrawTransform(te::entity::Entity* entity) {
    te::scene::Transform t = entity->GetLocalTransform();
    float pos[3] = {t.position.x, t.position.y, t.position.z};
    float rot[3] = {
      t.rotation.x * 180.0f / 3.14159f,
      t.rotation.y * 180.0f / 3.14159f,
      t.rotation.z * 180.0f / 3.14159f
    };
    float scl[3] = {t.scale.x, t.scale.y, t.scale.z};

    bool changed = false;
    if (ImGui::DragFloat3("Position", pos, 0.01f)) {
      t.position.x = pos[0]; t.position.y = pos[1]; t.position.z = pos[2];
      changed = true;
    }
    if (ImGui::DragFloat3("Rotation (deg)", rot, 0.5f)) {
      t.rotation.x = rot[0] * 3.14159f / 180.0f;
      t.rotation.y = rot[1] * 3.14159f / 180.0f;
      t.rotation.z = rot[2] * 3.14159f / 180.0f;
      changed = true;
    }
    if (ImGui::DragFloat3("Scale", scl, 0.01f, 0.0001f)) {
      t.scale.x = scl[0]; t.scale.y = scl[1]; t.scale.z = scl[2];
      changed = true;
    }

    if (changed) {
      entity->SetLocalTransform(t);
    }
  }

  void DrawAllComponents(te::entity::Entity* entity) {
    // Draw ModelComponent
    DrawModelComponent(entity);

    // Draw LightComponent
    DrawLightComponent(entity);

    // Draw CameraComponent
    DrawCameraComponent(entity);
  }

  void DrawModelComponent(te::entity::Entity* entity) {
    te::world::ModelComponent* comp = entity->GetComponent<te::world::ModelComponent>();
    if (!comp) return;

    ImGui::PushID("ModelComponent");
    bool open = ImGui::CollapsingHeader("Model Component", ImGuiTreeNodeFlags_DefaultOpen);

    // Remove button on same line as header
    ImGui::SameLine(ImGui::GetWindowWidth() - 30);
    if (ImGui::SmallButton("X##RemoveModel")) {
      entity->RemoveComponent<te::world::ModelComponent>();
      ImGui::PopID();
      return;
    }

    if (open) {
      char pathBuf[512] = "";
      te::resource::IResourceManager* mgr = te::resource::GetResourceManager();
      if (mgr) {
        char const* path = mgr->ResolvePath(comp->modelResourceId);
        if (path) std::strncpy(pathBuf, path, sizeof(pathBuf) - 1);
      }

      ImGui::Text("Model Path:");
      ImGui::SameLine();
      if (pathBuf[0]) {
        ImGui::TextColored(ImVec4(0.6f, 1.0f, 0.6f, 1.0f), "%s", pathBuf);
      } else {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "(none)");
      }
    }
    ImGui::PopID();
  }

  void DrawLightComponent(te::entity::Entity* entity) {
    te::world::LightComponent* comp = entity->GetComponent<te::world::LightComponent>();
    if (!comp) return;

    ImGui::PushID("LightComponent");
    bool open = ImGui::CollapsingHeader("Light Component", ImGuiTreeNodeFlags_DefaultOpen);

    // Remove button
    ImGui::SameLine(ImGui::GetWindowWidth() - 30);
    if (ImGui::SmallButton("X##RemoveLight")) {
      entity->RemoveComponent<te::world::LightComponent>();
      ImGui::PopID();
      return;
    }

    if (open) {
      // Light type selector
      const char* typeNames[] = { "Point", "Directional", "Spot" };
      int typeIndex = static_cast<int>(comp->type);
      if (ImGui::Combo("Type", &typeIndex, typeNames, IM_ARRAYSIZE(typeNames))) {
        comp->type = static_cast<te::world::LightType>(typeIndex);
      }

      // Color
      ImGui::ColorEdit3("Color", comp->color);

      // Intensity
      ImGui::DragFloat("Intensity", &comp->intensity, 0.01f, 0.0f, 100.0f);

      // Range (for Point and Spot)
      if (comp->type == te::world::LightType::Point ||
          comp->type == te::world::LightType::Spot) {
        ImGui::DragFloat("Range", &comp->range, 0.1f, 0.1f, 1000.0f);
      }

      // Direction (for Directional and Spot)
      if (comp->type == te::world::LightType::Directional ||
          comp->type == te::world::LightType::Spot) {
        ImGui::DragFloat3("Direction", comp->direction, 0.01f, -1.0f, 1.0f);
        // Normalize direction
        float len = sqrtf(comp->direction[0]*comp->direction[0] +
                         comp->direction[1]*comp->direction[1] +
                         comp->direction[2]*comp->direction[2]);
        if (len > 0.0001f && len != 1.0f) {
          comp->direction[0] /= len;
          comp->direction[1] /= len;
          comp->direction[2] /= len;
        }
      }

      // Spot angle (for Spot only)
      if (comp->type == te::world::LightType::Spot) {
        float degAngle = comp->spotAngle * 180.0f / 3.14159f;
        if (ImGui::DragFloat("Spot Angle (deg)", &degAngle, 0.5f, 1.0f, 90.0f)) {
          comp->spotAngle = degAngle * 3.14159f / 180.0f;
        }
      }
    }
    ImGui::PopID();
  }

  void DrawCameraComponent(te::entity::Entity* entity) {
    te::world::CameraComponent* comp = entity->GetComponent<te::world::CameraComponent>();
    if (!comp) return;

    ImGui::PushID("CameraComponent");
    bool open = ImGui::CollapsingHeader("Camera Component", ImGuiTreeNodeFlags_DefaultOpen);

    // Remove button
    ImGui::SameLine(ImGui::GetWindowWidth() - 30);
    if (ImGui::SmallButton("X##RemoveCamera")) {
      entity->RemoveComponent<te::world::CameraComponent>();
      ImGui::PopID();
      return;
    }

    if (open) {
      // FOV
      float fovDeg = comp->fovY * 180.0f / 3.14159f;
      if (ImGui::DragFloat("FOV (deg)", &fovDeg, 0.5f, 10.0f, 170.0f)) {
        comp->fovY = fovDeg * 3.14159f / 180.0f;
      }

      // Near plane
      ImGui::DragFloat("Near Plane", &comp->nearZ, 0.01f, 0.001f, 100.0f);

      // Far plane
      ImGui::DragFloat("Far Plane", &comp->farZ, 1.0f, 1.0f, 10000.0f);

      // Active toggle
      ImGui::Checkbox("Is Active Camera", &comp->isActive);
    }
    ImGui::PopID();
  }

  void DrawAddComponentMenu(te::entity::Entity* entity) {
    ImGui::TextDisabled("Add Component:");

    // Model Component button
    bool hasModel = entity->GetComponent<te::world::ModelComponent>() != nullptr;
    if (hasModel) ImGui::BeginDisabled();
    if (ImGui::SmallButton("Model")) {
      entity->AddComponent<te::world::ModelComponent>();
      te::core::Log(te::core::LogLevel::Info, "Editor: Added ModelComponent");
    }
    if (hasModel) ImGui::EndDisabled();
    if (hasModel) ImGui::SameLine();

    // Light Component button
    bool hasLight = entity->GetComponent<te::world::LightComponent>() != nullptr;
    if (hasLight) ImGui::BeginDisabled();
    ImGui::SameLine();
    if (ImGui::SmallButton("Light")) {
      te::world::LightComponent* comp = entity->AddComponent<te::world::LightComponent>();
      if (comp) {
        comp->type = te::world::LightType::Point;
        comp->color[0] = 1.f;
        comp->color[1] = 1.f;
        comp->color[2] = 1.f;
        comp->intensity = 1.f;
        comp->range = 10.f;
      }
      te::core::Log(te::core::LogLevel::Info, "Editor: Added LightComponent");
    }
    if (hasLight) ImGui::EndDisabled();
    if (hasLight) ImGui::SameLine();

    // Camera Component button
    bool hasCamera = entity->GetComponent<te::world::CameraComponent>() != nullptr;
    if (hasCamera) ImGui::BeginDisabled();
    ImGui::SameLine();
    if (ImGui::SmallButton("Camera")) {
      te::world::CameraComponent* comp = entity->AddComponent<te::world::CameraComponent>();
      if (comp) {
        comp->fovY = 1.0472f;
        comp->nearZ = 0.1f;
        comp->farZ = 1000.f;
        comp->isActive = false;
      }
      te::core::Log(te::core::LogLevel::Info, "Editor: Added CameraComponent");
    }
    if (hasCamera) ImGui::EndDisabled();

    // Help text
    ImGui::TextDisabled("(Each component type can only be added once)");
  }

  IUndoSystem* m_undo;
  std::vector<te::entity::EntityId> m_selection;
};

IPropertyPanel* CreatePropertyPanel(IUndoSystem* undoSystem) {
  return new PropertyPanelImpl(undoSystem);
}

}  // namespace editor
}  // namespace te
