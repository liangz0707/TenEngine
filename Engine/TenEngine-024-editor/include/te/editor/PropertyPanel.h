/**
 * @file PropertyPanel.h
 * @brief Property panel (ABI IPropertyPanel).
 */
#ifndef TE_EDITOR_PROPERTY_PANEL_H
#define TE_EDITOR_PROPERTY_PANEL_H

#include <vector>

namespace te {
namespace entity {
struct EntityId;
}
namespace editor {

class IPropertyPanel {
public:
  virtual ~IPropertyPanel() = default;
  virtual void Undo() = 0;
  virtual void Redo() = 0;
  virtual bool CanUndo() const = 0;
  virtual bool CanRedo() const = 0;
  virtual void OnDraw() = 0;
  /** Set selected entities (from SceneView). */
  virtual void SetSelection(std::vector<te::entity::EntityId> const& ids) = 0;
};

IPropertyPanel* CreatePropertyPanel(class IUndoSystem* undoSystem);

}  // namespace editor
}  // namespace te

#endif  // TE_EDITOR_PROPERTY_PANEL_H
