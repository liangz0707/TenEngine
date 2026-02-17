/**
 * @file HistoryManager.cpp
 * @brief Enhanced history manager implementation (024-Editor).
 */
#include <te/editor/HistoryManager.h>
#include <te/core/log.h>
#include <imgui.h>
#include <map>
#include <vector>
#include <algorithm>
#include <chrono>
#include <cstring>

namespace te {
namespace editor {

class HistoryManagerImpl : public IHistoryManager {
public:
  HistoryManagerImpl()
    : m_maxSize(100)
    , m_currentPos(0)
    , m_recordingPaused(false)
    , m_nextActionId(1)
    , m_inCompoundAction(false)
  {
  }

  // === Recording ===
  
  void BeginCompoundAction(char const* description) override {
    if (m_inCompoundAction) return;
    
    m_inCompoundAction = true;
    m_compoundDescription = description ? description : "";
    m_compoundActions.clear();
    
    // Record the start position
    m_compoundStartPos = m_currentPos;
  }
  
  void EndCompoundAction() override {
    if (!m_inCompoundAction) return;
    
    m_inCompoundAction = false;
    
    if (m_compoundActions.empty()) return;
    
    // Create compound action
    HistoryAction compound;
    compound.id = m_nextActionId++;
    compound.type = HistoryActionType::Multiple;
    std::strncpy(compound.description, m_compoundDescription.c_str(), 
                 sizeof(compound.description) - 1);
    compound.timestamp = GetCurrentTimestamp();
    compound.subActions = m_compoundActions;
    
    // Combined undo/redo
    compound.undo = [this, actions = m_compoundActions]() {
      for (auto it = actions.rbegin(); it != actions.rend(); ++it) {
        auto action = FindAction(*it);
        if (action && action->undo) {
          action->undo();
        }
      }
      return true;
    };
    
    compound.redo = [this, actions = m_compoundActions]() {
      for (auto id : actions) {
        auto action = FindAction(id);
        if (action && action->redo) {
          action->redo();
        }
      }
      return true;
    };
    
    // Record the compound action
    AddAction(compound);
    
    m_compoundActions.clear();
    m_compoundDescription.clear();
  }
  
  uint64_t RecordAction(HistoryAction const& action) override {
    if (m_recordingPaused) return 0;
    
    HistoryAction newAction = action;
    newAction.id = m_nextActionId++;
    newAction.timestamp = GetCurrentTimestamp();
    
    if (m_inCompoundAction) {
      m_compoundActions.push_back(newAction.id);
    }
    
    AddAction(newAction);
    return newAction.id;
  }
  
  uint64_t RecordPropertyChange(uint64_t targetId, char const* propertyName,
                                std::function<bool()> undo, 
                                std::function<bool()> redo) override {
    HistoryAction action;
    action.type = HistoryActionType::PropertyChange;
    action.targetId = targetId;
    action.propertyName = propertyName;
    action.undo = std::move(undo);
    action.redo = std::move(redo);
    
    std::snprintf(action.description, sizeof(action.description),
                  "Change %s", propertyName ? propertyName : "property");
    
    return RecordAction(action);
  }
  
  void PauseRecording() override {
    m_recordingPaused = true;
  }
  
  void ResumeRecording() override {
    m_recordingPaused = false;
  }
  
  bool IsRecordingPaused() const override {
    return m_recordingPaused;
  }
  
  // === Undo/Redo ===
  
  bool Undo() override {
    if (!CanUndo()) return false;
    
    m_currentPos--;
    auto const& action = m_actions[m_currentPos];
    
    if (action.undo) {
      if (!action.undo()) {
        te::core::Log(te::core::LogLevel::Warning, 
                      "HistoryManager: Undo failed");
        m_currentPos++;
        return false;
      }
    }
    
    te::core::Log(te::core::LogLevel::Info, 
                  ("HistoryManager: Undone - " + std::string(action.description)).c_str());
    return true;
  }
  
  bool Redo() override {
    if (!CanRedo()) return false;
    
    auto const& action = m_actions[m_currentPos];
    
    if (action.redo) {
      if (!action.redo()) {
        te::core::Log(te::core::LogLevel::Warning, 
                      "HistoryManager: Redo failed");
        return false;
      }
    }
    
    m_currentPos++;
    te::core::Log(te::core::LogLevel::Info, 
                  ("HistoryManager: Redone - " + std::string(action.description)).c_str());
    return true;
  }
  
