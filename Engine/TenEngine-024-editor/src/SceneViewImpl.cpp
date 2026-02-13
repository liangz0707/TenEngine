/**
 * @file SceneViewImpl.cpp
 * @brief Scene tree view (024-Editor).
 */
#include <te/editor/SceneView.h>
#include <te/entity/EntityId.h>
#include <te/entity/Entity.h>
#include <te/entity/EntityManager.h>
#include <te/scene/ISceneNode.h>
#include <te/scene/SceneManager.h>
#include <te/world/WorldManager.h>
#include <te/world/WorldTypes.h>
#include <te/world/LevelAssetDesc.h>
#include <te/world/ModelComponent.h>
#include <te/resource/ResourceId.h>
#include <imgui.h>
#include <vector>

namespace te {
namespace editor {

namespace {
void ExportNodeToDesc(te::scene::ISceneNode* node, te::world::SceneNodeDesc& out) {
  if (!node) return;
  char const* name = node->GetName();
  out.name = name ? name : "";
  out.localTransform = node->GetLocalTransform();
  out.modelGuid = te::resource::ResourceId();
  te::entity::Entity* e = dynamic_cast<te::entity::Entity*>(node);
  if (e) {
    te::world::ModelComponent* comp = e->GetComponent<te::world::ModelComponent>();
    if (comp) out.modelGuid = comp->modelResourceId;
  }
  std::vector<te::scene::ISceneNode*> children;
  node->GetChildren(children);
  out.children.resize(children.size());
  for (size_t i = 0; i < children.size(); ++i) {
    ExportNodeToDesc(children[i], out.children[i]);
  }
}

te::entity::Entity* CreateEntityFromDesc(te::scene::WorldRef world,
                                        te::world::SceneNodeDesc const& desc,
                                        te::scene::ISceneNode* parent) {
  te::entity::Entity* e = te::entity::EntityManager::GetInstance().CreateEntity(
      world, desc.name.empty() ? nullptr : desc.name.c_str());
  if (!e) return nullptr;
  e->SetLocalTransform(desc.localTransform);
  if (!desc.modelGuid.IsNull()) {
    te::world::ModelComponent* comp = e->AddComponent<te::world::ModelComponent>();
    if (comp) comp->modelResourceId = desc.modelGuid;
  }
  if (parent) {
    e->SetParent(parent);
  }
  return e;
}

void CreateEntitiesFromDescRecurse(te::scene::WorldRef world,
                                  te::world::SceneNodeDesc const& desc,
                                  te::scene::ISceneNode* parent) {
  te::entity::Entity* e = CreateEntityFromDesc(world, desc, parent);
  if (!e) return;
  for (te::world::SceneNodeDesc const& child : desc.children) {
    CreateEntitiesFromDescRecurse(world, child, e->GetSceneNode());
  }
}
}  // namespace

class SceneViewImpl : public ISceneView {
public:
  void SetLevelHandle(void* levelHandle) override { m_levelHandle = levelHandle; }
  void SetSelection(std::vector<te::entity::EntityId> const& ids) override { m_selection = ids; }
  void GetSelection(std::vector<te::entity::EntityId>& out) const override { out = m_selection; }

  void OnDraw() override {
    te::world::LevelHandle handle(static_cast<void*>(m_levelHandle));
    if (!handle.IsValid()) return;

    if (ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey_Delete)) {
      OnDelete();
    }
    if (ImGui::IsWindowFocused() && ImGui::IsKeyDown(ImGuiMod_Ctrl) && ImGui::IsKeyPressed(ImGuiKey_C)) {
      OnCopy();
    }
    if (ImGui::IsWindowFocused() && ImGui::IsKeyDown(ImGuiMod_Ctrl) && ImGui::IsKeyPressed(ImGuiKey_V)) {
      OnPaste();
    }

    if (ImGui::BeginPopupContextWindow("SceneContext")) {
      if (ImGui::MenuItem("Delete")) OnDelete();
      if (ImGui::MenuItem("Copy")) OnCopy();
      if (ImGui::MenuItem("Paste")) OnPaste();
      ImGui::EndPopup();
    }
    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(1)) {
      ImGui::OpenPopup("SceneContext");
    }

