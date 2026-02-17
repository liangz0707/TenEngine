/**
 * @file KeyBindingSystem.cpp
 * @brief Key binding system implementation (024-Editor).
 */
#include <te/editor/KeyBindingSystem.h>
#include <imgui.h>
#include <map>
#include <vector>
#include <cstring>
#include <fstream>
#include <sstream>

namespace te {
namespace editor {

// Simple hash for key combos
struct KeyComboHash {
  size_t operator()(KeyCombo const& k) const {
    return static_cast<size_t>(k.keyCode) |
           (k.ctrl ? 0x1000 : 0) |
           (k.alt ? 0x2000 : 0) |
           (k.shift ? 0x4000 : 0);
  }
};

class KeyBindingSystemImpl : public IKeyBindingSystem {
public:
  KeyBindingSystemImpl()
    : m_rebinding(false)
    , m_rebindingActionId(nullptr)
  {
  }

  // === Registration ===
  
  bool RegisterAction(KeyBindingAction const& action) override {
    if (!action.id) return false;
    
    std::string id(action.id);
    if (m_actions.find(id) != m_actions.end()) {
      return false;  // Already registered
    }
    
    KeyBindingAction newAction = action;
    newAction.currentBinding = action.defaultBinding;
    m_actions[id] = newAction;
    
    // Update lookup map
    if (newAction.currentBinding.keyCode != 0) {
      m_keyToAction[newAction.currentBinding] = id;
    }
    
    return true;
  }
  
  void UnregisterAction(const char* actionId) override {
    if (!actionId) return;
    
    std::string id(actionId);
    auto it = m_actions.find(id);
    if (it != m_actions.end()) {
      // Remove from lookup
      if (it->second.currentBinding.keyCode != 0) {
        m_keyToAction.erase(it->second.currentBinding);
      }
      m_actions.erase(it);
    }
  }
  
  void RegisterStandardActions() override {
    // File actions
    RegisterAction({Actions::NEW_SCENE, "New Scene", "File", {0, true, false, false}});  // Ctrl+N
    RegisterAction({Actions::OPEN_SCENE, "Open Scene", "File", {0, true, false, false}}); // Ctrl+O
    RegisterAction({Actions::SAVE_SCENE, "Save Scene", "File", {0, true, false, false}}); // Ctrl+S
    
    // Edit actions
    RegisterAction({Actions::UNDO, "Undo", "Edit", {0, true, false, false}});   // Ctrl+Z
    RegisterAction({Actions::REDO, "Redo", "Edit", {0, true, false, true}});    // Ctrl+Shift+Z
    RegisterAction({Actions::CUT, "Cut", "Edit", {0, true, false, false}});     // Ctrl+X
    RegisterAction({Actions::COPY, "Copy", "Edit", {0, true, false, false}});   // Ctrl+C
    RegisterAction({Actions::PASTE, "Paste", "Edit", {0, true, false, false}}); // Ctrl+V
    RegisterAction({Actions::DUPLICATE, "Duplicate", "Edit", {0, true, false, false}}); // Ctrl+D
    RegisterAction({Actions::DELETE, "Delete", "Edit", {0, false, false, false}});      // Delete
    RegisterAction({Actions::SELECT_ALL, "Select All", "Edit", {0, true, false, false}}); // Ctrl+A
    
    // Transform actions
    RegisterAction({Actions::SELECT_TOOL, "Select Tool", "Transform", {0, false, false, false}});   // Q
    RegisterAction({Actions::MOVE_TOOL, "Move Tool", "Transform", {0, false, false, false}});       // W
    RegisterAction({Actions::ROTATE_TOOL, "Rotate Tool", "Transform", {0, false, false, false}});   // E
    RegisterAction({Actions::SCALE_TOOL, "Scale Tool", "Transform", {0, false, false, false}});     // R
    
    // Play actions
    RegisterAction({Actions::PLAY, "Play", "Play", {0, false, false, false}});         // F5
    RegisterAction({Actions::PAUSE, "Pause", "Play", {0, false, false, false}});       // F6
    RegisterAction({Actions::STOP, "Stop", "Play", {0, false, false, false}});         // F7
    RegisterAction({Actions::STEP, "Step Frame", "Play", {0, false, false, false}});   // F8
  }
  
  // === Query ===
  
  KeyBindingAction const* GetAction(const char* actionId) const override {
    if (!actionId) return nullptr;
    
    auto it = m_actions.find(std::string(actionId));
    if (it != m_actions.end()) {
      return &it->second;
    }
    return nullptr;
  }
  
