/**
 * @file EditorScripting.cpp
 * @brief Editor scripting system implementation (024-Editor).
 */
#include <te/editor/EditorScripting.h>
#include <te/core/log.h>
#include <imgui.h>
#include <map>
#include <cstring>
#include <fstream>
#include <sstream>

namespace te {
namespace editor {

class EditorScriptingImpl : public IEditorScripting {
public:
  EditorScriptingImpl()
    : m_recordingMacro(false)
  {
  }

  // === Command Registration ===
  
  bool RegisterCommand(EditorCommand const& command) override {
    if (!command.id) return false;
    
    std::string id(command.id);
    if (m_commands.find(id) != m_commands.end()) {
      te::core::Log(te::core::LogLevel::Warning, 
                    ("EditorScripting: Command already registered: " + id).c_str());
      return false;
    }
    
    m_commands[id] = command;
    return true;
  }
  
  void UnregisterCommand(const char* commandId) override {
    if (!commandId) return;
    m_commands.erase(std::string(commandId));
  }
  
  void RegisterStandardCommands() override {
    // File commands
    RegisterCommand({
      Commands::NEW_SCENE, "New Scene", "File", "File/New Scene", 0,
      nullptr, nullptr, nullptr, true, true
    });
    RegisterCommand({
      Commands::OPEN_SCENE, "Open Scene", "File", "File/Open Scene", 100,
      nullptr, nullptr, nullptr, true, true
    });
    RegisterCommand({
      Commands::SAVE_SCENE, "Save Scene", "File", "File/Save", 200,
      nullptr, nullptr, nullptr, true, true
    });
    RegisterCommand({
      Commands::SAVE_SCENE_AS, "Save Scene As...", "File", "File/Save As...", 210,
      nullptr, nullptr, nullptr, true, true
    });
    RegisterCommand({
      Commands::EXIT, "Exit", "File", "File/Exit", 1000,
      nullptr, nullptr, nullptr, true, true
    });
    
    // Edit commands
    RegisterCommand({
      Commands::UNDO, "Undo", "Edit", "Edit/Undo", 0,
      nullptr, nullptr, nullptr, true, true
    });
    RegisterCommand({
      Commands::REDO, "Redo", "Edit", "Edit/Redo", 10,
      nullptr, nullptr, nullptr, true, true
    });
    RegisterCommand({
      Commands::CUT, "Cut", "Edit", "Edit/Cut", 100,
      nullptr, nullptr, nullptr, true, true
    });
    RegisterCommand({
      Commands::COPY, "Copy", "Edit", "Edit/Copy", 110,
      nullptr, nullptr, nullptr, true, true
    });
    RegisterCommand({
      Commands::PASTE, "Paste", "Edit", "Edit/Paste", 120,
      nullptr, nullptr, nullptr, true, true
    });
    RegisterCommand({
      Commands::DUPLICATE, "Duplicate", "Edit", "Edit/Duplicate", 130,
      nullptr, nullptr, nullptr, true, true
    });
    RegisterCommand({
      Commands::DELETE, "Delete", "Edit", "Edit/Delete", 200,
      nullptr, nullptr, nullptr, true, true
    });
    
    // GameObject commands
    RegisterCommand({
      Commands::CREATE_EMPTY, "Create Empty", "GameObject", "GameObject/Create Empty", 0,
      nullptr, nullptr, nullptr, true, true
    });
    RegisterCommand({
      Commands::CREATE_CUBE, "Create Cube", "GameObject", "GameObject/3D Object/Cube", 100,
      nullptr, nullptr, nullptr, true, true
    });
    RegisterCommand({
      Commands::CREATE_SPHERE, "Create Sphere", "GameObject", "GameObject/3D Object/Sphere", 110,
      nullptr, nullptr, nullptr, true, true
    });
    RegisterCommand({
      Commands::CREATE_LIGHT, "Create Light", "GameObject", "GameObject/Light/Directional Light", 200,
      nullptr, nullptr, nullptr, true, true
    });
    RegisterCommand({
      Commands::CREATE_CAMERA, "Create Camera", "GameObject", "GameObject/Camera", 300,
      nullptr, nullptr, nullptr, true, true
    });
    
    // View commands
    RegisterCommand({
      Commands::TOGGLE_GRID, "Toggle Grid", "View", "View/Grid", 0,
      nullptr, nullptr, nullptr, true, true
    });
    RegisterCommand({
      Commands::FRAME_SELECTED, "Frame Selected", "View", "View/Frame Selected", 100,
      nullptr, nullptr, nullptr, true, true
    });
    
    // Tools commands
    RegisterCommand({
      Commands::EDITOR_PREFERENCES, "Editor Preferences", "Tools", "Tools/Editor Preferences", 0,
      nullptr, nullptr, nullptr, true, true
    });
    RegisterCommand({
      Commands::PROJECT_SETTINGS, "Project Settings", "Tools", "Tools/Project Settings", 100,
      nullptr, nullptr, nullptr, true, true
    });
  }
  
