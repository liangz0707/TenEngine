/**
 * @file PropertyPanelImpl.cpp
 * @brief Property panel (024-Editor) - Dynamic component display using reflection.
 */
#include <te/editor/PropertyPanel.h>
#include <te/editor/UndoSystem.h>
#include <te/entity/Entity.h>
#include <te/entity/EntityId.h>
#include <te/entity/EntityManager.h>
#include <te/entity/ComponentRegistry.h>
#include <te/entity/PropertyReflection.h>
#include <te/scene/SceneTypes.h>
#include <te/world/ModelComponent.h>
#include <te/world/LightComponent.h>
#include <te/world/CameraComponent.h>
#include <te/resource/ResourceManager.h>
#include <te/core/log.h>
#include <imgui.h>
#include <vector>
#include <cstring>
#include <cmath>

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

    // Components section - using reflection
    ImGui::Text("Components");
    DrawComponentsWithReflection(entity);

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

  /**
   * @brief Draw all components using reflection system.
   */
  void DrawComponentsWithReflection(te::entity::Entity* entity) {
    te::entity::IPropertyRegistry* propReg = te::entity::GetPropertyRegistry();

    // Check each known component type
    DrawComponentByType<te::world::ModelComponent>(entity, "ModelComponent", propReg);
    DrawComponentByType<te::world::LightComponent>(entity, "LightComponent", propReg);
    DrawComponentByType<te::world::CameraComponent>(entity, "CameraComponent", propReg);
  }

  /**
   * @brief Draw a specific component type using reflection.
   */
  template<typename T>
  void DrawComponentByType(te::entity::Entity* entity, const char* typeName,
                           te::entity::IPropertyRegistry* propReg) {
    T* comp = entity->GetComponent<T>();
    if (!comp) return;

    ImGui::PushID(typeName);

    // Component header with remove button
    std::string headerLabel = GetComponentDisplayName(typeName);
    bool open = ImGui::CollapsingHeader(headerLabel.c_str(), ImGuiTreeNodeFlags_DefaultOpen);

    // Remove button on same line
    ImGui::SameLine(ImGui::GetWindowWidth() - 35);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.3f, 0.3f, 1.0f));
    if (ImGui::SmallButton("X##Remove")) {
      entity->RemoveComponent<T>();
      ImGui::PopStyleColor(2);
      ImGui::PopID();
      return;
    }
    ImGui::PopStyleColor(2);

    if (open) {
      // Special handling for ModelComponent (display resource path)
      if constexpr (std::is_same_v<T, te::world::ModelComponent>) {
        DrawModelComponentContent(comp);
      }
      // Use reflection for other components
      else if (propReg) {
        const te::entity::ComponentMeta* meta = propReg->GetComponentMeta(typeName);
        if (meta && meta->properties && meta->propertyCount > 0) {
          DrawPropertiesFromMeta(comp, meta);
        } else {
          // Fallback: no reflection data
          ImGui::TextDisabled("(No properties registered)");
        }
      } else {
        ImGui::TextDisabled("(Property registry not available)");
      }
    }

    ImGui::PopID();
  }

  /**
   * @brief Draw properties using reflection metadata.
   */
  void DrawPropertiesFromMeta(void* componentBase, const te::entity::ComponentMeta* meta) {
    ImGui::Indent(10.0f);

    for (size_t i = 0; i < meta->propertyCount; ++i) {
      const te::entity::PropertyMeta& prop = meta->properties[i];

      // Skip properties that shouldn't be shown for current component state
      if (!ShouldShowProperty(componentBase, meta, prop)) {
        continue;
      }

      DrawProperty(componentBase, prop);
    }

    ImGui::Unindent(10.0f);
  }

  /**
   * @brief Check if property should be shown (for conditional visibility).
   */
  bool ShouldShowProperty(void* componentBase, const te::entity::ComponentMeta* meta,
                          const te::entity::PropertyMeta& prop) {
    // For LightComponent, hide Range/Direction/SpotAngle based on type
    if (strcmp(meta->typeName, "LightComponent") == 0) {
      te::world::LightComponent* light = static_cast<te::world::LightComponent*>(componentBase);

      // Range only for Point and Spot
      if (strcmp(prop.name, "Range") == 0) {
        return light->type == te::world::LightType::Point ||
               light->type == te::world::LightType::Spot;
      }

      // Direction only for Directional and Spot
      if (strcmp(prop.name, "Direction") == 0) {
        return light->type == te::world::LightType::Directional ||
               light->type == te::world::LightType::Spot;
      }

      // Spot Angle only for Spot
      if (strcmp(prop.name, "Spot Angle") == 0) {
        return light->type == te::world::LightType::Spot;
      }
    }

    return true;
  }

  /**
   * @brief Draw a single property based on its metadata.
   */
  void DrawProperty(void* componentBase, const te::entity::PropertyMeta& prop) {
    void* valuePtr = prop.GetValuePtr(componentBase);
    if (!valuePtr) return;

    ImGui::PushID(prop.name);

    switch (prop.valueType) {
      case te::entity::PropertyValueType::Bool:
        DrawBoolProperty(prop.name, static_cast<bool*>(valuePtr));
        break;

      case te::entity::PropertyValueType::Int32:
        DrawIntProperty(prop.name, static_cast<int32_t*>(valuePtr), prop.hints);
        break;

      case te::entity::PropertyValueType::Float32:
        DrawFloatProperty(prop.name, static_cast<float*>(valuePtr), prop.hints);
        break;

      case te::entity::PropertyValueType::Float3:
        if (prop.hints.isColor) {
          DrawColor3Property(prop.name, static_cast<float*>(valuePtr));
        } else {
          DrawFloat3Property(prop.name, static_cast<float*>(valuePtr), prop.hints);
        }
        break;

      case te::entity::PropertyValueType::Float4:
        if (prop.hints.isColor) {
          DrawColor4Property(prop.name, static_cast<float*>(valuePtr));
        } else {
          DrawFloat4Property(prop.name, static_cast<float*>(valuePtr), prop.hints);
        }
        break;

      case te::entity::PropertyValueType::Enum:
        DrawEnumProperty(prop.name, static_cast<int32_t*>(valuePtr),
                        prop.enumNames, prop.enumCount);
        break;

      default:
        ImGui::Text("%s: (unsupported type)", prop.name);
        break;
    }

    ImGui::PopID();
  }

  // ========================================================================
  // Property Type Drawers
  // ========================================================================

  void DrawBoolProperty(const char* name, bool* value) {
    ImGui::Checkbox(name, value);
  }

  void DrawIntProperty(const char* name, int32_t* value,
                       const te::entity::EditorHints& hints) {
    int v = *value;
    int minVal = static_cast<int>(hints.minValue);
    int maxVal = static_cast<int>(hints.maxValue);

    if (maxVal > minVal && !hints.noSlider) {
      if (ImGui::SliderInt(name, &v, minVal, maxVal)) {
        *value = v;
      }
    } else {
      if (ImGui::DragInt(name, &v, static_cast<float>(hints.step), minVal, maxVal)) {
        *value = v;
      }
    }
  }

  void DrawFloatProperty(const char* name, float* value,
                         const te::entity::EditorHints& hints) {
    float v = *value;
    bool changed = false;

    // Convert radians to degrees for display if needed
    float displayValue = hints.isRadians ? (v * 180.0f / 3.14159f) : v;
    float displayMin = hints.isRadians ? (hints.minValue * 180.0f / 3.14159f) : hints.minValue;
    float displayMax = hints.isRadians ? (hints.maxValue * 180.0f / 3.14159f) : hints.maxValue;

    if (displayMax > displayMin && !hints.noSlider) {
      changed = ImGui::SliderFloat(name, &displayValue, displayMin, displayMax, hints.format);
    } else {
      changed = ImGui::DragFloat(name, &displayValue, hints.step,
                                  hints.noSlider ? 0.0f : displayMin,
                                  hints.noSlider ? 0.0f : displayMax,
                                  hints.format);
    }

    if (changed) {
      *value = hints.isRadians ? (displayValue * 3.14159f / 180.0f) : displayValue;
    }
  }

  void DrawFloat3Property(const char* name, float* value,
                          const te::entity::EditorHints& hints) {
    ImGui::DragFloat3(name, value, hints.step, hints.minValue, hints.maxValue, hints.format);

    // Normalize direction vectors if requested
    // (This is handled specially for LightComponent direction)
  }

  void DrawFloat4Property(const char* name, float* value,
                          const te::entity::EditorHints& hints) {
    ImGui::DragFloat4(name, value, hints.step, hints.minValue, hints.maxValue, hints.format);
  }

  void DrawColor3Property(const char* name, float* value) {
    ImGui::ColorEdit3(name, value, ImGuiColorEditFlags_Float);
  }

  void DrawColor4Property(const char* name, float* value) {
    ImGui::ColorEdit4(name, value, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_AlphaBar);
  }

  void DrawEnumProperty(const char* name, int32_t* value,
                        const char** enumNames, int enumCount) {
    int v = *value;
    if (ImGui::Combo(name, &v, enumNames, enumCount)) {
      *value = v;
    }
  }

  // ========================================================================
  // Model Component Special Handling
  // ========================================================================

  void DrawModelComponentContent(te::world::ModelComponent* comp) {
    ImGui::Indent(10.0f);

    char pathBuf[512] = "";
    te::resource::IResourceManager* mgr = te::resource::GetResourceManager();
    if (mgr) {
      char const* path = mgr->ResolvePath(comp->modelResourceId);
      if (path) std::strncpy(pathBuf, path, sizeof(pathBuf) - 1);
    }

    ImGui::Text("Model Path:");
    if (pathBuf[0]) {
      ImGui::TextColored(ImVec4(0.6f, 1.0f, 0.6f, 1.0f), "%s", pathBuf);
    } else {
      ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "(none)");
    }

    ImGui::Unindent(10.0f);
  }

  // ========================================================================
  // Add Component Menu
  // ========================================================================

  void DrawAddComponentMenu(te::entity::Entity* entity) {
    ImGui::TextDisabled("Add Component:");

    // Get all registered component types from property registry
    te::entity::IPropertyRegistry* propReg = te::entity::GetPropertyRegistry();

    // Model Component button
    bool hasModel = entity->GetComponent<te::world::ModelComponent>() != nullptr;
    if (hasModel) ImGui::BeginDisabled();
    if (ImGui::SmallButton("Model")) {
      entity->AddComponent<te::world::ModelComponent>();
      te::core::Log(te::core::LogLevel::Info, "Editor: Added ModelComponent");
    }
    if (hasModel) ImGui::EndDisabled();

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

    ImGui::TextDisabled("(Each component type can only be added once)");
  }

  std::string GetComponentDisplayName(const char* typeName) {
    // Convert "ModelComponent" to "Model Component"
    std::string result;
    const char* p = typeName;

    // Skip common prefix
    if (strncmp(typeName, "Component", 9) == 0) {
      return std::string("Component");
    }

    while (*p) {
      if (*p >= 'A' && *p <= 'Z' && !result.empty() && result.back() != ' ') {
        result += ' ';
      }
      result += *p;
      ++p;
    }

    // Remove "Component" suffix if present
    const std::string suffix = "Component";
    if (result.size() > suffix.size() &&
        result.compare(result.size() - suffix.size(), suffix.size(), suffix) == 0) {
      result = result.substr(0, result.size() - suffix.size());
      // Remove trailing space
      while (!result.empty() && result.back() == ' ') {
        result.pop_back();
      }
    }

    return result;
  }

  IUndoSystem* m_undo;
  std::vector<te::entity::EntityId> m_selection;
};

IPropertyPanel* CreatePropertyPanel(IUndoSystem* undoSystem) {
  return new PropertyPanelImpl(undoSystem);
}

}  // namespace editor
}  // namespace te