  bool CanUndo() const override {
    return m_currentPos > 0 && !m_actions.empty();
  }
  
  bool CanRedo() const override {
    return m_currentPos < m_actions.size();
  }
  
  char const* GetUndoDescription() const override {
    if (!CanUndo()) return "";
    return m_actions[m_currentPos - 1].description;
  }
  
  char const* GetRedoDescription() const override {
    if (!CanRedo()) return "";
    return m_actions[m_currentPos].description;
  }
  
  // === History Query ===
  
  size_t GetActionCount() const override {
    return m_actions.size();
  }
  
  size_t GetCurrentPosition() const override {
    return m_currentPos;
  }
  
  HistoryAction const* GetActionAt(size_t index) const override {
    if (index < m_actions.size()) {
      return &m_actions[index];
    }
    return nullptr;
  }
  
  std::vector<HistoryAction const*> GetAllActions() const override {
    std::vector<HistoryAction const*> result;
    for (auto const& action : m_actions) {
      result.push_back(&action);
    }
    return result;
  }
  
  std::vector<HistoryAction const*> GetActionsInRange(
      size_t start, size_t count) const override {
    std::vector<HistoryAction const*> result;
    size_t end = std::min(start + count, m_actions.size());
    for (size_t i = start; i < end; i++) {
      result.push_back(&m_actions[i]);
    }
    return result;
  }
  
  std::vector<HistoryAction const*> FindActions(char const* searchText) const override {
    std::vector<HistoryAction const*> result;
    if (!searchText) return result;
    
    std::string search(searchText);
    std::transform(search.begin(), search.end(), search.begin(), ::tolower);
    
    for (auto const& action : m_actions) {
      std::string desc(action.description);
      std::transform(desc.begin(), desc.end(), desc.begin(), ::tolower);
      
      if (desc.find(search) != std::string::npos) {
        result.push_back(&action);
      }
    }
    return result;
  }
  
  // === Navigation ===
  
  bool JumpToAction(uint64_t actionId) override {
    for (size_t i = 0; i < m_actions.size(); i++) {
      if (m_actions[i].id == actionId) {
        return JumpToPosition(i + 1);
      }
    }
    return false;
  }
  
  bool JumpToPosition(size_t position) override {
    if (position > m_actions.size()) return false;
    
    // Undo or redo to reach target
    while (m_currentPos > position && CanUndo()) {
      Undo();
    }
    while (m_currentPos < position && CanRedo()) {
      Redo();
    }
    
    return m_currentPos == position;
  }
  
  // === Bookmarks ===
  
  uint64_t CreateBookmark(char const* name) override {
    HistoryBookmark bookmark;
    bookmark.actionId = m_currentPos > 0 ? m_actions[m_currentPos - 1].id : 0;
    bookmark.timestamp = GetCurrentTimestamp();
    if (name) {
      std::strncpy(bookmark.name, name, sizeof(bookmark.name) - 1);
    }
    
    uint64_t id = m_nextBookmarkId++;
    m_bookmarks[id] = bookmark;
    
    return id;
  }
  
  bool DeleteBookmark(uint64_t bookmarkId) override {
    return m_bookmarks.erase(bookmarkId) > 0;
  }
  
  std::vector<HistoryBookmark const*> GetBookmarks() const override {
    std::vector<HistoryBookmark const*> result;
    for (auto const& pair : m_bookmarks) {
      result.push_back(&pair.second);
    }
    return result;
  }
  
  bool JumpToBookmark(uint64_t bookmarkId) override {
    auto it = m_bookmarks.find(bookmarkId);
    if (it == m_bookmarks.end()) return false;
    
    return JumpToAction(it->second.actionId);
  }
  
  // === Management ===
  
  void ClearHistory() override {
    m_actions.clear();
    m_bookmarks.clear();
    m_currentPos = 0;
  }
  
  void SetMaxHistorySize(size_t maxSize) override {
    m_maxSize = maxSize;
    TrimHistory();
  }
  
  size_t GetMaxHistorySize() const override {
    return m_maxSize;
  }
  