  // === Command Execution ===
  
  bool ExecuteCommand(const char* commandId, CommandContext* ctx) override {
    if (!commandId) return false;
    
    auto it = m_commands.find(std::string(commandId));
    if (it == m_commands.end()) {
      te::core::Log(te::core::LogLevel::Warning, 
                    ("EditorScripting: Command not found: " + std::string(commandId)).c_str());
      return false;
    }
    
    if (!it->second.enabled) return false;
    if (it->second.canExecute && !it->second.canExecute(ctx ? *ctx : m_defaultContext)) {
      return false;
    }
    
    // Record to macro if recording
    if (m_recordingMacro && !m_currentMacroName.empty()) {
      m_currentMacroCommands.push_back(commandId);
    }
    
    // Execute
    if (it->second.execute) {
      it->second.execute(ctx ? *ctx : m_defaultContext);
      return true;
    }
    
    return false;
  }
  
  bool CanExecuteCommand(const char* commandId) const override {
    if (!commandId) return false;
    
    auto it = m_commands.find(std::string(commandId));
    if (it == m_commands.end()) return false;
    
    if (!it->second.enabled) return false;
    if (it->second.canExecute) {
      return it->second.canExecute(m_defaultContext);
    }
    return true;
  }
  
  bool IsCommandChecked(const char* commandId) const override {
    if (!commandId) return false;
    
    auto it = m_commands.find(std::string(commandId));
    if (it == m_commands.end()) return false;
    
    if (it->second.isChecked) {
      return it->second.isChecked(m_defaultContext);
    }
    return false;
  }
  
  // === Command Query ===
  
  EditorCommand const* GetCommand(const char* commandId) const override {
    if (!commandId) return nullptr;
    
    auto it = m_commands.find(std::string(commandId));
    if (it != m_commands.end()) {
      return &it->second;
    }
    return nullptr;
  }
  
  std::vector<EditorCommand const*> GetAllCommands() const override {
    std::vector<EditorCommand const*> result;
    for (auto const& pair : m_commands) {
      result.push_back(&pair.second);
    }
    return result;
  }
  
  std::vector<EditorCommand const*> GetCommandsByCategory(const char* category) const override {
    std::vector<EditorCommand const*> result;
    for (auto const& pair : m_commands) {
      if (pair.second.category && strcmp(pair.second.category, category) == 0) {
        result.push_back(&pair.second);
      }
    }
    return result;
  }
  
  std::vector<const char*> GetCategories() const override {
    std::vector<const char*> categories;
    std::map<std::string, bool> seen;
    
    for (auto const& pair : m_commands) {
      if (pair.second.category && seen.find(pair.second.category) == seen.end()) {
        categories.push_back(pair.second.category);
        seen[pair.second.category] = true;
      }
    }
    return categories;
  }
  
  // === Menu Integration ===
  
  const char* GetCommandMenuPath(const char* commandId) const override {
    auto cmd = GetCommand(commandId);
    return cmd ? cmd->menuItem : nullptr;
  }
  
  std::vector<EditorCommand const*> GetCommandsForMenu(const char* menuPath) const override {
    std::vector<EditorCommand const*> result;
    if (!menuPath) return result;
    
    size_t pathLen = strlen(menuPath);
    for (auto const& pair : m_commands) {
      if (pair.second.menuItem) {
        // Check if command starts with menu path
        if (strncmp(pair.second.menuItem, menuPath, pathLen) == 0) {
          if (pair.second.menuItem[pathLen] == '/' || pair.second.menuItem[pathLen] == '\0') {
            result.push_back(&pair.second);
          }
        }
      }
    }
    
    // Sort by priority
    std::sort(result.begin(), result.end(), 
              [](EditorCommand const* a, EditorCommand const* b) {
                return a->priority < b->priority;
              });
    
    return result;
  }
  
  // === Macros ===
  
