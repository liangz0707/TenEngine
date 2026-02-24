/**
 * @file HistoryManager.h
 * @brief Enhanced history and undo/redo system.
 */
#ifndef TE_EDITOR_HISTORY_MANAGER_H
#define TE_EDITOR_HISTORY_MANAGER_H

#include <te/editor/EditorTypes.h>
#include <functional>
#include <vector>
#include <cstdint>

namespace te {
namespace editor {

// Note: HistoryActionType, HistoryAction, and HistoryBookmark are defined in EditorTypes.h

/**
 * @brief Enhanced history manager interface.
 *
 * Provides undo/redo with history visualization, bookmarks, and search.
 */
class IHistoryManager {
public:
  virtual ~IHistoryManager() = default;

  // === Recording ===

  /**
   * @brief Begin a compound action.
   */
  virtual void BeginCompoundAction(char const* description) = 0;

  /**
   * @brief End the current compound action.
   */
  virtual void EndCompoundAction() = 0;

  /**
   * @brief Record an action.
   * @param action Action to record
   * @return Action ID
   */
  virtual uint64_t RecordAction(HistoryAction const& action) = 0;

  /**
   * @brief Record a simple property change.
   */
  virtual uint64_t RecordPropertyChange(uint64_t targetId, char const* propertyName,
                                        std::function<void()> undo,
                                        std::function<void()> redo) = 0;

  /**
   * @brief Pause recording (for programmatic changes).
   */
  virtual void PauseRecording() = 0;

  /**
   * @brief Resume recording.
   */
  virtual void ResumeRecording() = 0;

  /**
   * @brief Check if recording is paused.
   */
  virtual bool IsRecordingPaused() const = 0;

  // === Undo/Redo ===

  /**
   * @brief Undo the last action.
   * @return true if undone
   */
  virtual bool Undo() = 0;

  /**
   * @brief Redo the next action.
   * @return true if redone
   */
  virtual bool Redo() = 0;

  /**
   * @brief Can undo?
   */
  virtual bool CanUndo() const = 0;

  /**
   * @brief Can redo?
   */
  virtual bool CanRedo() const = 0;

  /**
   * @brief Get undo description.
   */
  virtual char const* GetUndoDescription() const = 0;

  /**
   * @brief Get redo description.
   */
  virtual char const* GetRedoDescription() const = 0;

  // === History Query ===

  /**
   * @brief Get total action count.
   */
  virtual size_t GetActionCount() const = 0;

  /**
   * @brief Get current position in history.
   */
  virtual size_t GetCurrentPosition() const = 0;

  /**
   * @brief Get action at position.
   */
  virtual HistoryAction const* GetActionAt(size_t index) const = 0;

  /**
   * @brief Get all actions.
   */
  virtual std::vector<HistoryAction const*> GetAllActions() const = 0;

  /**
   * @brief Get actions in range.
   */
  virtual std::vector<HistoryAction const*> GetActionsInRange(
      size_t start, size_t count) const = 0;

  /**
   * @brief Find actions matching description.
   */
  virtual std::vector<HistoryAction const*> FindActions(
      char const* searchText) const = 0;

  // === Navigation ===

  /**
   * @brief Jump to a specific action.
   */
  virtual bool JumpToAction(uint64_t actionId) = 0;

  /**
   * @brief Jump to position.
   */
  virtual bool JumpToPosition(size_t position) = 0;

  // === Bookmarks ===

  /**
   * @brief Create a bookmark at current position.
   */
  virtual uint64_t CreateBookmark(char const* name) = 0;

  /**
   * @brief Delete a bookmark.
   */
  virtual bool DeleteBookmark(uint64_t bookmarkId) = 0;

  /**
   * @brief Get all bookmarks.
   */
  virtual std::vector<HistoryBookmark const*> GetBookmarks() const = 0;

  /**
   * @brief Jump to bookmark.
   */
  virtual bool JumpToBookmark(uint64_t bookmarkId) = 0;

  // === Management ===

  /**
   * @brief Clear all history.
   */
  virtual void ClearHistory() = 0;

  /**
   * @brief Set maximum history size.
   */
  virtual void SetMaxHistorySize(size_t maxSize) = 0;

  /**
   * @brief Get maximum history size.
   */
  virtual size_t GetMaxHistorySize() const = 0;

  /**
   * @brief Compress history (merge similar consecutive actions).
   */
  virtual void CompressHistory() = 0;

  // === Persistence ===

  /**
   * @brief Save history to file.
   */
  virtual bool SaveHistory(char const* path) = 0;

  /**
   * @brief Load history from file.
   */
  virtual bool LoadHistory(char const* path) = 0;

  // === UI ===

  /**
   * @brief Draw history panel UI.
   */
  virtual void OnDraw() = 0;
};

/**
 * @brief Factory function to create history manager.
 */
IHistoryManager* CreateHistoryManager();

}  // namespace editor
}  // namespace te

#endif  // TE_EDITOR_HISTORY_MANAGER_H
