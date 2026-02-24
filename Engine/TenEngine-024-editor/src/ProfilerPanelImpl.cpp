/**
 * @file ProfilerPanelImpl.cpp
 * @brief Performance profiler panel implementation (024-Editor).
 */
#include <te/editor/ProfilerPanel.h>
#include <imgui.h>
#include <algorithm>
#include <cmath>
#include <cstring>

namespace te {
namespace editor {

class ProfilerPanelImpl : public IProfilerPanel {
public:
  ProfilerPanelImpl()
    : m_historyLength(120)
    , m_graphHeight(100.0f)
    , m_showTimeline(true)
    , m_warningThreshold(16.67f)  // ~60 FPS
    , m_paused(false)
    , m_visible(true)
  {
    m_frameHistory.resize(m_historyLength);
    m_currentStats = {};
  }

  // === Drawing ===
  
  void OnDraw() override {
    if (!m_visible) return;
    
    if (!ImGui::Begin("Profiler", &m_visible)) {
      ImGui::End();
      return;
    }
    
    DrawStatsOverview();
    ImGui::Separator();
    
    DrawFrameTimeGraph();
    ImGui::Separator();
    
    if (m_showTimeline) {
      DrawTimeline();
      ImGui::Separator();
    }
    
    DrawDetailedStats();
    
    ImGui::End();
  }
  
  // === Frame Stats ===
  
  void UpdateFrameStats(FrameStats const& stats) override {
    if (m_paused) return;
    
    m_currentStats = stats;
    
    // Update history
    m_frameHistory[m_historyIndex].frameTimeMs = stats.frameTimeMs;
    m_frameHistory[m_historyIndex].cpuTimeMs = stats.cpuTimeMs;
    m_frameHistory[m_historyIndex].gpuTimeMs = stats.gpuTimeMs;
    m_frameHistory[m_historyIndex].drawCalls = stats.drawCalls;
    m_frameHistory[m_historyIndex].triangles = stats.triangles;
    
    m_historyIndex = (m_historyIndex + 1) % m_historyLength;
  }
  
  FrameStats GetCurrentStats() const override {
    return m_currentStats;
  }
  
  float GetAverageFrameTime() const override {
    float sum = 0.0f;
    int count = 0;
    
    for (auto const& entry : m_frameHistory) {
      if (entry.frameTimeMs > 0.0f) {
        sum += entry.frameTimeMs;
        count++;
      }
    }
    
    return count > 0 ? sum / count : 0.0f;
  }
  
  void GetFrameTimeRange(float& min, float& max) const override {
    min = 1e10f;
    max = 0.0f;
    
    for (auto const& entry : m_frameHistory) {
      if (entry.frameTimeMs > 0.0f) {
        min = std::min(min, entry.frameTimeMs);
        max = std::max(max, entry.frameTimeMs);
      }
    }
    
    if (min > max) {
      min = max = 0.0f;
    }
  }
  
  // === Profiling Scopes ===
  
  void BeginScope(const char* name, uint32_t color) override {
    if (m_paused) return;
    
    ProfilerScope scope;
    scope.name = name;
    scope.color = color;
    scope.depth = static_cast<int>(m_scopeStack.size());
    scope.startTimeMs = GetCurrentTimeMs();
    
    m_scopeStack.push_back(m_scopes.size());
    m_scopes.push_back(scope);
  }
  
  void EndScope() override {
    if (m_paused || m_scopeStack.empty()) return;
    
    size_t scopeIndex = m_scopeStack.back();
    m_scopeStack.pop_back();
    
    m_scopes[scopeIndex].endTimeMs = GetCurrentTimeMs();
    m_scopes[scopeIndex].durationMs = 
      m_scopes[scopeIndex].endTimeMs - m_scopes[scopeIndex].startTimeMs;
  }
  
  void ClearScopes() override {
    m_scopes.clear();
    m_scopeStack.clear();
  }
  
  std::vector<ProfilerScope> const& GetScopes() const override {
    return m_scopes;
  }
  
