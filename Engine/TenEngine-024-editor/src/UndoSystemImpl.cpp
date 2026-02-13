/**
 * @file UndoSystemImpl.cpp
 * @brief Undo/Redo system (024-Editor).
 */
#include <te/editor/UndoSystem.h>
#include <deque>
#include <algorithm>

namespace te {
namespace editor {

class UndoSystemImpl : public IUndoSystem {
public:
  explicit UndoSystemImpl(int maxDepth) : m_maxDepth(maxDepth) {}

  void PushCommand(ICommand* cmd) override {
    if (!cmd) return;
    cmd->Execute();
    m_undoStack.push_back(cmd);
    if (static_cast<int>(m_undoStack.size()) > m_maxDepth) {
      delete m_undoStack.front();
      m_undoStack.pop_front();
    }
    m_redoStack.clear();
  }

  void Undo() override {
    if (m_undoStack.empty()) return;
    ICommand* cmd = m_undoStack.back();
    m_undoStack.pop_back();
    cmd->Undo();
    m_redoStack.push_back(cmd);
  }

  void Redo() override {
    if (m_redoStack.empty()) return;
    ICommand* cmd = m_redoStack.back();
    m_redoStack.pop_back();
    cmd->Redo();
    m_undoStack.push_back(cmd);
  }

  bool CanUndo() const override { return !m_undoStack.empty(); }
  bool CanRedo() const override { return !m_redoStack.empty(); }

  ~UndoSystemImpl() override {
    for (auto* c : m_undoStack) delete c;
    for (auto* c : m_redoStack) delete c;
  }

private:
  int m_maxDepth;
  std::deque<ICommand*> m_undoStack;
  std::deque<ICommand*> m_redoStack;
};

IUndoSystem* CreateUndoSystem(int maxDepth) {
  return new UndoSystemImpl(maxDepth);
}

}  // namespace editor
}  // namespace te
