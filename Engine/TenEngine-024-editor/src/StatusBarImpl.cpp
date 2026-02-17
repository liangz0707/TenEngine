/**
 * @file StatusBarImpl.cpp
 * @brief Status bar implementation (024-Editor).
 */
#include <te/editor/StatusBar.h>
#include <imgui.h>
#include <cstring>
#include <chrono>

namespace te {
namespace editor {

class StatusBarImpl : public IStatusBar {
public:
  StatusBarImpl()
    : m_levelName("Untitled")
    , m_selectionCount(0)
    , m_fps(0.0f)
    , m_frameTime(0.0f)
    , m_memoryUsage(0)
    , m_visible(true)
    , m_statusMessage("")
    , m_statusMessageDuration(0.0f)
    , m_nextTaskId(1)
  {
    m_statusMessageExpire = std::chrono::steady_clock::now();
  }

  // === Drawing ===
  
  void OnDraw() override {
    if (!m_visible) return;
    
    // Check if status message expired
    auto now = std::chrono::steady_clock::now();
    if (m_statusMessage[0] != '\0' && now > m_statusMessageExpire) {
      m_statusMessage = "";
    }
    
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + viewport->Size.y - m_height));
    ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, m_height));
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar | 
                              ImGuiWindowFlags_NoSavedSettings |
                              ImGuiWindowFlags_NoDocking |
                              ImGuiWindowFlags_NoTitleBar;
    
    if (!ImGui::Begin("##StatusBar", nullptr, flags)) {
      ImGui::End();
      return;
    }
    
    float spacing = ImGui::GetStyle().ItemSpacing.x;
    float width = ImGui::GetContentRegionAvail().x;
    
    // Left section: Level name and selection
    float leftWidth = 0.0f;
    
    // Level name
    ImGui::Text("Level: %s", m_levelName);
    leftWidth += ImGui::CalcTextSize(ImGui::TextBuffer).x + spacing * 2;
    
    ImGui::SameLine();
    ImGui::Text("| Selection: %zu", m_selectionCount);
    leftWidth += ImGui::CalcTextSize(ImGui::TextBuffer).x + spacing * 2;
    
    // Status message in center if present
    if (m_statusMessage[0] != '\0') {
      ImGui::SameLine();
      ImGui::Text("| %s", m_statusMessage);
    }
    
    // Right section: Performance stats (right-aligned)
    ImGui::SameLine(width - 280.0f);
    
    // FPS
    ImGui::Text("FPS: %.1f", m_fps);
    ImGui::SameLine();
    
    // Frame time
    ImGui::Text("Frame: %.2fms", m_frameTime);
    ImGui::SameLine();
    
    // Memory usage
    if (m_memoryUsage > 0) {
      float memMB = static_cast<float>(m_memoryUsage) / (1024.0f * 1024.0f);
      ImGui::Text("Mem: %.1fMB", memMB);
    }
    
    // Background tasks
    if (!m_backgroundTasks.empty()) {
      ImGui::SameLine();
      ImGui::Text("|");
      ImGui::SameLine();
      
      for (auto& task : m_backgroundTasks) {
        if (task.active) {
          if (task.progress < 0.0f) {
            // Indeterminate progress
            ImGui::Text("%s...", task.name);
          } else {
            ImGui::Text("%s: %.0f%%", task.name, task.progress * 100.0f);
          }
          ImGui::SameLine();
        }
      }
    }
    
    ImGui::End();
  }
  
  // === Level Info ===
  
  void SetLevelName(const char* name) override {
    m_levelName = name ? name : "Untitled";
  }
  
  const char* GetLevelName() const override {
    return m_levelName;
  }
  
  // === Selection Info ===
  
  void SetSelectionCount(size_t count) override {
    m_selectionCount = count;
  }
  
  size_t GetSelectionCount() const override {
    return m_selectionCount;
  }
  
  // === Performance Stats ===
  
  void SetFPS(float fps) override {
    m_fps = fps;
  }
  
  void SetFrameTime(float ms) override {
    m_frameTime = ms;
  }
  
  void SetMemoryUsage(size_t bytes) override {
    m_memoryUsage = bytes;
  }
  
  void SetFrameStats(FrameStats const& stats) override {
    m_fps = (stats.frameTimeMs > 0.0f) ? 1000.0f / stats.frameTimeMs : 0.0f;
    m_frameTime = stats.frameTimeMs;
    m_memoryUsage = stats.memoryUsed;
  }
  
  // === Background Tasks ===
  
  int AddBackgroundTask(const char* name, float progress) override {
    BackgroundTask task;
    task.name = name;
    task.progress = progress;
    task.active = true;
    task.id = m_nextTaskId++;
    
    m_backgroundTasks.push_back(task);
    return task.id;
  }
  
  void UpdateBackgroundTask(int taskId, float progress) override {
    for (auto& task : m_backgroundTasks) {
      if (task.id == taskId) {
        task.progress = progress;
        return;
      }
    }
  }
  
  void RemoveBackgroundTask(int taskId) override {
    m_backgroundTasks.erase(
      std::remove_if(m_backgroundTasks.begin(), m_backgroundTasks.end(),
        [taskId](BackgroundTask const& t) { return t.id == taskId; }),
      m_backgroundTasks.end());
  }
  
  std::vector<BackgroundTask> const& GetBackgroundTasks() const override {
    return m_backgroundTasks;
  }
  
  // === Status Messages ===
  
  void SetStatusMessage(const char* message, float duration) override {
    m_statusMessage = message ? message : "";
    
    if (duration <= 0.0f) {
      duration = 5.0f;  // Default 5 seconds
    }
    
    m_statusMessageDuration = duration;
    m_statusMessageExpire = std::chrono::steady_clock::now() + 
                             std::chrono::milliseconds(static_cast<int>(duration * 1000));
  }
  
  void ClearStatusMessage() override {
    m_statusMessage = "";
  }
  
  // === Visibility ===
  
  void SetVisible(bool visible) override {
    m_visible = visible;
  }
  
  bool IsVisible() const override {
    return m_visible;
  }

private:
  static constexpr float m_height = 24.0f;
  
  // Level info
  const char* m_levelName;
  size_t m_selectionCount;
  
  // Performance stats
  float m_fps;
  float m_frameTime;
  size_t m_memoryUsage;
  
  // Status message
  const char* m_statusMessage;
  float m_statusMessageDuration;
  std::chrono::steady_clock::time_point m_statusMessageExpire;
  
  // Background tasks
  std::vector<BackgroundTask> m_backgroundTasks;
  int m_nextTaskId;
  
  // Visibility
  bool m_visible;
};

IStatusBar* CreateStatusBar() {
  return new StatusBarImpl();
}

}  // namespace editor
}  // namespace te