  // === History ===
  
  void SetHistoryLength(int frames) override {
    m_historyLength = std::max(10, frames);
    m_frameHistory.resize(m_historyLength);
    m_historyIndex = m_historyIndex % m_historyLength;
  }
  
  int GetHistoryLength() const override {
    return m_historyLength;
  }
  
  std::vector<FrameTimingEntry> const& GetHistory() const override {
    return m_frameHistory;
  }
  
  // === Display Options ===
  
  void SetGraphHeight(float height) override {
    m_graphHeight = std::max(50.0f, std::min(300.0f, height));
  }
  
  float GetGraphHeight() const override {
    return m_graphHeight;
  }
  
  void SetShowTimeline(bool show) override {
    m_showTimeline = show;
  }
  
  bool IsShowTimelineEnabled() const override {
    return m_showTimeline;
  }
  
  void SetWarningThreshold(float ms) override {
    m_warningThreshold = std::max(1.0f, ms);
  }
  
  float GetWarningThreshold() const override {
    return m_warningThreshold;
  }
  
  // === Pause/Resume ===
  
  void Pause() override {
    m_paused = true;
  }
  
  void Resume() override {
    m_paused = false;
  }
  
  bool IsPaused() const override {
    return m_paused;
  }
  
  // === Visibility ===
  
  void SetVisible(bool visible) override {
    m_visible = visible;
  }
  
  bool IsVisible() const override {
    return m_visible;
  }

private:
  void DrawStatsOverview() {
    ImGui::Text("Frame: %.2f ms (%.1f FPS)", 
                m_currentStats.frameTimeMs,
                m_currentStats.frameTimeMs > 0 ? 1000.0f / m_currentStats.frameTimeMs : 0.0f);
    ImGui::SameLine();
    
    // Color indicator
    ImVec4 color;
    if (m_currentStats.frameTimeMs <= m_warningThreshold) {
      color = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);  // Green
    } else if (m_currentStats.frameTimeMs <= m_warningThreshold * 2.0f) {
      color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);  // Yellow
    } else {
      color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);  // Red
    }
    ImGui::ColorButton("##status", color, ImGuiColorEditFlags_NoTooltip, ImVec2(16, 16));
    
    ImGui::Text("CPU: %.2f ms  GPU: %.2f ms", 
                m_currentStats.cpuTimeMs, m_currentStats.gpuTimeMs);
    ImGui::Text("Draw Calls: %d  Triangles: %d", 
                m_currentStats.drawCalls, m_currentStats.triangles);
    
