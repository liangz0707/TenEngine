/**
 * @file EditorScripting.h
 * @brief Editor scripting and command system interface.
 */
#ifndef TE_EDITOR_EDITOR_SCRIPTING_H
#define TE_EDITOR_EDITOR_SCRIPTING_H

#include <te/editor/EditorTypes.h>
#include <functional>
#include <vector>
#include <cstdint>

namespace te {
namespace editor {

/**
 * @brief Editor command context.
 */
struct CommandContext {
  void* editor = nullptr;           ///< Editor instance
  void* selection = nullptr;        ///< Current selection
  void* activeObject = nullptr;     ///< Active object
};

/**
 * @brief Editor command definition.
 */
struct EditorCommand {
  const char* id = nullptr;               ///< Unique command ID
  const char* displayName = nullptr;      ///< Display name
  const char* category = nullptr;         ///< Category for organization
  const char* menuItem = nullptr;         ///< Menu path (e.g., "File/Save")
  int priority = 0;                       ///< Menu item priority
  
  std::function<void(CommandContext&)> execute;  ///< Execute callback
  std::function<bool(CommandContext&)> canExecute;  ///< Can execute check
  std::function<bool(CommandContext&)> isChecked;   ///< Is checked for toggle
  
  bool visible = true;
  bool enabled = true;
};

/**
 * @brief Macro definition for recorded actions.
 */
struct EditorMacro {
  char name[64] = "";
  std::vector<const char*> commandIds;
  bool enabled = true;
};

/**
 * @brief Editor scripting system interface.
 * 
 * Provides command registration, execution, and scripting capabilities.
 */
class IEditorScripting {
public:
  virtual ~IEditorScripting() = default;
  
  // === Command Registration ===
  
  /**
   * @brief Register a command.
   * @param command Command definition
   * @return true if registered successfully
   */
  virtual bool RegisterCommand(EditorCommand const& command) = 0;
  
  /**
   * @brief Unregister a command.
   * @param commandId Command ID to unregister
   */
  virtual void UnregisterCommand(const char* commandId) = 0;
  
  /**
   * @brief Register standard editor commands.
   */
  virtual void RegisterStandardCommands() = 0;
  
  // === Command Execution ===
  
  /**
   * @brief Execute a command by ID.
   * @param commandId Command to execute
   * @param ctx Execution context (nullptr for default)
   * @return true if executed successfully
   */
  virtual bool ExecuteCommand(const char* commandId, CommandContext* ctx = nullptr) = 0;
  
  /**
   * @brief Check if a command can be executed.
   */
  virtual bool CanExecuteCommand(const char* commandId) const = 0;
  
  /**
   * @brief Check if a command is checked (for toggle commands).
   */
  virtual bool IsCommandChecked(const char* commandId) const = 0;
  
  // === Command Query ===
  
  /**
   * @brief Get command by ID.
   */
  virtual EditorCommand const* GetCommand(const char* commandId) const = 0;
  
  /**
   * @brief Get all commands.
   */
  virtual std::vector<EditorCommand const*> GetAllCommands() const = 0;
  
  /**
   * @brief Get commands by category.
   */
  virtual std::vector<EditorCommand const*> GetCommandsByCategory(const char* category) const = 0;
  
  /**
   * @brief Get command categories.
   */
  virtual std::vector<const char*> GetCategories() const = 0;
  
  // === Menu Integration ===
  
  /**
   * @brief Get menu path for a command.
   */
  virtual const char* GetCommandMenuPath(const char* commandId) const = 0;
  
  /**
   * @brief Get all commands for a menu.
   */
  virtual std::vector<EditorCommand const*> GetCommandsForMenu(const char* menuPath) const = 0;
  
  // === Macros ===
  
  /**
   * @brief Create a new macro.
   */
  virtual void CreateMacro(const char* name) = 0;
  
  /**
   * @brief Add command to current macro.
   */
  virtual void AddToMacro(const char* commandId) = 0;
  
  /**
   * @brief Finish recording macro.
   */
  virtual void FinishMacro() = 0;
  
  /**
   * @brief Execute a macro.
   */
  virtual bool ExecuteMacro(const char* name) = 0;
  
  /**
   * @brief Get all macros.
   */
  virtual std::vector<EditorMacro const*> GetMacros() const = 0;
  
  /**
   * @brief Delete a macro.
   */
  virtual void DeleteMacro(const char* name) = 0;
  
  // === Script Execution ===
  
  /**
   * @brief Execute a script file.
   * @param path Script file path
   * @return true if executed successfully
   */
  virtual bool ExecuteScript(char const* path) = 0;
  
  /**
   * @brief Execute a script string.
   * @param script Script content
   * @return true if executed successfully
   */
  virtual bool ExecuteScriptString(char const* script) = 0;
  
  // === Persistence ===
  
  /**
   * @brief Save macros to file.
   */
  virtual bool SaveMacros(char const* path) = 0;
  
  /**
   * @brief Load macros from file.
   */
  virtual bool LoadMacros(char const* path) = 0;
};

/**
 * @brief Factory function to create editor scripting system.
 */
IEditorScripting* CreateEditorScripting();

// === Standard Command IDs ===

namespace Commands {
  // File
  constexpr const char* NEW_SCENE = "file.new_scene";
  constexpr const char* OPEN_SCENE = "file.open_scene";
  constexpr const char* SAVE_SCENE = "file.save_scene";
  constexpr const char* SAVE_SCENE_AS = "file.save_scene_as";
  constexpr const char* RECENT_SCENES = "file.recent_scenes";
  constexpr const char* EXIT = "file.exit";
  
  // Edit
  constexpr const char* UNDO = "edit.undo";
  constexpr const char* REDO = "edit.redo";
  constexpr const char* CUT = "edit.cut";
  constexpr const char* COPY = "edit.copy";
  constexpr const char* PASTE = "edit.paste";
  constexpr const char* DUPLICATE = "edit.duplicate";
  constexpr const char* DELETE = "edit.delete";
  constexpr const char* RENAME = "edit.rename";
  constexpr const char* SELECT_ALL = "edit.select_all";
  constexpr const char* DESELECT_ALL = "edit.deselect_all";
  
  // GameObject
  constexpr const char* CREATE_EMPTY = "gameObject.create_empty";
  constexpr const char* CREATE_CUBE = "gameObject.create_cube";
  constexpr const char* CREATE_SPHERE = "gameObject.create_sphere";
  constexpr const char* CREATE_PLANE = "gameObject.create_plane";
  constexpr const char* CREATE_LIGHT = "gameObject.create_light";
  constexpr const char* CREATE_CAMERA = "gameObject.create_camera";
  constexpr const char* GROUP_OBJECTS = "gameObject.group";
  constexpr const char* UNGROUP_OBJECTS = "gameObject.ungroup";
  
  // View
  constexpr const char* TOGGLE_GRID = "view.toggle_grid";
  constexpr const char* TOGGLE_SNAP = "view.toggle_snap";
  constexpr const char* RESET_VIEW = "view.reset_view";
  constexpr const char* FRAME_SELECTED = "view.frame_selected";
  constexpr const char* TOGGLE_PANEL = "view.toggle_panel";
  
  // Tools
  constexpr const char* REIMPORT_ALL = "tools.reimport_all";
  constexpr const char* PROJECT_SETTINGS = "tools.project_settings";
  constexpr const char* EDITOR_PREFERENCES = "tools.editor_preferences";
}

}  // namespace editor
}  // namespace te

#endif  // TE_EDITOR_EDITOR_SCRIPTING_H