    std::vector<te::scene::ISceneNode*> roots;
    te::world::WorldManager::GetInstance().GetRootNodes(handle, roots);

    for (te::scene::ISceneNode* node : roots) {
      DrawNode(node);
    }
  }

private:
  void DrawNode(te::scene::ISceneNode* node) {
    if (!node) return;
    te::entity::Entity* entity = dynamic_cast<te::entity::Entity*>(node);
    if (!entity) return;

    char const* name = node->GetName();
    if (!name || !name[0]) name = "(unnamed)";

    te::entity::EntityId id = entity->GetEntityId();
    bool selected = IsSelected(id);
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
    if (selected) flags |= ImGuiTreeNodeFlags_Selected;

    std::vector<te::scene::ISceneNode*> children;
    node->GetChildren(children);
    bool hasChildren = !children.empty();
    if (!hasChildren) flags |= ImGuiTreeNodeFlags_Leaf;

    bool open = ImGui::TreeNodeEx(reinterpret_cast<void*>(id.value), flags, "%s", name);
    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
      OnNodeClicked(id);
    }

    if (open) {
      for (te::scene::ISceneNode* child : children) {
        DrawNode(child);
      }
      ImGui::TreePop();
    }
  }

  bool IsSelected(te::entity::EntityId id) const {
    for (te::entity::EntityId const& s : m_selection) {
      if (s.value == id.value) return true;
    }
    return false;
  }

  void OnNodeClicked(te::entity::EntityId id) {
    if (ImGui::GetIO().KeyCtrl) {
      bool found = false;
      for (size_t i = 0; i < m_selection.size(); ++i) {
        if (m_selection[i].value == id.value) {
          m_selection.erase(m_selection.begin() + static_cast<ptrdiff_t>(i));
          found = true;
          break;
        }
      }
      if (!found) m_selection.push_back(id);
    } else {
      m_selection.clear();
      m_selection.push_back(id);
    }
  }

  bool IsParentSelected(te::entity::EntityId id) const {
    te::entity::Entity* e = te::entity::EntityManager::GetInstance().GetEntity(id);
    if (!e) return false;
    te::scene::ISceneNode* parent = e->GetParent();
    if (!parent) return false;
    te::entity::Entity* p = dynamic_cast<te::entity::Entity*>(parent);
    if (!p) return false;
    for (te::entity::EntityId const& s : m_selection) {
      if (s.value == p->GetEntityId().value) return true;
    }
    return false;
  }

  void OnDelete() {
    for (te::entity::EntityId id : m_selection) {
      te::entity::EntityManager::GetInstance().DestroyEntity(id);
    }
    m_selection.clear();
  }

  void OnCopy() {
    m_clipboard.roots.clear();
    for (te::entity::EntityId id : m_selection) {
      if (IsParentSelected(id)) continue;
      te::entity::Entity* e = te::entity::EntityManager::GetInstance().GetEntity(id);
      if (!e) continue;
      te::world::SceneNodeDesc desc;
      ExportNodeToDesc(e->GetSceneNode(), desc);
      m_clipboard.roots.push_back(std::move(desc));
    }
  }

  void OnPaste() {
    te::world::LevelHandle handle(static_cast<void*>(m_levelHandle));
    if (!handle.IsValid() || m_clipboard.roots.empty()) return;
    te::scene::SceneRef worldRef = te::world::WorldManager::GetInstance().GetSceneRef(handle);
    if (!worldRef.IsValid()) return;
    te::scene::SceneManager::GetInstance().SetActiveWorld(worldRef);
    for (te::world::SceneNodeDesc const& root : m_clipboard.roots) {
      CreateEntitiesFromDescRecurse(worldRef, root, nullptr);
    }
  }

  void* m_levelHandle = nullptr;
  std::vector<te::entity::EntityId> m_selection;
  te::world::LevelAssetDesc m_clipboard;
};

ISceneView* CreateSceneView() {
  return new SceneViewImpl();
}

}  // namespace editor
}  // namespace te