    // Pause button
    if (ImGui::Button(m_paused ? "Resume" : "Pause")) {
      m_paused = !m_paused;
    }
  }
  
  void DrawFrameTimeGraph() {
    ImGui::Text("Frame Time History");
    
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    ImVec2 canvasSize = ImGui::GetContentRegionAvail();
    canvasSize.y = m_graphHeight;
    
    // Background
    drawList->AddRectFilled(canvasPos, 
                            ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y),
                            IM_COL32(30, 30, 30, 255));
    
    // Find max value for scaling
    float maxTime = 33.33f;  // Default to 30 FPS scale
    GetFrameTimeRange(maxTime, maxTime);
    maxTime = std::max(maxTime, m_warningThreshold * 1.5f);
    
    // Draw warning threshold line
    float warningY = canvasPos.y + canvasSize.y - (m_warningThreshold / maxTime) * canvasSize.y;
    drawList->AddLine(ImVec2(canvasPos.x, warningY), 
                      ImVec2(canvasPos.x + canvasSize.x, warningY),
                      IM_COL32(255, 255, 0, 128));
    
    // Draw frame time graph
    float stepX = canvasSize.x / static_cast<float>(m_historyLength);
    
    for (int i = 0; i < m_historyLength - 1; ++i) {
      int idx0 = (m_historyIndex + i) % m_historyLength;
      int idx1 = (m_historyIndex + i + 1) % m_historyLength;
      
      float x0 = canvasPos.x + i * stepX;
      float x1 = canvasPos.x + (i + 1) * stepX;
      
      float y0 = canvasPos.y + canvasSize.y - 
                 (m_frameHistory[idx0].frameTimeMs / maxTime) * canvasSize.y;
      float y1 = canvasPos.y + canvasSize.y - 
                 (m_frameHistory[idx1].frameTimeMs / maxTime) * canvasSize.y;
      
      // Clamp Y values
      y0 = std::max(canvasPos.y, std::min(canvasPos.y + canvasSize.y, y0));
      y1 = std::max(canvasPos.y, std::min(canvasPos.y + canvasSize.y, y1));
      
      drawList->AddLine(ImVec2(x0, y0), ImVec2(x1, y1), IM_COL32(0, 200, 255, 255), 2.0f);
    }
    
    ImGui::Dummy(canvasSize);
  }
  
  void DrawTimeline() {
    ImGui::Text("Frame Timeline");
    
    if (m_scopes.empty()) {
      ImGui::TextDisabled("No profiling scopes recorded");
      return;
    }
    
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    ImVec2 canvasSize = ImGui::GetContentRegionAvail();
    canvasSize.y = std::min(150.0f, canvasSize.y);
    
    // Find time range
    float minTime = m_scopes[0].startTimeMs;
    float maxTime = m_scopes[0].endTimeMs;
    
    for (auto const& scope : m_scopes) {
      minTime = std::min(minTime, scope.startTimeMs);
      maxTime = std::max(maxTime, scope.endTimeMs);
    }
    
    float timeRange = maxTime - minTime;
    if (timeRange < 0.001f) timeRange = 1.0f;
    
    // Draw scopes
    float rowHeight = 20.0f;
    int maxDepth = 0;
    for (auto const& scope : m_scopes) {
      maxDepth = std::max(maxDepth, scope.depth + 1);
    }
    
    for (auto const& scope : m_scopes) {
      float x0 = canvasPos.x + ((scope.startTimeMs - minTime) / timeRange) * canvasSize.x;
      float x1 = canvasPos.x + ((scope.endTimeMs - minTime) / timeRange) * canvasSize.x;
      float y = canvasPos.y + scope.depth * rowHeight;
      
      // Ensure minimum width
      if (x1 - x0 < 2.0f) x1 = x0 + 2.0f;
      
      uint32_t color = scope.color;
      drawList->AddRectFilled(ImVec2(x0, y), ImVec2(x1, y + rowHeight - 2), color);
      
      // Label if wide enough
      if (x1 - x0 > 30.0f && !scope.name.empty()) {
        drawList->AddText(ImVec2(x0 + 2, y + 2), IM_COL32(0, 0, 0, 255), scope.name.c_str());
      }
    }
    
    ImGui::Dummy(canvasSize);
  }
  
  void DrawDetailedStats() {
    ImGui::Text("Average: %.2f ms", GetAverageFrameTime());
    
    float minTime, maxTime;
    GetFrameTimeRange(minTime, maxTime);
    ImGui::Text("Range: %.2f - %.2f ms", minTime, maxTime);
  }
  
  static float GetCurrentTimeMs() {
    // Placeholder - in real implementation, use high-resolution timer
    static float time = 0.0f;
    time += 0.1f;  // Simulated time
    return time;
  }
  
  // Current stats
  FrameStats m_currentStats;
  
  // History
  std::vector<FrameTimingEntry> m_frameHistory;
  int m_historyLength;
  int m_historyIndex = 0;
  
  // Profiling scopes
  std::vector<ProfilerScope> m_scopes;
  std::vector<size_t> m_scopeStack;
  
  // Display options
  float m_graphHeight;
  bool m_showTimeline;
  float m_warningThreshold;
  
  // State
  bool m_paused;
  bool m_visible;
};

IProfilerPanel* CreateProfilerPanel() {
  return new ProfilerPanelImpl();
}

}  // namespace editor
}  // namespace te
