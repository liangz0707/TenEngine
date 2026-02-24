/**
 * @file KeyBindingSystem.h
 * @brief Key binding and shortcut system interface.
 */
#ifndef TE_EDITOR_KEY_BINDING_SYSTEM_H
#define TE_EDITOR_KEY_BINDING_SYSTEM_H

#include <te/editor/EditorTypes.h>
#include <functional>
#include <cstdint>

namespace te {
namespace editor {

// Note: KeyCombo and KeyBindingAction are defined in EditorTypes.h

/**
 * @brief Key binding system interface.
 *
 * Manages keyboard shortcuts and key bindings for the editor.
 */
class IKeyBindingSystem {
public:
  virtual ~IKeyBindingSystem() = default;
  
  // === Registration ===
  
  /**
   * @brief Register a key binding action.
   * @param action Action to register
   * @return true if registered successfully
   */
  virtual bool RegisterAction(KeyBindingAction const& action) = 0;
  
  /**
   * @brief Unregister an action.
   * @param actionId Action ID to unregister
   */
  virtual void UnregisterAction(const char* actionId) = 0;
  
  /**
   * @brief Register standard editor actions.
   */
  virtual void RegisterStandardActions() = 0;
  
  // === Query ===
  
  /**
   * @brief Get action by ID.
   */
  virtual KeyBindingAction const* GetAction(const char* actionId) const = 0;
  
  /**
   * @brief Get all actions in a category.
   */
  virtual std::vector<KeyBindingAction const*> GetActionsByCategory(const char* category) const = 0;
  
  /**
   * @brief Get all categories.
   */
  virtual std::vector<const char*> GetCategories() const = 0;
  
  /**
   * @brief Get binding for an action.
   */
  virtual KeyCombo GetBinding(const char* actionId) const = 0;
  
  /**
   * @brief Find action by key combo.
   */
  virtual const char* FindActionByKeyCombo(KeyCombo const& combo) const = 0;
  
  // === Modification ===
  
  /**
   * @brief Set binding for an action.
   * @param actionId Action ID
   * @param combo New key combination
   * @return true if set successfully
   */
  virtual bool SetBinding(const char* actionId, KeyCombo const& combo) = 0;
  
  /**
   * @brief Reset binding to default.
   */
  virtual void ResetBinding(const char* actionId) = 0;
  
  /**
   * @brief Reset all bindings to defaults.
   */
  virtual void ResetAllBindings() = 0;
  
  /**
   * @brief Remove binding from an action.
   */
  virtual void ClearBinding(const char* actionId) = 0;
  
  // === Execution ===
  
  /**
   * @brief Process a key event.
   * @param combo Key combination pressed
   * @return true if an action was triggered
   */
  virtual bool ProcessKeyEvent(KeyCombo const& combo) = 0;
  
  /**
   * @brief Execute an action by ID.
   * @param actionId Action to execute
   * @return true if action was executed
   */
  virtual bool ExecuteAction(const char* actionId) = 0;
  
  // === Persistence ===
  
  /**
   * @brief Save bindings to file.
   */
  virtual bool SaveBindings(char const* path) = 0;
  
  /**
   * @brief Load bindings from file.
   */
  virtual bool LoadBindings(char const* path) = 0;
  
  // === UI ===
  
  /**
   * @brief Draw key binding configuration UI.
   */
  virtual void OnDraw() = 0;
  
  /**
   * @brief Start rebinding mode for an action.
   */
  virtual void StartRebind(const char* actionId) = 0;
  
  /**
   * @brief Cancel rebinding mode.
   */
  virtual void CancelRebind() = 0;
  
  /**
   * @brief Check if in rebinding mode.
   */
  virtual bool IsRebinding() const = 0;
  
  /**
   * @brief Check for conflicting bindings.
   */
  virtual std::vector<const char*> FindConflicts() const = 0;
};

/**
 * @brief Factory function to create key binding system.
 */
IKeyBindingSystem* CreateKeyBindingSystem();

// === Standard Action IDs ===

namespace Actions {
  // File
  constexpr const char* NEW_SCENE = "file.new_scene";
  constexpr const char* OPEN_SCENE = "file.open_scene";
  constexpr const char* SAVE_SCENE = "file.save_scene";
  constexpr const char* SAVE_SCENE_AS = "file.save_scene_as";
  
  // Edit
  constexpr const char* UNDO = "edit.undo";
  constexpr const char* REDO = "edit.redo";
  constexpr const char* CUT = "edit.cut";
  constexpr const char* COPY = "edit.copy";
  constexpr const char* PASTE = "edit.paste";
  constexpr const char* DUPLICATE = "edit.duplicate";
  constexpr const char* DELETE = "edit.delete";
  constexpr const char* SELECT_ALL = "edit.select_all";
  
  // View
  constexpr const char* FOCUS_SELECTED = "view.focus_selected";
  constexpr const char* TOGGLE_GRID = "view.toggle_grid";
  constexpr const char* TOGGLE_SNAP = "view.toggle_snap";
  
  // Transform
  constexpr const char* SELECT_TOOL = "transform.select";
  constexpr const char* MOVE_TOOL = "transform.move";
  constexpr const char* ROTATE_TOOL = "transform.rotate";
  constexpr const char* SCALE_TOOL = "transform.scale";
  
  // Play
  constexpr const char* PLAY = "play.play";
  constexpr const char* PAUSE = "play.pause";
  constexpr const char* STOP = "play.stop";
  constexpr const char* STEP = "play.step";
  
  // Camera
  constexpr const char* CAMERA_BOOKMARK_1 = "camera.bookmark_1";
  constexpr const char* CAMERA_BOOKMARK_2 = "camera.bookmark_2";
  constexpr const char* CAMERA_BOOKMARK_3 = "camera.bookmark_3";
}

}  // namespace editor
}  // namespace te

#endif  // TE_EDITOR_KEY_BINDING_SYSTEM_H
