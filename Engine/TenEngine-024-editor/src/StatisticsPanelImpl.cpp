/**
 * @file StatisticsPanelImpl.cpp
 * @brief Scene statistics panel implementation (024-Editor).
 */
#include <te/editor/StatisticsPanel.h>
#include <imgui.h>
#include <chrono>
#include <algorithm>
#include <cstring>

namespace te {
namespace editor {

class StatisticsPanelImpl : public IStatisticsPanel {
public:
  StatisticsPanelImpl()
    : m_entityCount(0)
    , m_rootEntityCount(0)
    , m_activeEntityCount(0)
    , m_autoRefreshInterval(1.0f)
    , m_visible(true)
    , m_refreshRequested(false)
  {
    m_boundsMin = math::Vec3{0.0f, 0.0f, 0.0f};
    m_boundsMax = math::Vec3{0.0f, 0.0f, 0.0f};
    m_lastRefresh = std::chrono::steady_clock::now();
  }

  // === Drawing ===
  
  void OnDraw() override {
    if (!m_visible) return;
    
    if (!ImGui::Begin("Statistics", &m_visible)) {
      ImGui::End();
      return;
    }
    
    CheckAutoRefresh();
    
    DrawEntityStats();
    ImGui::Separator();
    
    DrawComponentStats();
    ImGui::Separator();
    
    DrawSceneBounds();
    ImGui::Separator();
    
    DrawRefreshControls();
    
    ImGui::End();
  }
  
  // === Entity Stats ===
  
  void SetEntityCount(int count) override {
    m_entityCount = count;
  }
  
  int GetEntityCount() const override {
    return m_entityCount;
  }
  
  void SetRootEntityCount(int count) override {
    m_rootEntityCount = count;
  }
  
  int GetRootEntityCount() const override {
    return m_rootEntityCount;
  }
  
  void SetActiveEntityCount(int count) override {
    m_activeEntityCount = count;
  }
  
  int GetActiveEntityCount() const override {
    return m_activeEntityCount;
  }
  
  // === Component Stats ===
  
  void SetComponentStats(std::vector<ComponentStats> const& stats) override {
    m_componentStats = stats;
  }
  
  std::vector<ComponentStats> const& GetComponentStats() const override {
    return m_componentStats;
  }
  
  void SetComponentCount(const char* typeName, int count, int enabledCount) override {
    if (!typeName) return;

    std::string typeNameStr(typeName);
    // Find existing entry
    for (auto& stat : m_componentStats) {
      if (stat.typeName == typeNameStr) {
        stat.count = count;
        if (enabledCount >= 0) {
          stat.enabledCount = enabledCount;
        } else {
          stat.enabledCount = count;
        }
        return;
      }
    }

    // Add new entry
    ComponentStats stat;
    stat.typeName = typeNameStr;
    stat.count = count;
    stat.enabledCount = enabledCount >= 0 ? enabledCount : count;
    m_componentStats.push_back(stat);
  }

  int GetComponentCount(const char* typeName) const override {
    if (!typeName) return 0;

    std::string typeNameStr(typeName);
    for (auto const& stat : m_componentStats) {
      if (stat.typeName == typeNameStr) {
        return stat.count;
      }
    }
    return 0;
  }
  
  // === Scene Bounds ===
  
  void SetSceneBounds(math::Vec3 const& min, math::Vec3 const& max) override {
    m_boundsMin = min;
    m_boundsMax = max;
  }
  
  void GetSceneBounds(math::Vec3& min, math::Vec3& max) const override {
    min = m_boundsMin;
    max = m_boundsMax;
  }

  math::Vec3 GetSceneCenter() const override {
    return math::Vec3{
      (m_boundsMin.x + m_boundsMax.x) * 0.5f,
      (m_boundsMin.y + m_boundsMax.y) * 0.5f,
      (m_boundsMin.z + m_boundsMax.z) * 0.5f
    };
  }

  math::Vec3 GetSceneExtents() const override {
    return math::Vec3{
      (m_boundsMax.x - m_boundsMin.x) * 0.5f,
      (m_boundsMax.y - m_boundsMin.y) * 0.5f,
      (m_boundsMax.z - m_boundsMin.z) * 0.5f
    };
  }
  
  // === Batch Update ===
  
  void UpdateStats(SceneStats const& stats) override {
    m_entityCount = stats.entityCount;
    m_rootEntityCount = stats.rootEntityCount;
    // Map component counts
    m_componentStats.clear();
    if (stats.meshCount > 0) {
      m_componentStats.push_back({"Mesh", stats.meshCount, stats.meshCount});
    }
    if (stats.lightCount > 0) {
      m_componentStats.push_back({"Light", stats.lightCount, stats.lightCount});
    }
    if (stats.cameraCount > 0) {
      m_componentStats.push_back({"Camera", stats.cameraCount, stats.cameraCount});
    }
    if (stats.audioSourceCount > 0) {
      m_componentStats.push_back({"AudioSource", stats.audioSourceCount, stats.audioSourceCount});
    }
  }
  
  SceneStats GetStats() const override {
    SceneStats stats;
    stats.entityCount = m_entityCount;
    stats.rootEntityCount = m_rootEntityCount;
    stats.meshCount = GetComponentCount("Mesh");
    stats.lightCount = GetComponentCount("Light");
    stats.cameraCount = GetComponentCount("Camera");
    stats.audioSourceCount = GetComponentCount("AudioSource");
    return stats;
  }
  
  // === Refresh ===
  
