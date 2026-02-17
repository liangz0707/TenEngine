/**
 * @file SceneSearch.cpp
 * @brief Scene search implementation (024-Editor).
 */
#include <te/editor/SceneSearch.h>
#include <imgui.h>
#include <cstring>
#include <algorithm>

namespace te {
namespace editor {

class SceneSearchImpl : public ISceneSearch {
public:
  SceneSearchImpl()
    : m_maxHistorySize(20)
    , m_searchFocused(false)
  {
    m_filter.namePattern = "";
    m_results.clear();
  }

  // === Search Operations ===
  
  size_t Search() override {
    m_results.clear();
    
    // TODO: Integrate with SceneView/EntityManager to get actual entities
    // For now, this is a placeholder that would be connected to the scene
    
    if (m_onSearchComplete) {
      m_onSearchComplete();
    }
    
    return m_results.size();
  }
  
  void SetFilter(SceneSearchFilter const& filter) override {
    m_filter = filter;
    
    // Add to history if pattern is not empty
    if (filter.namePattern && filter.namePattern[0] != '\0') {
      AddToHistory(filter.namePattern);
    }
  }
  
  SceneSearchFilter const& GetFilter() const override {
    return m_filter;
  }
  
  void ClearResults() override {
    m_results.clear();
  }
  
  // === Results ===
  
  std::vector<te::entity::EntityId> const& GetResults() const override {
    return m_results;
  }
  
  size_t GetResultCount() const override {
    return m_results.size();
  }
  
  bool MatchesFilter(te::entity::EntityId id) const override {
    // TODO: Implement actual filtering logic
    (void)id;
    return true;
  }
  
  // === History ===
  
  std::vector<const char*> const& GetHistory() const override {
    return m_history;
  }
  
  void ClearHistory() override {
    m_history.clear();
  }
  
  void SetMaxHistorySize(size_t size) override {
    m_maxHistorySize = size;
    while (m_history.size() > m_maxHistorySize) {
      m_history.pop_back();
    }
  }
  
  // === Callbacks ===
  
  void SetOnSearchComplete(std::function<void()> callback) override {
    m_onSearchComplete = std::move(callback);
  }
  
  // === Drawing ===
  
  void OnDraw() override {
    // Search bar
    ImGui::SetNextItemWidth(-40.0f);
    
    bool searchNow = false;
    if (ImGui::InputTextWithHint("##SceneSearch", "Search...", m_searchBuffer, sizeof(m_searchBuffer),
                                  ImGuiInputTextFlags_EnterReturnsTrue)) {
      searchNow = true;
    }
    
    m_searchFocused = ImGui::IsItemFocused();
    
    ImGui::SameLine();
    if (ImGui::Button("Search") || searchNow) {
      m_filter.namePattern = m_searchBuffer;
      Search();
    }
    
    // Filter options
    if (ImGui::BeginPopupContextItem("SearchOptions")) {
      ImGui::Checkbox("Case Sensitive", &m_filter.caseSensitive);
      ImGui::Checkbox("Search Components", &m_filter.searchComponents);
      ImGui::Checkbox("Include Inactive", &m_filter.searchInactive);
      ImGui::EndPopup();
    }
    
    // Show result count
    if (!m_results.empty()) {
      ImGui::Text("%zu results", m_results.size());
    }
    
    // History dropdown
    if (ImGui::BeginPopup("SearchHistory")) {
      ImGui::Text("Recent Searches:");
      ImGui::Separator();
      for (auto const& item : m_history) {
        if (ImGui::Selectable(item)) {
          std::strncpy(m_searchBuffer, item, sizeof(m_searchBuffer) - 1);
          m_searchBuffer[sizeof(m_searchBuffer) - 1] = '\0';
          m_filter.namePattern = m_searchBuffer;
          Search();
          ImGui::CloseCurrentPopup();
        }
      }
      ImGui::EndPopup();
    }
    
    // Right-click on search bar for history
    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1)) {
      ImGui::OpenPopup("SearchHistory");
    }
  }
  
  bool IsSearchFocused() const override {
    return m_searchFocused;
  }
  
  void FocusSearch() override {
    ImGui::SetKeyboardFocusHere();
    ImGui::SetNextItemWidth(-40.0f);
  }

private:
  void AddToHistory(const char* pattern) {
    if (!pattern || pattern[0] == '\0') return;
    
    // Check if already in history
    for (auto const& item : m_history) {
      if (std::strcmp(item, pattern) == 0) {
        return;  // Already exists
      }
    }
    
    // Add new entry (note: in production, this should copy the string)
    m_history.push_back(pattern);
    
    // Trim history
    while (m_history.size() > m_maxHistorySize) {
      m_history.pop_back();
    }
  }

private:
  SceneSearchFilter m_filter;
  std::vector<te::entity::EntityId> m_results;
  std::vector<const char*> m_history;
  size_t m_maxHistorySize;
  bool m_searchFocused;
  
  char m_searchBuffer[256] = "";
  
  std::function<void()> m_onSearchComplete;
};

ISceneSearch* CreateSceneSearch() {
  return new SceneSearchImpl();
}

}  // namespace editor
}  // namespace te
