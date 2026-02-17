/**
 * @file EditorPreferences.cpp
 * @brief Editor preferences implementation (024-Editor).
 */
#include <te/editor/EditorPreferences.h>
#include <fstream>
#include <sstream>
#include <cstring>

namespace te {
namespace editor {

class EditorPreferencesImpl : public IEditorPreferences {
public:
  EditorPreferencesImpl() {
    ResetToDefaults();
    InitializeDefaultKeyBindings();
  }

  // === UI Settings ===
  
  void SetTheme(EditorTheme theme) override {
    m_uiPrefs.theme = theme;
    NotifyChange();
  }
  
  EditorTheme GetTheme() const override {
    return m_uiPrefs.theme;
  }
  
  void SetFontSize(float size) override {
    m_uiPrefs.fontSize = std::max(8.0f, std::min(32.0f, size));
    NotifyChange();
  }
  
  float GetFontSize() const override {
    return m_uiPrefs.fontSize;
  }
  
  void SetUIScale(float scale) override {
    m_uiPrefs.uiScale = std::max(0.5f, std::min(3.0f, scale));
    NotifyChange();
  }
  
  float GetUIScale() const override {
    return m_uiPrefs.uiScale;
  }
  
  void SetUIPreferences(UIPreferences const& prefs) override {
    m_uiPrefs = prefs;
    NotifyChange();
  }
  
  UIPreferences GetUIPreferences() const override {
    return m_uiPrefs;
  }
  
  // === Viewport Settings ===
  
  void SetViewportFOV(float fov) override {
    m_viewportPrefs.fov = std::max(10.0f, std::min(120.0f, fov));
    NotifyChange();
  }
  
  float GetViewportFOV() const override {
    return m_viewportPrefs.fov;
  }
  
  void SetViewportNearClip(float nearClip) override {
    m_viewportPrefs.nearClip = std::max(0.001f, nearClip);
    NotifyChange();
  }
  
  float GetViewportNearClip() const override {
    return m_viewportPrefs.nearClip;
  }
  
  void SetViewportFarClip(float farClip) override {
    m_viewportPrefs.farClip = std::max(1.0f, farClip);
    NotifyChange();
  }
  
  float GetViewportFarClip() const override {
    return m_viewportPrefs.farClip;
  }
  
  void SetViewportPreferences(ViewportPreferences const& prefs) override {
    m_viewportPrefs = prefs;
    NotifyChange();
  }
  
  ViewportPreferences GetViewportPreferences() const override {
    return m_viewportPrefs;
  }
  
  // === Auto-save Settings ===
  
  void SetAutoSaveEnabled(bool enabled) override {
    m_autoSave.enabled = enabled;
    NotifyChange();
  }
  
  bool IsAutoSaveEnabled() const override {
    return m_autoSave.enabled;
  }
  
  void SetAutoSaveInterval(int seconds) override {
    m_autoSave.intervalSeconds = std::max(30, seconds);
    NotifyChange();
  }
  
  int GetAutoSaveInterval() const override {
    return m_autoSave.intervalSeconds;
  }
  
  void SetAutoSaveSettings(AutoSaveSettings const& settings) override {
    m_autoSave = settings;
    NotifyChange();
  }
  
  AutoSaveSettings GetAutoSaveSettings() const override {
    return m_autoSave;
  }
  
  // === Key Bindings ===
  
  void SetKeyBinding(const char* actionId, KeyBinding const& binding) override {
    if (actionId) {
      m_keyBindings[actionId] = binding;
      NotifyChange();
    }
  }
  
  KeyBinding GetKeyBinding(const char* actionId) const override {
    if (!actionId) return KeyBinding{};
    
    auto it = m_keyBindings.find(actionId);
    if (it != m_keyBindings.end()) {
      return it->second;
    }
    
    // Return default if available
    auto defaultIt = m_defaultKeyBindings.find(actionId);
    if (defaultIt != m_defaultKeyBindings.end()) {
      return defaultIt->second;
    }
    
    return KeyBinding{};
  }
  
