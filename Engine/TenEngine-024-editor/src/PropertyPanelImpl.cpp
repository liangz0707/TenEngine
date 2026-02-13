/**
 * @file PropertyPanelImpl.cpp
 * @brief Property panel (024-Editor) - Transform and Component display.
 */
#include <te/editor/PropertyPanel.h>
#include <te/editor/UndoSystem.h>
#include <te/entity/Entity.h>
#include <te/entity/EntityId.h>
#include <te/entity/EntityManager.h>
#include <te/scene/SceneTypes.h>
#include <te/world/ModelComponent.h>
#include <te/resource/ResourceManager.h>
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

    ImGui::Text("Transform");
    te::scene::Transform t = entity->GetLocalTransform();
    float pos[3] = {t.position.x, t.position.y, t.position.z};
    float rot[4] = {t.rotation.x, t.rotation.y, t.rotation.z, t.rotation.w};
    float scl[3] = {t.scale.x, t.scale.y, t.scale.z};

    if (ImGui::DragFloat3("Position", pos, 0.01f)) {
      t.position.x = pos[0]; t.position.y = pos[1]; t.position.z = pos[2];
      entity->SetLocalTransform(t);
    }
    if (ImGui::DragFloat4("Rotation", rot, 0.01f)) {
      t.rotation.x = rot[0]; t.rotation.y = rot[1]; t.rotation.z = rot[2]; t.rotation.w = rot[3];
      entity->SetLocalTransform(t);
    }
    if (ImGui::DragFloat3("Scale", scl, 0.01f, 0.0001f)) {
      t.scale.x = scl[0]; t.scale.y = scl[1]; t.scale.z = scl[2];
      entity->SetLocalTransform(t);
    }

    ImGui::Separator();
    ImGui::Text("Components");
    DrawModelComponent(entity);
  }

private:
  void DrawModelComponent(te::entity::Entity* entity) {
    te::world::ModelComponent* comp = entity->GetComponent<te::world::ModelComponent>();
    if (!comp) return;

    ImGui::PushID("ModelComponent");
    if (ImGui::TreeNodeEx("ModelComponent", ImGuiTreeNodeFlags_DefaultOpen)) {
      char pathBuf[512] = "";
      te::resource::IResourceManager* mgr = te::resource::GetResourceManager();
      if (mgr) {
        char const* path = mgr->ResolvePath(comp->modelResourceId);
        if (path) std::strncpy(pathBuf, path, sizeof(pathBuf) - 1);
      }
      ImGui::Text("Model: %s", pathBuf[0] ? pathBuf : "(none)");
      ImGui::TreePop();
    }
    ImGui::PopID();
  }

  IUndoSystem* m_undo;
  std::vector<te::entity::EntityId> m_selection;
};

IPropertyPanel* CreatePropertyPanel(IUndoSystem* undoSystem) {
  return new PropertyPanelImpl(undoSystem);
}

}  // namespace editor
}  // namespace te
