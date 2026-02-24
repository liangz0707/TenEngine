/**
 * @file MainMenuImpl.cpp
 * @brief Main menu implementation (024-Editor).
 */
#include <te/editor/MainMenu.h>
#include <te/editor/UndoSystem.h>
#include <imgui.h>
#include <algorithm>

namespace te {
namespace editor {

class MainMenuImpl : public IMainMenu {
public:
  MainMenuImpl()
    : m_canUndo(false)
    , m_canRedo(false)
    , m_hasSelection(false)
    , m_hasLevel(false)
  {
  }

  // === Drawing ===
  
  void OnDraw() override {
    if (!ImGui::BeginMenuBar()) return;
    
    for (auto& menu : m_menus) {
      DrawMenu(menu);
    }
    
    ImGui::EndMenuBar();
  }
  
  // === Menu Setup ===
  
  void AddMenu(MenuDef const& menu) override {
    // Check if menu already exists
    for (auto& m : m_menus) {
      if (m.name == menu.name) {
        m = menu;
        return;
      }
    }
    m_menus.push_back(menu);
  }
  
  void RemoveMenu(const char* name) override {
    if (!name) return;

    std::string nameStr(name);
    m_menus.erase(
      std::remove_if(m_menus.begin(), m_menus.end(),
        [&nameStr](MenuDef const& m) { return m.name == nameStr; }),
      m_menus.end());
  }
  
  void ClearMenus() override {
    m_menus.clear();
  }
  
  MenuDef* GetMenu(const char* name) override {
    if (!name) return nullptr;

    std::string nameStr(name);
    for (auto& m : m_menus) {
      if (m.name == nameStr) {
        return &m;
      }
    }
    return nullptr;
  }
  
  // === Menu Item Operations ===
  
  void SetItemEnabled(const char* menuName, int itemId, bool enabled) override {
    MenuDef* menu = GetMenu(menuName);
    if (!menu) return;
    
    SetItemEnabledInList(menu->items, itemId, enabled);
  }
  
  void SetItemChecked(const char* menuName, int itemId, bool checked) override {
    MenuDef* menu = GetMenu(menuName);
    if (!menu) return;
    
    SetItemCheckedInList(menu->items, itemId, checked);
  }
  
  void SetItemLabel(const char* menuName, int itemId, const char* label) override {
    MenuDef* menu = GetMenu(menuName);
    if (!menu) return;
    
    SetItemLabelInList(menu->items, itemId, label);
  }
  
  // === Callbacks ===
  
  void SetOnMenuItemClicked(std::function<void(const char*, int)> callback) override {
    m_onMenuItemClicked = std::move(callback);
  }
  
  // === Recent Files ===
  
  void AddRecentFile(const char* path) override {
    if (!path || !path[0]) return;
    
    // Remove if already exists
    m_recentFiles.erase(
      std::remove_if(m_recentFiles.begin(), m_recentFiles.end(),
        [path](std::string const& s) { return s == path; }),
      m_recentFiles.end());
    
    // Add to front
    m_recentFiles.insert(m_recentFiles.begin(), std::string(path));
    
    // Limit size
    if (m_recentFiles.size() > 10) {
      m_recentFiles.resize(10);
    }
  }
  
  void ClearRecentFiles() override {
    m_recentFiles.clear();
  }
  
  std::vector<std::string> const& GetRecentFiles() const override {
    return m_recentFiles;
  }
  
  void SetOnRecentFileSelected(std::function<void(const char*)> callback) override {
    m_onRecentFileSelected = std::move(callback);
  }
  
  // === Standard Menus ===
  