  void RequestRefresh() override {
    m_refreshRequested = true;
    if (m_onRefreshRequested) {
      m_onRefreshRequested();
    }
  }
  
  void SetOnRefreshRequested(std::function<void()> callback) override {
    m_onRefreshRequested = std::move(callback);
  }
  
  void SetAutoRefreshInterval(float seconds) override {
    m_autoRefreshInterval = std::max(0.0f, seconds);
  }
  
  float GetAutoRefreshInterval() const override {
    return m_autoRefreshInterval;
  }
  
  // === Visibility ===
  
  void SetVisible(bool visible) override {
    m_visible = visible;
  }
  
  bool IsVisible() const override {
    return m_visible;
  }

private:
  void CheckAutoRefresh() {
    if (m_autoRefreshInterval <= 0.0f) return;
    
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
      now - m_lastRefresh).count() / 1000.0f;
    
    if (elapsed >= m_autoRefreshInterval) {
      m_lastRefresh = now;
      RequestRefresh();
    }
  }
  
  void DrawEntityStats() {
    ImGui::Text("Entities");
    ImGui::Indent();
    
    ImGui::Text("Total: %d", m_entityCount);
    ImGui::Text("Root: %d", m_rootEntityCount);
    ImGui::Text("Active: %d", m_activeEntityCount);
    
    // Inactive entities
    int inactiveCount = m_entityCount - m_activeEntityCount;
    if (inactiveCount > 0) {
      ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.0f, 1.0f), "Inactive: %d", inactiveCount);
    }
    
    ImGui::Unindent();
  }
  
  void DrawComponentStats() {
    ImGui::Text("Components");
    
    if (m_componentStats.empty()) {
      ImGui::TextDisabled("No components");
      return;
    }
    
    // Sort by count (descending)
    std::vector<ComponentStats> sorted = m_componentStats;
    std::sort(sorted.begin(), sorted.end(), 
              [](ComponentStats const& a, ComponentStats const& b) {
                return a.count > b.count;
              });
    
    if (ImGui::BeginTable("ComponentTable", 3, 
                          ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
      ImGui::TableSetupColumn("Type");
      ImGui::TableSetupColumn("Count");
      ImGui::TableSetupColumn("Enabled");
      ImGui::TableHeadersRow();
      
      for (auto const& stat : sorted) {
        ImGui::TableNextRow();

        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%s", stat.typeName.empty() ? "Unknown" : stat.typeName.c_str());

        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d", stat.count);

        ImGui::TableSetColumnIndex(2);
        if (stat.enabledCount == stat.count) {
          ImGui::Text("%d", stat.enabledCount);
        } else {
          ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.0f, 1.0f), "%d", stat.enabledCount);
        }
      }
      
      ImGui::EndTable();
    }
  }
  
  void DrawSceneBounds() {
    ImGui::Text("Scene Bounds");

    math::Vec3 center = GetSceneCenter();
    math::Vec3 extents = GetSceneExtents();
    math::Vec3 size{
      m_boundsMax.x - m_boundsMin.x,
      m_boundsMax.y - m_boundsMin.y,
      m_boundsMax.z - m_boundsMin.z
    };
    
    ImGui::Text("Center: (%.2f, %.2f, %.2f)", center.x, center.y, center.z);
    ImGui::Text("Size: (%.2f, %.2f, %.2f)", size.x, size.y, size.z);
    ImGui::Text("Extents: (%.2f, %.2f, %.2f)", extents.x, extents.y, extents.z);
    
    // Draw a simple visualization
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 p = ImGui::GetCursorScreenPos();
    float boxSize = 60.0f;
    
    // Draw bounding box wireframe (simple 2D representation)
    drawList->AddRect(ImVec2(p.x, p.y), ImVec2(p.x + boxSize, p.y + boxSize), 
                      IM_COL32(100, 100, 255, 200));
    
    // Draw center point
    ImVec2 center2D(p.x + boxSize * 0.5f, p.y + boxSize * 0.5f);
    drawList->AddCircleFilled(center2D, 3.0f, IM_COL32(255, 255, 0, 255));
    
    ImGui::Dummy(ImVec2(boxSize, boxSize));
  }
  
  void DrawRefreshControls() {
    ImGui::Text("Refresh");
    
    // Auto-refresh interval
    ImGui::Text("Auto-refresh:");
    ImGui::SameLine();
    
    float interval = m_autoRefreshInterval;
    if (ImGui::SliderFloat("##Interval", &interval, 0.0f, 10.0f, "%.1f s")) {
      SetAutoRefreshInterval(interval);
    }
    
    if (ImGui::Button("Refresh Now")) {
      RequestRefresh();
    }
    
    if (m_refreshRequested) {
      ImGui::SameLine();
      ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Updated!");
      m_refreshRequested = false;
    }
  }
  
  // Entity stats
  int m_entityCount;
  int m_rootEntityCount;
  int m_activeEntityCount;
  
  // Component stats
  std::vector<ComponentStats> m_componentStats;
  
  // Scene bounds
  math::Vec3 m_boundsMin;
  math::Vec3 m_boundsMax;
  
  // Refresh
  float m_autoRefreshInterval;
  std::chrono::steady_clock::time_point m_lastRefresh;
  bool m_refreshRequested;
  std::function<void()> m_onRefreshRequested;
  
  // Visibility
  bool m_visible;
};

IStatisticsPanel* CreateStatisticsPanel() {
  return new StatisticsPanelImpl();
}

}  // namespace editor
}  // namespace te
