/**
 * @file ViewportStats.cpp
 * @brief Viewport statistics overlay implementation (024-Editor).
 */
#include <te/editor/ViewportStats.h>
#include <imgui.h>
#include <chrono>
#include <cstring>

namespace te {
namespace editor {

class ViewportStatsImpl : public IViewportStats {
public:
  ViewportStatsImpl()
    : m_visible(true)
    , m_frameStartTime(0.0)
  {
    std::memset(&m_stats, 0, sizeof(m_stats));
  }

  // === Data Collection ===
  
  void BeginFrame() override {
    m_frameStartTime = GetCurrentTime();
    m_stats.frameNumber++;
    
    // Reset per-frame stats
    m_stats.render.drawCalls = 0;
    m_stats.render.triangles = 0;
    m_stats.render.vertices = 0;
  }
  
  void EndFrame() override {
    double endTime = GetCurrentTime();
    float frameTime = static_cast<float>((endTime - m_frameStartTime) * 1000.0);
    
    m_stats.timing.totalFrameTimeMs = frameTime;
    m_stats.timing.fps = frameTime > 0.0f ? (1000.0f / frameTime) : 0.0f;
    
    // Update history
    int idx = m_stats.timing.historyIndex % 60;
    m_stats.timing.cpuTimeHistory[idx] = m_stats.timing.cpuTimeMs;
    m_stats.timing.gpuTimeHistory[idx] = m_stats.timing.gpuTimeMs;
    m_stats.timing.historyIndex++;
    
    m_stats.totalTime += frameTime / 1000.0;
  }
  
  void UpdateTiming(float cpuTimeMs, float gpuTimeMs) override {
    m_stats.timing.cpuTimeMs = cpuTimeMs;
    m_stats.timing.gpuTimeMs = gpuTimeMs;
  }
  
  void UpdateRenderStats(RenderStats const& stats) override {
    m_stats.render = stats;
  }
  
  void UpdateSceneStats(SceneStats const& stats) override {
    m_stats.scene = stats;
  }
  
  void UpdatePhysicsStats(PhysicsStats const& stats) override {
    m_stats.physics = stats;
  }
  
  void AddDrawCall(uint32_t triangles, uint32_t vertices) override {
    m_stats.render.drawCalls++;
    m_stats.render.triangles += triangles;
    m_stats.render.vertices += vertices;
  }
  
  void ResetStats() override {
    std::memset(&m_stats, 0, sizeof(m_stats));
    m_stats.timing.historyIndex = 0;
  }
  
  // === Data Access ===
  
  ViewportStatistics const& GetStats() const override {
    return m_stats;
  }
  
  float GetFPS() const override {
    return m_stats.timing.fps;
  }
  
  float GetFrameTimeMs() const override {
    return m_stats.timing.totalFrameTimeMs;
  }
  
  uint32_t GetDrawCalls() const override {
    return m_stats.render.drawCalls;
  }
  
  // === Display Settings ===
  
  void SetDisplaySettings(StatsDisplaySettings const& settings) override {
    m_settings = settings;
  }
  
  StatsDisplaySettings const& GetDisplaySettings() const override {
    return m_settings;
  }
  
  void SetVisible(bool visible) override {
    m_visible = visible;
  }
  
  bool IsVisible() const override {
    return m_visible;
  }
  
  void SetPosition(StatsDisplaySettings::Position position) override {
    m_settings.position = position;
  }
  
  // === Rendering ===
  