  void ResetKeyBinding(const char* actionId) override {
    if (!actionId) return;
    
    auto defaultIt = m_defaultKeyBindings.find(actionId);
    if (defaultIt != m_defaultKeyBindings.end()) {
      m_keyBindings[actionId] = defaultIt->second;
      NotifyChange();
    } else {
      m_keyBindings.erase(actionId);
    }
  }
  
  void ResetAllKeyBindings() override {
    m_keyBindings = m_defaultKeyBindings;
    NotifyChange();
  }
  
  // === External Tools ===
  
  void AddExternalTool(ExternalTool const& tool) override {
    // Remove existing tool with same name
    RemoveExternalTool(tool.name.c_str());
    m_externalTools.push_back(tool);
    NotifyChange();
  }
  
  void RemoveExternalTool(const char* name) override {
    if (!name) return;
    
    m_externalTools.erase(
      std::remove_if(m_externalTools.begin(), m_externalTools.end(),
        [name](ExternalTool const& t) { return t.name == name; }),
      m_externalTools.end());
    NotifyChange();
  }
  
  std::vector<ExternalTool> const& GetExternalTools() const override {
    return m_externalTools;
  }
  
  ExternalTool const* GetExternalTool(const char* name) const override {
    if (!name) return nullptr;
    
    for (auto const& tool : m_externalTools) {
      if (tool.name == name) {
        return &tool;
      }
    }
    return nullptr;
  }
  
  // === Persistence ===
  
  bool Save(const char* path) override {
    const char* filePath = path ? path : m_defaultPath.c_str();
    if (!filePath || !filePath[0]) return false;
    
    // Simple text-based save format
    std::ofstream file(filePath);
    if (!file.is_open()) return false;
    
    // UI preferences
    file << "[UI]\n";
    file << "theme=" << static_cast<int>(m_uiPrefs.theme) << "\n";
    file << "fontSize=" << m_uiPrefs.fontSize << "\n";
    file << "uiScale=" << m_uiPrefs.uiScale << "\n";
    file << "showTooltips=" << (m_uiPrefs.showTooltips ? 1 : 0) << "\n";
    file << "undoHistorySize=" << m_uiPrefs.undoHistorySize << "\n";
    
    // Viewport preferences
    file << "[Viewport]\n";
    file << "fov=" << m_viewportPrefs.fov << "\n";
    file << "nearClip=" << m_viewportPrefs.nearClip << "\n";
    file << "farClip=" << m_viewportPrefs.farClip << "\n";
    file << "cameraSpeed=" << m_viewportPrefs.cameraSpeed << "\n";
    file << "rotationSpeed=" << m_viewportPrefs.rotationSpeed << "\n";
    file << "showGrid=" << (m_viewportPrefs.showGrid ? 1 : 0) << "\n";
    file << "gridSize=" << m_viewportPrefs.gridSize << "\n";
    
    // Auto-save
    file << "[AutoSave]\n";
    file << "enabled=" << (m_autoSave.enabled ? 1 : 0) << "\n";
    file << "interval=" << m_autoSave.intervalSeconds << "\n";
    file << "maxBackups=" << m_autoSave.maxBackups << "\n";
    
    // Key bindings
    file << "[KeyBindings]\n";
    for (auto const& kb : m_keyBindings) {
      file << kb.first << "=" << kb.second.keyCode << "," 
           << (kb.second.ctrl ? 1 : 0) << ","
           << (kb.second.alt ? 1 : 0) << ","
           << (kb.second.shift ? 1 : 0) << "\n";
    }
    
    // External tools
    file << "[ExternalTools]\n";
    for (auto const& tool : m_externalTools) {
      file << tool.name << "=" << tool.path << "," << tool.arguments << "\n";
    }
    
    return true;
  }
  