  void CreateMacro(const char* name) override {
    if (!name) return;
    
    m_recordingMacro = true;
    m_currentMacroName = name;
    m_currentMacroCommands.clear();
  }
  
  void AddToMacro(const char* commandId) override {
    if (m_recordingMacro && commandId) {
      m_currentMacroCommands.push_back(commandId);
    }
  }
  
  void FinishMacro() override {
    if (!m_recordingMacro) return;
    
    if (!m_currentMacroName.empty() && !m_currentMacroCommands.empty()) {
      EditorMacro macro;
      std::strncpy(macro.name, m_currentMacroName.c_str(), sizeof(macro.name) - 1);
      macro.commandIds = m_currentMacroCommands;
      macro.enabled = true;
      m_macros.push_back(macro);
    }
    
    m_recordingMacro = false;
    m_currentMacroName.clear();
    m_currentMacroCommands.clear();
  }
  
  bool ExecuteMacro(const char* name) override {
    if (!name) return false;
    
    for (auto const& macro : m_macros) {
      if (strcmp(macro.name, name) == 0) {
        for (auto const& cmdId : macro.commandIds) {
          ExecuteCommand(cmdId);
        }
        return true;
      }
    }
    return false;
  }
  
  std::vector<EditorMacro const*> GetMacros() const override {
    std::vector<EditorMacro const*> result;
    for (auto const& macro : m_macros) {
      result.push_back(&macro);
    }
    return result;
  }
  
  void DeleteMacro(const char* name) override {
    if (!name) return;
    
    for (auto it = m_macros.begin(); it != m_macros.end(); ++it) {
      if (strcmp(it->name, name) == 0) {
        m_macros.erase(it);
        return;
      }
    }
  }
  
  // === Script Execution ===
  
  bool ExecuteScript(char const* path) override {
    if (!path) return false;
    
    std::ifstream file(path);
    if (!file.is_open()) {
      te::core::Log(te::core::LogLevel::Error, 
                    ("EditorScripting: Cannot open script file: " + std::string(path)).c_str());
      return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string script = buffer.str();
    
    return ExecuteScriptString(script.c_str());
  }
  
  bool ExecuteScriptString(char const* script) override {
    if (!script) return false;
    
    // Simple command-per-line script format
    // Each line is a command ID
    std::istringstream stream(script);
    std::string line;
    bool anyExecuted = false;
    
    while (std::getline(stream, line)) {
      // Skip empty lines and comments
      if (line.empty() || line[0] == '#') continue;
      
      // Trim whitespace
      size_t start = line.find_first_not_of(" \t\r\n");
      size_t end = line.find_last_not_of(" \t\r\n");
      if (start == std::string::npos) continue;
      
      std::string commandId = line.substr(start, end - start + 1);
      
      if (ExecuteCommand(commandId.c_str())) {
        anyExecuted = true;
      }
    }
    
    return anyExecuted;
  }
  
  // === Persistence ===
  
  bool SaveMacros(char const* path) override {
    if (!path) return false;
    
    std::ofstream file(path);
    if (!file.is_open()) return false;
    
    for (auto const& macro : m_macros) {
      file << "macro:" << macro.name << "\n";
      for (auto const& cmdId : macro.commandIds) {
        file << "  " << cmdId << "\n";
      }
    }
    
    return true;
  }
  
  bool LoadMacros(char const* path) override {
    if (!path) return false;
    
    std::ifstream file(path);
    if (!file.is_open()) return false;
    
    std::string line;
    EditorMacro* currentMacro = nullptr;
    
    while (std::getline(file, line)) {
      if (line.substr(0, 6) == "macro:") {
        m_macros.push_back(EditorMacro{});
        currentMacro = &m_macros.back();
        std::strncpy(currentMacro->name, line.substr(6).c_str(), sizeof(currentMacro->name) - 1);
      } else if (currentMacro && line.substr(0, 2) == "  ") {
        // Command in macro
        currentMacro->commandIds.push_back(strdup(line.substr(2).c_str()));
      }
    }
    
    return true;
  }

private:
  std::map<std::string, EditorCommand> m_commands;
  CommandContext m_defaultContext;
  
  std::vector<EditorMacro> m_macros;
  bool m_recordingMacro;
  std::string m_currentMacroName;
  std::vector<const char*> m_currentMacroCommands;
};

IEditorScripting* CreateEditorScripting() {
  return new EditorScriptingImpl();
}

}  // namespace editor
}  // namespace te
