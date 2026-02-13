/**
 * @file PropertyPanelImpl.cpp
 * @brief Property panel (024-Editor).
 */
#include <te/editor/PropertyPanel.h>
#include <te/editor/UndoSystem.h>

namespace te {
namespace editor {

class PropertyPanelImpl : public IPropertyPanel {
public:
  explicit PropertyPanelImpl(IUndoSystem* undo) : m_undo(undo) {}
  void Undo() override { if (m_undo) m_undo->Undo(); }
  void Redo() override { if (m_undo) m_undo->Redo(); }
  bool CanUndo() const override { return m_undo && m_undo->CanUndo(); }
  bool CanRedo() const override { return m_undo && m_undo->CanRedo(); }
  void OnDraw() override {}
private:
  IUndoSystem* m_undo;
};

IPropertyPanel* CreatePropertyPanel(IUndoSystem* undoSystem) {
  return new PropertyPanelImpl(undoSystem);
}

}  // namespace editor
}  // namespace te