  std::vector<KeyBindingAction const*> GetActionsByCategory(const char* category) const override {
    std::vector<KeyBindingAction const*> result;
    for (auto const& pair : m_actions) {
      if (pair.second.category && strcmp(pair.second.category, category) == 0) {
        result.push_back(&pair.second);
      }
    }
    return result;
  }
  
  std::vector<const char*> GetCategories() const override {
    std::vector<const char*> categories;
    std::map<std::string, bool> seen;
    
    for (auto const& pair : m_actions) {
      if (pair.second.category && seen.find(pair.second.category) == seen.end()) {
        categories.push_back(pair.second.category);
        seen[pair.second.category] = true;
      }
    }
    return categories;
  }
  
  KeyCombo GetBinding(const char* actionId) const override {
    auto action = GetAction(actionId);
    if (action) {
      return action->currentBinding;
    }
    return {};
  }
  
  const char* FindActionByKeyCombo(KeyCombo const& combo) const override {
    auto it = m_keyToAction.find(combo);
    if (it != m_keyToAction.end()) {
      return it->second.c_str();
    }
    return nullptr;
  }
  
  // === Modification ===
  
  bool SetBinding(const char* actionId, KeyCombo const& combo) override {
    if (!actionId) return false;
    
    auto it = m_actions.find(std::string(actionId));
    if (it == m_actions.end()) return false;
    
    // Remove old binding from lookup
    if (it->second.currentBinding.keyCode != 0) {
      m_keyToAction.erase(it->second.currentBinding);
    }
    
    // Set new binding
    it->second.currentBinding = combo;
    
    // Add to lookup
    if (combo.keyCode != 0) {
      m_keyToAction[combo] = std::string(actionId);
    }
    
    return true;
  }
  
  void ResetBinding(const char* actionId) override {
    if (!actionId) return;
    
    auto it = m_actions.find(std::string(actionId));
    if (it != m_actions.end()) {
      SetBinding(actionId, it->second.defaultBinding);
    }
  }
  
  void ResetAllBindings() override {
    for (auto& pair : m_actions) {
      SetBinding(pair.first.c_str(), pair.second.defaultBinding);
    }
  }
  
  void ClearBinding(const char* actionId) override {
    SetBinding(actionId, {});
  }
  
  // === Execution ===
  
  bool ProcessKeyEvent(KeyCombo const& combo) override {
    if (combo.keyCode == 0) return false;
    
    const char* actionId = FindActionByKeyCombo(combo);
    if (actionId) {
      return ExecuteAction(actionId);
    }
    return false;
  }
  
  bool ExecuteAction(const char* actionId) override {
    if (!actionId) return false;
    
    auto it = m_actions.find(std::string(actionId));
    if (it == m_actions.end()) return false;
    
    if (!it->second.enabled || !it->second.callback) return false;
    
    it->second.callback();
    return true;
  }
  
  // === Persistence ===
  
  bool SaveBindings(char const* path) override {
    if (!path) return false;
    
    std::ofstream file(path);
    if (!file.is_open()) return false;
    
    for (auto const& pair : m_actions) {
      auto const& action = pair.second;
      file << action.id << "=";
      file << action.currentBinding.keyCode << ",";
      file << (action.currentBinding.ctrl ? 1 : 0) << ",";
      file << (action.currentBinding.alt ? 1 : 0) << ",";
      file << (action.currentBinding.shift ? 1 : 0) << "\n";
    }
    
    return true;
  }
  
  bool LoadBindings(char const* path) override {
    if (!path) return false;
    
    std::ifstream file(path);
    if (!file.is_open()) return false;
    
    std::string line;
    while (std::getline(file, line)) {
      // Parse line: actionId=keyCode,ctrl,alt,shift
      size_t eqPos = line.find('=');
      if (eqPos == std::string::npos) continue;
      
      std::string actionId = line.substr(0, eqPos);
      std::string values = line.substr(eqPos + 1);
      
      KeyCombo combo;
      size_t pos = 0;
      int field = 0;
      
      while ((pos = values.find(',')) != std::string::npos || field < 4) {
        std::string token = (pos != std::string::npos) ? 
                            values.substr(0, pos) : values;
        
        switch (field) {
          case 0: combo.keyCode = std::stoi(token); break;
          case 1: combo.ctrl = (std::stoi(token) != 0); break;
          case 2: combo.alt = (std::stoi(token) != 0); break;
          case 3: combo.shift = (std::stoi(token) != 0); break;
        }
        
        if (pos == std::string::npos) break;
        values = values.substr(pos + 1);
        field++;
      }
      
      SetBinding(actionId.c_str(), combo);
    }
    
    return true;
  }
  