  void InitializeStandardMenus() override {
    m_menus.clear();
    
    // File menu
    MenuDef fileMenu;
    fileMenu.name = "File";
    fileMenu.items = {
      {"New Scene", "Ctrl+N", true, false, false, ID_NEW_SCENE},
      {"Open Scene", "Ctrl+O", true, false, false, ID_OPEN_SCENE},
      {"Save", "Ctrl+S", false, false, false, ID_SAVE},
      {"Save As...", "Ctrl+Shift+S", false, false, false, ID_SAVE_AS},
      {"", "", false, false, true, 0},  // Separator
      {"Recent Files", "", false, false, false, ID_RECENT_FILES, true},
      {"", "", false, false, true, 0},  // Separator
      {"Exit", "Alt+F4", true, false, false, ID_EXIT}
    };
    m_menus.push_back(fileMenu);
    
    // Edit menu
    MenuDef editMenu;
    editMenu.name = "Edit";
    editMenu.items = {
      {"Undo", "Ctrl+Z", false, false, false, ID_UNDO},
      {"Redo", "Ctrl+Y", false, false, false, ID_REDO},
      {"", "", false, false, true, 0},
      {"Cut", "Ctrl+X", false, false, false, ID_CUT},
      {"Copy", "Ctrl+C", false, false, false, ID_COPY},
      {"Paste", "Ctrl+V", false, false, false, ID_PASTE},
      {"Duplicate", "Ctrl+D", false, false, false, ID_DUPLICATE},
      {"Delete", "Del", false, false, false, ID_DELETE},
      {"", "", false, false, true, 0},
      {"Select All", "Ctrl+A", true, false, false, ID_SELECT_ALL},
      {"", "", false, false, true, 0},
      {"Preferences...", "", true, false, false, ID_PREFERENCES}
    };
    m_menus.push_back(editMenu);
    
    // View menu
    MenuDef viewMenu;
    viewMenu.name = "View";
    viewMenu.items = {
      {"Reset Layout", "", true, false, false, ID_VIEW_RESET_LAYOUT},
      {"Fullscreen", "F11", true, false, false, ID_VIEW_FULLSCREEN},
      {"", "", false, false, true, 0},
      {"Console", "", true, true, false, ID_VIEW_TOGGLE_CONSOLE},
      {"Scene Tree", "", true, true, false, ID_VIEW_TOGGLE_SCENE},
      {"Properties", "", true, true, false, ID_VIEW_TOGGLE_PROPERTIES},
      {"Resources", "", true, true, false, ID_VIEW_TOGGLE_RESOURCES}
    };
    m_menus.push_back(viewMenu);
    
    // GameObject menu
    MenuDef gameObjectMenu;
    gameObjectMenu.name = "GameObject";
    gameObjectMenu.items = {
      {"Create Empty", "Ctrl+Shift+N", true, false, false, ID_CREATE_EMPTY},
      {"", "", false, false, true, 0},
      {"3D Object", "", true, false, false, 0, true, {
        {"Cube", "", true, false, false, ID_CREATE_CUBE},
        {"Sphere", "", true, false, false, ID_CREATE_SPHERE},
        {"Plane", "", true, false, false, ID_CREATE_PLANE},
        {"Cylinder", "", true, false, false, ID_CREATE_CYLINDER}
      }},
      {"Camera", "", true, false, false, ID_CREATE_CAMERA},
      {"Light", "", true, false, false, 0, true, {
        {"Directional Light", "", true, false, false, ID_CREATE_LIGHT_DIR},
        {"Point Light", "", true, false, false, ID_CREATE_LIGHT_POINT},
        {"Spot Light", "", true, false, false, ID_CREATE_LIGHT_SPOT}
      }}
    };
    m_menus.push_back(gameObjectMenu);
    
    // Tools menu
    MenuDef toolsMenu;
    toolsMenu.name = "Tools";
    toolsMenu.items = {
      {"Reimport All", "", true, false, false, ID_REIMPORT_ALL},
      {"Project Settings...", "", true, false, false, ID_PROJECT_SETTINGS}
    };
    m_menus.push_back(toolsMenu);

    // Help menu
    MenuDef helpMenu;
    helpMenu.name = "Help";
    helpMenu.items = {
      {"Documentation", "F1", true, false, false, ID_DOCUMENTATION},
      {"", "", false, false, true, 0},
      {"About", "", true, false, false, ID_ABOUT}
    };
    m_menus.push_back(helpMenu);
  }
  