  void OnDraw() override {
    if (!m_visible) return;
    
    ImGui::SetNextWindowBgAlpha(m_settings.opacity);
    
    // Calculate position
    ImVec2 windowPos;
    ImVec2 windowPivot;
    
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 viewportSize = viewport->WorkSize;
    
    switch (m_settings.position) {
      case StatsDisplaySettings::Position::TopLeft:
        windowPos = ImVec2(10, 10);
        windowPivot = ImVec2(0, 0);
        break;
      case StatsDisplaySettings::Position::TopRight:
        windowPos = ImVec2(viewportSize.x - 10, 10);
        windowPivot = ImVec2(1, 0);
        break;
      case StatsDisplaySettings::Position::BottomLeft:
        windowPos = ImVec2(10, viewportSize.y - 10);
        windowPivot = ImVec2(0, 1);
        break;
      case StatsDisplaySettings::Position::BottomRight:
        windowPos = ImVec2(viewportSize.x - 10, viewportSize.y - 10);
        windowPivot = ImVec2(1, 1);
        break;
    }
    
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, windowPivot);
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | 
                              ImGuiWindowFlags_NoResize |
                              ImGuiWindowFlags_NoCollapse |
                              ImGuiWindowFlags_AlwaysAutoResize |
                              ImGuiWindowFlags_NoFocusOnAppearing;
    if (!m_settings.showBackground) {
      flags |= ImGuiWindowFlags_NoBackground;
    }
    
    if (ImGui::Begin("Viewport Stats", nullptr, flags)) {
      DrawStatsContent();
    }
    ImGui::End();
  }
  
  void DrawTimingGraph() override {
    if (!m_settings.showGraphs) return;
    
    ImGui::Text("Frame Time Graph:");
    
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    ImVec2 canvasSize = ImVec2(200, 60);
    
    // Draw background
    drawList->AddRectFilled(canvasPos, 
                            ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y),
                            IM_COL32(30, 30, 30, 200));
    
    // Draw grid lines
    for (int i = 0; i <= 4; i++) {
      float y = canvasPos.y + (canvasSize.y * i / 4.0f);
      drawList->AddLine(ImVec2(canvasPos.x, y),
                        ImVec2(canvasPos.x + canvasSize.x, y),
                        IM_COL32(60, 60, 60, 200));
    }
    
    // Draw CPU timing (green)
    float maxTime = 33.33f;  // 30 FPS threshold
    for (int i = 0; i < 59; i++) {
      int idx1 = (m_stats.timing.historyIndex + i) % 60;
      int idx2 = (m_stats.timing.historyIndex + i + 1) % 60;
      
      float x1 = canvasPos.x + (i / 60.0f) * canvasSize.x;
      float x2 = canvasPos.x + ((i + 1) / 60.0f) * canvasSize.x;
      
      float y1 = canvasPos.y + canvasSize.y - 
                 (m_stats.timing.cpuTimeHistory[idx1] / maxTime) * canvasSize.y;
      float y2 = canvasPos.y + canvasSize.y - 
                 (m_stats.timing.cpuTimeHistory[idx2] / maxTime) * canvasSize.y;
      
      y1 = std::max(canvasPos.y, std::min(canvasPos.y + canvasSize.y, y1));
      y2 = std::max(canvasPos.y, std::min(canvasPos.y + canvasSize.y, y2));
      
      drawList->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), 
                        IM_COL32(0, 255, 0, 255));
    }
    
    ImGui::Dummy(canvasSize);
  }