  bool Load(const char* path) override {
    const char* filePath = path ? path : m_defaultPath.c_str();
    if (!filePath || !filePath[0]) return false;
    
    std::ifstream file(filePath);
    if (!file.is_open()) return false;
    
    std::string line;
    std::string section;
    
    while (std::getline(file, line)) {
      if (line.empty() || line[0] == '#') continue;
      
      // Check for section header
      if (line[0] == '[') {
        section = line.substr(1, line.find(']') - 1);
        continue;
      }
      
      // Parse key=value
      size_t eqPos = line.find('=');
      if (eqPos == std::string::npos) continue;
      
      std::string key = line.substr(0, eqPos);
      std::string value = line.substr(eqPos + 1);
      
      if (section == "UI") {
        ParseUIPreference(key, value);
      } else if (section == "Viewport") {
        ParseViewportPreference(key, value);
      } else if (section == "AutoSave") {
        ParseAutoSaveSetting(key, value);
      } else if (section == "KeyBindings") {
        ParseKeyBinding(key, value);
      } else if (section == "ExternalTools") {
        ParseExternalTool(key, value);
      }
    }
    
    NotifyChange();
    return true;
  }
  
  void ResetToDefaults() override {
    // UI defaults
    m_uiPrefs.theme = EditorTheme::Dark;
    m_uiPrefs.fontSize = 14.0f;
    m_uiPrefs.uiScale = 1.0f;
    m_uiPrefs.showTooltips = true;
    m_uiPrefs.undoHistorySize = 100;
    
    // Viewport defaults
    m_viewportPrefs.fov = 60.0f;
    m_viewportPrefs.nearClip = 0.1f;
    m_viewportPrefs.farClip = 1000.0f;
    m_viewportPrefs.cameraSpeed = 10.0f;
    m_viewportPrefs.rotationSpeed = 0.3f;
    m_viewportPrefs.showGrid = true;
    m_viewportPrefs.gridSize = 1.0f;
    m_viewportPrefs.showStats = false;
    
    // Auto-save defaults
    m_autoSave.enabled = true;
    m_autoSave.intervalSeconds = 300;
    m_autoSave.maxBackups = 10;
    
    // External tools
    m_externalTools.clear();
    
    NotifyChange();
  }
  
  // === Change Notification ===
  
  void SetOnPreferencesChanged(std::function<void()> callback) override {
    m_onPreferencesChanged = std::move(callback);
  }

private:
  void InitializeDefaultKeyBindings() {
    // Standard editor key bindings (using GLFW key codes)
    // W = 87, A = 65, S = 83, D = 68, Q = 81, E = 69
    // Z = 90, X = 88, C = 67, V = 86
    // F = 70, G = 71, R = 82
    // Ctrl = 341/345, Shift = 340/344, Alt = 342/346
    
    m_defaultKeyBindings["translate"] = {87, false, false, false};  // W
    m_defaultKeyBindings["rotate"] = {69, false, false, false};     // E
    m_defaultKeyBindings["scale"] = {82, false, false, false};      // R
    
    m_defaultKeyBindings["undo"] = {90, true, false, false};        // Ctrl+Z
    m_defaultKeyBindings["redo"] = {89, true, false, false};        // Ctrl+Y
    m_defaultKeyBindings["save"] = {83, true, false, false};        // Ctrl+S
    m_defaultKeyBindings["save_as"] = {83, true, false, true};      // Ctrl+Shift+S
    m_defaultKeyBindings["open"] = {79, true, false, false};        // Ctrl+O
    m_defaultKeyBindings["new"] = {78, true, false, false};         // Ctrl+N
    
    m_defaultKeyBindings["delete"] = {261, false, false, false};    // Delete
    m_defaultKeyBindings["copy"] = {67, true, false, false};        // Ctrl+C
    m_defaultKeyBindings["paste"] = {86, true, false, false};       // Ctrl+V
    m_defaultKeyBindings["cut"] = {88, true, false, false};         // Ctrl+X
    m_defaultKeyBindings["duplicate"] = {68, true, false, false};   // Ctrl+D
    m_defaultKeyBindings["select_all"] = {65, true, false, false};  // Ctrl+A
    
    m_defaultKeyBindings["focus"] = {70, false, false, false};      // F
    m_defaultKeyBindings["toggle_grid"] = {71, false, false, false}; // G
    
    // Camera bookmarks Ctrl+1-9
    for (int i = 0; i < 9; ++i) {
      char buf[16];
      snprintf(buf, sizeof(buf), "bookmark_%d", i);
      m_defaultKeyBindings[buf] = {49 + i, true, false, false};  // Ctrl+1-9
    }
    
    // Copy defaults to active bindings
    m_keyBindings = m_defaultKeyBindings;
  }
  