  void UpdateStandardMenuStates(bool canUndo, bool canRedo, 
                                bool hasSelection, bool hasLevel) override {
    m_canUndo = canUndo;
    m_canRedo = canRedo;
    m_hasSelection = hasSelection;
    m_hasLevel = hasLevel;
    
    // Update File menu
    SetItemEnabled("File", ID_SAVE, hasLevel);
    SetItemEnabled("File", ID_SAVE_AS, hasLevel);
    
    // Update Edit menu
    SetItemEnabled("Edit", ID_UNDO, canUndo);
    SetItemEnabled("Edit", ID_REDO, canRedo);
    SetItemEnabled("Edit", ID_CUT, hasSelection);
    SetItemEnabled("Edit", ID_COPY, hasSelection);
    SetItemEnabled("Edit", ID_PASTE, hasLevel);  // TODO: Check clipboard
    SetItemEnabled("Edit", ID_DUPLICATE, hasSelection);
    SetItemEnabled("Edit", ID_DELETE, hasSelection);
    
    // Update GameObject menu
    SetItemEnabled("GameObject", ID_CREATE_EMPTY, hasLevel);
    SetItemEnabled("GameObject", ID_CREATE_CUBE, hasLevel);
    SetItemEnabled("GameObject", ID_CREATE_SPHERE, hasLevel);
    SetItemEnabled("GameObject", ID_CREATE_PLANE, hasLevel);
    SetItemEnabled("GameObject", ID_CREATE_CYLINDER, hasLevel);
    SetItemEnabled("GameObject", ID_CREATE_CAMERA, hasLevel);
    SetItemEnabled("GameObject", ID_CREATE_LIGHT_DIR, hasLevel);
    SetItemEnabled("GameObject", ID_CREATE_LIGHT_POINT, hasLevel);
    SetItemEnabled("GameObject", ID_CREATE_LIGHT_SPOT, hasLevel);
  }

private:
  void DrawMenu(MenuDef& menu) {
    if (!ImGui::BeginMenu(menu.name.c_str())) return;

    for (auto& item : menu.items) {
      DrawMenuItem(menu.name.c_str(), item);
    }

    ImGui::EndMenu();
  }

  void DrawMenuItem(const char* menuName, MenuItem& item) {
    if (item.separator) {
      ImGui::Separator();
      return;
    }

    if (item.hasSubmenu) {
      if (ImGui::BeginMenu(item.label.c_str())) {
        for (auto& subItem : item.submenuItems) {
          DrawMenuItem(menuName, subItem);
        }
        ImGui::EndMenu();
      }
      return;
    }

    // Handle Recent Files submenu
    if (item.id == ID_RECENT_FILES) {
      if (ImGui::BeginMenu("Recent Files", !m_recentFiles.empty())) {
        for (size_t i = 0; i < m_recentFiles.size(); ++i) {
          if (ImGui::MenuItem(m_recentFiles[i].c_str())) {
            if (m_onRecentFileSelected) {
              m_onRecentFileSelected(m_recentFiles[i].c_str());
            }
          }
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Clear Recent Files")) {
          ClearRecentFiles();
        }
        ImGui::EndMenu();
      }
      return;
    }

    // Standard menu item
    if (ImGui::MenuItem(item.label.c_str(), item.shortcut.c_str(), item.checked, item.enabled)) {
      if (m_onMenuItemClicked) {
        m_onMenuItemClicked(menuName, item.id);
      }
    }
  }
  
  void SetItemEnabledInList(std::vector<MenuItem>& items, int itemId, bool enabled) {
    for (auto& item : items) {
      if (item.id == itemId) {
        item.enabled = enabled;
        return;
      }
      if (item.hasSubmenu) {
        SetItemEnabledInList(item.submenuItems, itemId, enabled);
      }
    }
  }
  
  void SetItemCheckedInList(std::vector<MenuItem>& items, int itemId, bool checked) {
    for (auto& item : items) {
      if (item.id == itemId) {
        item.checked = checked;
        return;
      }
      if (item.hasSubmenu) {
        SetItemCheckedInList(item.submenuItems, itemId, checked);
      }
    }
  }
  
  void SetItemLabelInList(std::vector<MenuItem>& items, int itemId, const char* label) {
    for (auto& item : items) {
      if (item.id == itemId) {
        item.label = label;
        return;
      }
      if (item.hasSubmenu) {
        SetItemLabelInList(item.submenuItems, itemId, label);
      }
    }
  }
  
  // Menu data
  std::vector<MenuDef> m_menus;
  std::vector<std::string> m_recentFiles;
  
  // State
  bool m_canUndo;
  bool m_canRedo;
  bool m_hasSelection;
  bool m_hasLevel;
  
  // Callbacks
  std::function<void(const char*, int)> m_onMenuItemClicked;
  std::function<void(const char*)> m_onRecentFileSelected;
};

IMainMenu* CreateMainMenu() {
  return new MainMenuImpl();
}

}  // namespace editor
}  // namespace te
