/**
 * @file MainMenu.h
 * @brief Main menu interface (ABI IMainMenu).
 */
#ifndef TE_EDITOR_MAIN_MENU_H
#define TE_EDITOR_MAIN_MENU_H

#include <te/editor/EditorTypes.h>
#include <functional>
#include <vector>
#include <string>

namespace te {
namespace editor {

/**
 * @brief Menu item definition.
 */
struct MenuItem {
  const char* label = nullptr;
  const char* shortcut = nullptr;  ///< e.g., "Ctrl+S"
  bool enabled = true;
  bool checked = false;
  bool separator = false;  ///< Render as separator if true
  int id = 0;  ///< User-defined ID for callback
  
  // For submenus
  bool hasSubmenu = false;
  std::vector<MenuItem> submenuItems;
};

/**
 * @brief Menu bar definition.
 */
struct MenuDef {
  const char* name = nullptr;
  std::vector<MenuItem> items;
};

/**
 * @brief Main menu interface.
 * 
 * Provides the main menu bar for the editor with customizable
 * menus, items, and keyboard shortcuts.
 */
class IMainMenu {
public:
  virtual ~IMainMenu() = default;
  
  // === Drawing ===
  
  /**
   * @brief Draw the main menu bar.
   */
  virtual void OnDraw() = 0;
  
  // === Menu Setup ===
  
  /**
   * @brief Add a menu to the menu bar.
   * @param menu Menu definition
   */
  virtual void AddMenu(MenuDef const& menu) = 0;
  
  /**
   * @brief Remove a menu by name.
   */
  virtual void RemoveMenu(const char* name) = 0;
  
  /**
   * @brief Clear all menus.
   */
  virtual void ClearMenus() = 0;
  
  /**
   * @brief Get menu by name.
   * @return Pointer to menu or nullptr if not found
   */
  virtual MenuDef* GetMenu(const char* name) = 0;
  
  // === Menu Item Operations ===
  
  /**
   * @brief Enable/disable a menu item.
   * @param menuName Menu name
   * @param itemId Item ID
   * @param enabled Enable state
   */
  virtual void SetItemEnabled(const char* menuName, int itemId, bool enabled) = 0;
  
  /**
   * @brief Set checked state of a menu item.
   */
  virtual void SetItemChecked(const char* menuName, int itemId, bool checked) = 0;
  
  /**
   * @brief Update item label dynamically.
   */
  virtual void SetItemLabel(const char* menuName, int itemId, const char* label) = 0;
  
  // === Callbacks ===
  
  /**
   * @brief Set callback for menu item clicks.
   * @param callback Function(menuName, itemId)
   */
  virtual void SetOnMenuItemClicked(std::function<void(const char*, int)> callback) = 0;
  
  // === Recent Files ===
  
  /**
   * @brief Add a recent file entry.
   */
  virtual void AddRecentFile(const char* path) = 0;
  
  /**
   * @brief Clear recent files list.
   */
  virtual void ClearRecentFiles() = 0;
  
  /**
   * @brief Get recent files list.
   */
  virtual std::vector<std::string> const& GetRecentFiles() const = 0;
  
  /**
   * @brief Set callback for recent file selection.
   */
  virtual void SetOnRecentFileSelected(std::function<void(const char*)> callback) = 0;
  
  // === Standard Menus ===
  
  /**
   * @brief Initialize standard menus (File, Edit, View, etc.).
   */
  virtual void InitializeStandardMenus() = 0;
  
  /**
   * @brief Update standard menu states (undo/redo enabled, etc.).
   */
  virtual void UpdateStandardMenuStates(bool canUndo, bool canRedo, 
                                         bool hasSelection, bool hasLevel) = 0;

  // === IDs for Standard Menu Items ===
  
  // File menu
  static constexpr int ID_NEW_SCENE = 1001;
  static constexpr int ID_OPEN_SCENE = 1002;
  static constexpr int ID_SAVE = 1003;
  static constexpr int ID_SAVE_AS = 1004;
  static constexpr int ID_RECENT_FILES = 1005;
  static constexpr int ID_EXIT = 1099;
  
  // Edit menu
  static constexpr int ID_UNDO = 2001;
  static constexpr int ID_REDO = 2002;
  static constexpr int ID_CUT = 2003;
  static constexpr int ID_COPY = 2004;
  static constexpr int ID_PASTE = 2005;
  static constexpr int ID_DUPLICATE = 2006;
  static constexpr int ID_DELETE = 2007;
  static constexpr int ID_SELECT_ALL = 2008;
  static constexpr int ID_PREFERENCES = 2099;
  
  // View menu
  static constexpr int ID_VIEW_RESET_LAYOUT = 3001;
  static constexpr int ID_VIEW_FULLSCREEN = 3002;
  static constexpr int ID_VIEW_TOGGLE_CONSOLE = 3010;
  static constexpr int ID_VIEW_TOGGLE_SCENE = 3011;
  static constexpr int ID_VIEW_TOGGLE_PROPERTIES = 3012;
  static constexpr int ID_VIEW_TOGGLE_RESOURCES = 3013;
  
  // GameObject menu
  static constexpr int ID_CREATE_EMPTY = 4001;
  static constexpr int ID_CREATE_CUBE = 4002;
  static constexpr int ID_CREATE_SPHERE = 4003;
  static constexpr int ID_CREATE_PLANE = 4004;
  static constexpr int ID_CREATE_CYLINDER = 4005;
  static constexpr int ID_CREATE_CAMERA = 4010;
  static constexpr int ID_CREATE_LIGHT_DIR = 4020;
  static constexpr int ID_CREATE_LIGHT_POINT = 4021;
  static constexpr int ID_CREATE_LIGHT_SPOT = 4022;
  
  // Tools menu
  static constexpr int ID_REIMPORT_ALL = 5001;
  static constexpr int ID_PROJECT_SETTINGS = 5002;
  
  // Help menu
  static constexpr int ID_ABOUT = 6001;
  static constexpr int ID_DOCUMENTATION = 6002;
};

/**
 * @brief Factory function to create main menu.
 */
IMainMenu* CreateMainMenu();

}  // namespace editor
}  // namespace te

#endif  // TE_EDITOR_MAIN_MENU_H
