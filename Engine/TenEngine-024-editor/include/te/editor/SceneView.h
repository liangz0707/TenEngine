/**
 * @file SceneView.h
 * @brief Scene tree view (ABI ISceneView).
 */
#ifndef TE_EDITOR_SCENE_VIEW_H
#define TE_EDITOR_SCENE_VIEW_H

#include <vector>

namespace te {
namespace entity {
struct EntityId;
}
namespace editor {

class ISceneView {
public:
  virtual ~ISceneView() = default;
  virtual void SetLevelHandle(void* levelHandle) = 0;
  virtual void SetSelection(std::vector<te::entity::EntityId> const& ids) = 0;
  virtual void GetSelection(std::vector<te::entity::EntityId>& out) const = 0;
  virtual void OnDraw() = 0;
};

ISceneView* CreateSceneView();

}  // namespace editor
}  // namespace te

#endif  // TE_EDITOR_SCENE_VIEW_H