  void ParseUIPreference(std::string const& key, std::string const& value) {
    if (key == "theme") {
      m_uiPrefs.theme = static_cast<EditorTheme>(std::stoi(value));
    } else if (key == "fontSize") {
      m_uiPrefs.fontSize = std::stof(value);
    } else if (key == "uiScale") {
      m_uiPrefs.uiScale = std::stof(value);
    } else if (key == "showTooltips") {
      m_uiPrefs.showTooltips = std::stoi(value) != 0;
    } else if (key == "undoHistorySize") {
      m_uiPrefs.undoHistorySize = std::stoi(value);
    }
  }
  
  void ParseViewportPreference(std::string const& key, std::string const& value) {
    if (key == "fov") {
      m_viewportPrefs.fov = std::stof(value);
    } else if (key == "nearClip") {
      m_viewportPrefs.nearClip = std::stof(value);
    } else if (key == "farClip") {
      m_viewportPrefs.farClip = std::stof(value);
    } else if (key == "cameraSpeed") {
      m_viewportPrefs.cameraSpeed = std::stof(value);
    } else if (key == "rotationSpeed") {
      m_viewportPrefs.rotationSpeed = std::stof(value);
    } else if (key == "showGrid") {
      m_viewportPrefs.showGrid = std::stoi(value) != 0;
    } else if (key == "gridSize") {
      m_viewportPrefs.gridSize = std::stof(value);
    }
  }
  
  void ParseAutoSaveSetting(std::string const& key, std::string const& value) {
    if (key == "enabled") {
      m_autoSave.enabled = std::stoi(value) != 0;
    } else if (key == "interval") {
      m_autoSave.intervalSeconds = std::stoi(value);
    } else if (key == "maxBackups") {
      m_autoSave.maxBackups = std::stoi(value);
    }
  }
  
  void ParseKeyBinding(std::string const& action, std::string const& value) {
    KeyBinding binding = {};
    
    size_t pos = 0;
    size_t next = value.find(',');
    if (next != std::string::npos) {
      binding.keyCode = std::stoi(value.substr(pos, next - pos));
      pos = next + 1;
      
      next = value.find(',', pos);
      if (next != std::string::npos) {
        binding.ctrl = std::stoi(value.substr(pos, next - pos)) != 0;
        pos = next + 1;
        
        next = value.find(',', pos);
        if (next != std::string::npos) {
          binding.alt = std::stoi(value.substr(pos, next - pos)) != 0;
          pos = next + 1;
          binding.shift = std::stoi(value.substr(pos)) != 0;
        }
      }
    }
    
    m_keyBindings[action] = binding;
  }
  
  void ParseExternalTool(std::string const& name, std::string const& value) {
    ExternalTool tool;
    tool.name = name;
    
    size_t comma = value.find(',');
    if (comma != std::string::npos) {
      tool.path = value.substr(0, comma);
      tool.arguments = value.substr(comma + 1);
    } else {
      tool.path = value;
    }
    
    m_externalTools.push_back(tool);
  }
  
  void NotifyChange() {
    if (m_onPreferencesChanged) {
      m_onPreferencesChanged();
    }
  }
  
  // Preferences storage
  UIPreferences m_uiPrefs;
  ViewportPreferences m_viewportPrefs;
  AutoSaveSettings m_autoSave;
  
  // Key bindings
  std::map<std::string, KeyBinding> m_keyBindings;
  std::map<std::string, KeyBinding> m_defaultKeyBindings;
  
  // External tools
  std::vector<ExternalTool> m_externalTools;
  
  // File path
  std::string m_defaultPath = "editor_preferences.ini";
  
  // Callback
  std::function<void()> m_onPreferencesChanged;
};

IEditorPreferences* CreateEditorPreferences() {
  return new EditorPreferencesImpl();
}

}  // namespace editor
}  // namespace te