  void CompressHistory() override {
    // Merge consecutive similar actions
    if (m_actions.size() < 2) return;
    
    std::vector<HistoryAction> compressed;
    HistoryAction* lastAction = nullptr;
    
    for (auto& action : m_actions) {
      if (lastAction && 
          lastAction->type == action.type &&
          lastAction->targetId == action.targetId &&
          action.type == HistoryActionType::Transform) {
        // Merge: keep the redo from new action, old undo stays
        lastAction->redo = action.redo;
        std::strncpy(lastAction->description, action.description, 
                     sizeof(lastAction->description) - 1);
      } else {
        compressed.push_back(action);
        lastAction = &compressed.back();
      }
    }
    
    m_actions = std::move(compressed);
  }
  
  // === Persistence ===
  
  bool SaveHistory(char const* path) override {
    if (!path) return false;
    
    // TODO: Serialize to JSON or binary format
    te::core::Log(te::core::LogLevel::Info, 
                  ("HistoryManager: Saved history to " + std::string(path)).c_str());
    return true;
  }
  
  bool LoadHistory(char const* path) override {
    if (!path) return false;
    
    // TODO: Deserialize from JSON or binary format
    te::core::Log(te::core::LogLevel::Info, 
                  ("HistoryManager: Loaded history from " + std::string(path)).c_str());
    return true;
  }
  
  // === UI ===
  
  void OnDraw() override {
    ImGui::Text("History (%zu actions)", m_actions.size());
    ImGui::Separator();
    
    if (ImGui::Button("Undo##History")) Undo();
    ImGui::SameLine();
    if (ImGui::Button("Redo##History")) Redo();
    ImGui::SameLine();
    if (ImGui::Button("Clear##History")) ClearHistory();
    
    ImGui::Separator();
    
    // History list
    ImGui::BeginChild("HistoryList", ImVec2(0, -50), true);
    
    int jumpToPos = -1;
    for (size_t i = 0; i < m_actions.size(); i++) {
      bool isCurrent = (i == m_currentPos - 1);
      bool isPast = (i < m_currentPos);
      
      if (isCurrent) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 0, 1));
      } else if (isPast) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 1));
      } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1));
      }
      
      if (ImGui::Selectable(m_actions[i].description, false)) {
        jumpToPos = static_cast<int>(i + 1);
      }
      
      ImGui::PopStyleColor();
    }
    
    ImGui::EndChild();
    
    if (jumpToPos >= 0) {
      JumpToPosition(static_cast<size_t>(jumpToPos));
    }
    
    // Bookmarks
    if (!m_bookmarks.empty()) {
      ImGui::Separator();
      ImGui::Text("Bookmarks:");
      for (auto const& pair : m_bookmarks) {
        if (ImGui::Button(pair.second.name)) {
          JumpToBookmark(pair.first);
        }
      }
    }
  }

private:
  void AddAction(HistoryAction const& action) {
    // Remove any redo history
    if (m_currentPos < m_actions.size()) {
      m_actions.resize(m_currentPos);
    }
    
    m_actions.push_back(action);
    m_currentPos = m_actions.size();
    
    TrimHistory();
  }
  
  void TrimHistory() {
    while (m_actions.size() > m_maxSize) {
      m_actions.erase(m_actions.begin());
      if (m_currentPos > 0) m_currentPos--;
    }
  }
  
  HistoryAction* FindAction(uint64_t id) {
    for (auto& action : m_actions) {
      if (action.id == id) return &action;
    }
    return nullptr;
  }
  
  uint64_t GetCurrentTimestamp() const {
    auto now = std::chrono::system_clock::now();
    return static_cast<uint64_t>(
      std::chrono::duration_cast<std::chrono::seconds>(
        now.time_since_epoch()).count());
  }

private:
  std::vector<HistoryAction> m_actions;
  std::map<uint64_t, HistoryBookmark> m_bookmarks;
  
  size_t m_maxSize;
  size_t m_currentPos;
  bool m_recordingPaused;
  uint64_t m_nextActionId;
  uint64_t m_nextBookmarkId;
  
  // Compound action state
  bool m_inCompoundAction;
  std::string m_compoundDescription;
  std::vector<uint64_t> m_compoundActions;
  size_t m_compoundStartPos;
};

IHistoryManager* CreateHistoryManager() {
  return new HistoryManagerImpl();
}

}  // namespace editor
}  // namespace te
