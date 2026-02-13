/**
 * @file UndoSystem.h
 * @brief Undo/Redo system.
 */
#ifndef TE_EDITOR_UNDO_SYSTEM_H
#define TE_EDITOR_UNDO_SYSTEM_H

namespace te {
namespace editor {

class ICommand {
public:
  virtual ~ICommand() = default;
  virtual void Execute() = 0;
  virtual void Undo() = 0;
  virtual void Redo() = 0;
};

class IUndoSystem {
public:
  virtual ~IUndoSystem() = default;
  virtual void PushCommand(ICommand* cmd) = 0;
  virtual void Undo() = 0;
  virtual void Redo() = 0;
  virtual bool CanUndo() const = 0;
  virtual bool CanRedo() const = 0;
};

IUndoSystem* CreateUndoSystem(int maxDepth = 50);

}  // namespace editor
}  // namespace te

#endif  // TE_EDITOR_UNDO_SYSTEM_H