  // === UI ===
  
  void OnDraw() override {
    ImGui::Text("Key Bindings");
    ImGui::Separator();
    
    auto categories = GetCategories();
    
    for (auto const& category : categories) {
      if (ImGui::CollapsingHeader(category)) {
        ImGui::Indent();
        
        auto actions = GetActionsByCategory(category);
        for (auto const& action : actions) {
          DrawActionBinding(action);
        }
        
        ImGui::Unindent();
      }
    }
    
    ImGui::Separator();
    if (ImGui::Button("Reset All to Defaults")) {
      ResetAllBindings();
    }
    
    // Show conflicts
    auto conflicts = FindConflicts();
    if (!conflicts.empty()) {
      ImGui::Separator();
      ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "Conflicts detected!");
      for (auto const& actionId : conflicts) {
        ImGui::BulletText("%s", actionId);
      }
    }
  }
  
  void StartRebind(const char* actionId) override {
    m_rebinding = true;
    m_rebindingActionId = actionId;
  }
  
  void CancelRebind() override {
    m_rebinding = false;
    m_rebindingActionId = nullptr;
  }
  
  bool IsRebinding() const override {
    return m_rebinding;
  }
  
  std::vector<const char*> FindConflicts() const override {
    std::vector<const char*> conflicts;
    std::map<KeyCombo, std::string, KeyComboHash> usedCombos;
    
    for (auto const& pair : m_actions) {
      if (pair.second.currentBinding.keyCode == 0) continue;
      
      auto const& combo = pair.second.currentBinding;
      auto it = usedCombos.find(combo);
      
      if (it != usedCombos.end()) {
        // Found a conflict
        bool found1 = false, found2 = false;
        for (auto const& c : conflicts) {
          if (strcmp(c, it->second.c_str()) == 0) found1 = true;
          if (strcmp(c, pair.first.c_str()) == 0) found2 = true;
        }
        if (!found1) conflicts.push_back(it->second.c_str());
        if (!found2) conflicts.push_back(pair.first.c_str());
      } else {
        usedCombos[combo] = pair.first;
      }
    }
    
    return conflicts;
  }

private:
  void DrawActionBinding(KeyBindingAction const* action) {
    ImGui::PushID(action->id);
    
    ImGui::Text("%s", action->displayName);
    ImGui::SameLine(200.0f);
    
    std::string bindingStr = FormatBinding(action->currentBinding);
    
    if (m_rebinding && m_rebindingActionId && 
        strcmp(m_rebindingActionId, action->id) == 0) {
      // Rebinding mode
      ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.4f, 0.1f, 1.0f));
      if (ImGui::Button("Press any key...")) {
        CancelRebind();
      }
      ImGui::PopStyleColor();
      
      // Check for key press
      ImGuiIO& io = ImGui::GetIO();
      for (int i = 0; i < 512; i++) {
        if (ImGui::IsKeyPressed(i)) {
          KeyCombo combo;
          combo.keyCode = i;
          combo.ctrl = io.KeyCtrl;
          combo.alt = io.KeyAlt;
          combo.shift = io.KeyShift;
          SetBinding(action->id, combo);
          CancelRebind();
          break;
        }
      }
    } else {
      if (ImGui::Button(bindingStr.c_str(), ImVec2(120, 0))) {
        StartRebind(action->id);
      }
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Reset")) {
      ResetBinding(action->id);
    }
    
    ImGui::PopID();
  }
  
  std::string FormatBinding(KeyCombo const& combo) const {
    if (combo.keyCode == 0) return "None";
    
    std::string result;
    if (combo.ctrl) result += "Ctrl+";
    if (combo.alt) result += "Alt+";
    if (combo.shift) result += "Shift+";
    
    // Convert key code to string
    // This is a simplified version
    if (combo.keyCode >= 'A' && combo.keyCode <= 'Z') {
      result += static_cast<char>(combo.keyCode);
    } else {
      result += std::to_string(combo.keyCode);
    }
    
    return result;
  }

private:
  std::map<std::string, KeyBindingAction> m_actions;
  std::unordered_map<KeyCombo, std::string, KeyComboHash> m_keyToAction;
  
  bool m_rebinding;
  const char* m_rebindingActionId;
};

IKeyBindingSystem* CreateKeyBindingSystem() {
  return new KeyBindingSystemImpl();
}

}  // namespace editor
}  // namespace te