private:
  void DrawStatsContent() {
    // FPS and Frame Time
    if (m_settings.showFPS) {
      ImVec4 fpsColor = m_stats.timing.fps >= 60 ? ImVec4(0, 1, 0, 1) :
                        m_stats.timing.fps >= 30 ? ImVec4(1, 1, 0, 1) :
                        ImVec4(1, 0, 0, 1);
      
      ImGui::TextColored(fpsColor, "FPS: %.1f", m_stats.timing.fps);
    }
    
    if (m_settings.showFrameTime) {
      ImGui::Text("Frame: %.2f ms", m_stats.timing.totalFrameTimeMs);
      ImGui::Text("  CPU: %.2f ms", m_stats.timing.cpuTimeMs);
      ImGui::Text("  GPU: %.2f ms", m_stats.timing.gpuTimeMs);
    }
    
    ImGui::Separator();
    
    // Render Stats
    if (m_settings.showDrawCalls) {
      ImGui::Text("Draw Calls: %u", m_stats.render.drawCalls);
    }
    
    if (m_settings.showTriangleCount) {
      ImGui::Text("Triangles: %u", m_stats.render.triangles);
    }
    
    if (m_settings.showVertexCount) {
      ImGui::Text("Vertices: %u", m_stats.render.vertices);
    }
    
    if (m_settings.showMemoryUsage) {
      float vramMB = m_stats.render.vramUsed / (1024.0f * 1024.0f);
      float budgetMB = m_stats.render.vramBudget / (1024.0f * 1024.0f);
      
      if (budgetMB > 0) {
        ImGui::Text("VRAM: %.1f / %.1f MB", vramMB, budgetMB);
        
        // Progress bar
        float fraction = vramMB / budgetMB;
        ImVec4 barColor = fraction < 0.5f ? ImVec4(0, 1, 0, 1) :
                          fraction < 0.8f ? ImVec4(1, 1, 0, 1) :
                          ImVec4(1, 0, 0, 1);
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, barColor);
        ImGui::ProgressBar(fraction, ImVec2(-1, 0), "");
        ImGui::PopStyleColor();
      } else {
        ImGui::Text("VRAM: %.1f MB", vramMB);
      }
    }
    
    ImGui::Separator();
    
    // Scene Stats
    if (m_settings.showEntityCount) {
      ImGui::Text("Entities: %u / %u visible", 
                  m_stats.scene.visibleEntities, m_stats.scene.totalEntities);
    }
    
    if (m_settings.showLightCount) {
      ImGui::Text("Lights: %u / %u", 
                  m_stats.scene.visibleLights, m_stats.scene.totalLights);
    }
    
    // Physics Stats
    if (m_settings.showPhysicsStats) {
      ImGui::Separator();
      ImGui::Text("Physics: %.2f ms", m_stats.physics.physicsTimeMs);
      ImGui::Text("  Bodies: %u active, %u sleeping",
                  m_stats.physics.activeBodies, m_stats.physics.sleepingBodies);
      ImGui::Text("  Contacts: %u", m_stats.physics.activeContacts);
    }
    
    // Graph
    if (m_settings.showGraphs) {
      ImGui::Separator();
      DrawTimingGraph();
    }
    
    // Right-click menu
    if (ImGui::BeginPopupContextItem("StatsSettings")) {
      ImGui::Checkbox("Show FPS", &m_settings.showFPS);
      ImGui::Checkbox("Show Frame Time", &m_settings.showFrameTime);
      ImGui::Checkbox("Show Draw Calls", &m_settings.showDrawCalls);
      ImGui::Checkbox("Show Triangle Count", &m_settings.showTriangleCount);
      ImGui::Checkbox("Show Vertex Count", &m_settings.showVertexCount);
      ImGui::Checkbox("Show Memory", &m_settings.showMemoryUsage);
      ImGui::Checkbox("Show Entity Count", &m_settings.showEntityCount);
      ImGui::Checkbox("Show Light Count", &m_settings.showLightCount);
      ImGui::Checkbox("Show Physics", &m_settings.showPhysicsStats);
      ImGui::Checkbox("Show Graphs", &m_settings.showGraphs);
      ImGui::Separator();
      ImGui::SliderFloat("Opacity", &m_settings.opacity, 0.0f, 1.0f);
      ImGui::EndPopup();
    }
  }
  
  double GetCurrentTime() const {
    auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double>(now.time_since_epoch()).count();
  }

private:
  ViewportStatistics m_stats;
  StatsDisplaySettings m_settings;
  bool m_visible;
  double m_frameStartTime;
};

IViewportStats* CreateViewportStats() {
  return new ViewportStatsImpl();
}

}  // namespace editor
}  // namespace te
