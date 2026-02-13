/**
 * @file EditorPanel.h
 * @brief Base class for dockable/floatable editor panels.
 */
#ifndef TE_EDITOR_EDITOR_PANEL_H
#define TE_EDITOR_EDITOR_PANEL_H

namespace te {
namespace editor {

class IEditorPanel {
public:
  virtual ~IEditorPanel() = default;
  virtual void SetDocked(bool docked) = 0;
  virtual bool IsFloating() const = 0;
  virtual void SetPosition(int x, int y) = 0;
  virtual void SetSize(int w, int h) = 0;
  virtual char const* GetTitle() const = 0;
  virtual bool CanClose() const = 0;
  virtual void OnDraw() = 0;
};

}  // namespace editor
}  // namespace te

#endif  // TE_EDITOR_EDITOR_PANEL_H
