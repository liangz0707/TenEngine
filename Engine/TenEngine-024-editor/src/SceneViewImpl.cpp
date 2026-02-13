/**
 * @file SceneViewImpl.cpp
 * @brief Scene tree view stub (024-Editor).
 */
#include <te/editor/SceneView.h>
#include <te/entity/EntityId.h>

namespace te {
namespace editor {

class SceneViewImpl : public ISceneView {
public:
  void SetLevelHandle(void* levelHandle) override { m_levelHandle = levelHandle; }
  void SetSelection(std::vector<te::entity::EntityId> const& ids) override { m_selection = ids; }
  void GetSelection(std::vector<te::entity::EntityId>& out) const override { out = m_selection; }
  void OnDraw() override {}
private:
  void* m_levelHandle = nullptr;
  std::vector<te::entity::EntityId> m_selection;
};

ISceneView* CreateSceneView() {
  return new SceneViewImpl();
}

}  // namespace editor
}  // namespace te
