/**
 * @file ConsolePanelImpl.cpp
 * @brief Console log panel implementation (024-Editor).
 */
#include <te/editor/ConsolePanel.h>
#include <imgui.h>
#include <chrono>
#include <ctime>
#include <algorithm>
#include <cstring>

namespace te {
namespace editor {

class ConsolePanelImpl : public IConsolePanel {
public:
  ConsolePanelImpl()
    : m_searchFilter("")
    , m_minLogLevel(LogLevel::Info)
    , m_collapseEnabled(false)
    , m_autoScroll(true)
    , m_scrollToBottom(false)
    , m_visible(true)
    , m_commandCallback(nullptr)
  {
    // Enable all log levels by default
    m_levelEnabled[0] = true;  // Info
    m_levelEnabled[1] = true;  // Warning
    m_levelEnabled[2] = true;  // Error
    m_levelEnabled[3] = true;  // Fatal
    
    m_inputBuffer[0] = '\0';
  }

  // === Drawing ===
  
  void OnDraw() override {
    if (!m_visible) return;
    
    if (!ImGui::Begin("Console", &m_visible)) {
      ImGui::End();
      return;
    }
    
    DrawToolbar();
    ImGui::Separator();
    DrawLogArea();
    ImGui::Separator();
    DrawCommandInput();
    
    ImGui::End();
  }
  
  // === Log Operations ===
  
  void Log(const char* message, LogLevel level, const char* category) override {
    ConsoleEntry entry;
    entry.message = message ? message : "";
    entry.level = level;
    entry.timestamp = GetCurrentTimestamp();
    entry.category = category ? category : "";
    entry.frameCount = 0;  // TODO: Get actual frame count
    entry.count = 1;
    
    // Check for collapse
    if (m_collapseEnabled && !m_entries.empty()) {
      ConsoleEntry& last = m_entries.back();
      if (last.message == entry.message && last.level == entry.level) {
        last.count++;
        return;
      }
    }
    
    m_entries.push_back(entry);
    
    // Update counts
    if (static_cast<size_t>(level) < 4) {
      m_levelCounts[static_cast<size_t>(level)]++;
    }
    
    if (m_autoScroll) {
      m_scrollToBottom = true;
    }
  }
  
  void LogInfo(const char* message) override {
    Log(message, LogLevel::Info, nullptr);
  }
  
  void LogWarning(const char* message) override {
    Log(message, LogLevel::Warning, nullptr);
  }
  
  void LogError(const char* message) override {
    Log(message, LogLevel::Error, nullptr);
  }
  
  void Clear() override {
    m_entries.clear();
    m_levelCounts[0] = 0;
    m_levelCounts[1] = 0;
    m_levelCounts[2] = 0;
    m_levelCounts[3] = 0;
  }
  
  std::vector<ConsoleEntry> const& GetEntries() const override {
    return m_entries;
  }
  
  size_t GetEntryCount() const override {
    return m_entries.size();
  }
  
  // === Filtering ===
  
  void SetSearchFilter(const char* filter) override {
    m_searchFilter = filter ? filter : "";
  }
  
  const char* GetSearchFilter() const override {
    return m_searchFilter.c_str();
  }
  
  void SetLogLevelFilter(LogLevel minLevel) override {
    m_minLogLevel = minLevel;
  }
  
  LogLevel GetLogLevelFilter() const override {
    return m_minLogLevel;
  }
  
  void SetLogLevelEnabled(LogLevel level, bool enabled) override {
    size_t idx = static_cast<size_t>(level);
    if (idx < 4) {
      m_levelEnabled[idx] = enabled;
    }
  }
  
  bool IsLogLevelEnabled(LogLevel level) const override {
    size_t idx = static_cast<size_t>(level);
    return idx < 4 ? m_levelEnabled[idx] : false;
  }
  
  // === Collapse ===
  
  void SetCollapseEnabled(bool enabled) override {
    m_collapseEnabled = enabled;
  }
  
  bool IsCollapseEnabled() const override {
    return m_collapseEnabled;
  }
  
  // === Scroll ===
  
  void ScrollToBottom() override {
    m_scrollToBottom = true;
  }
  
  void SetAutoScroll(bool enabled) override {
    m_autoScroll = enabled;
  }
  
  bool IsAutoScrollEnabled() const override {
    return m_autoScroll;
  }
  
  // === Command Input ===
  
  void SetCommandCallback(ConsoleCommandCallback callback) override {
    m_commandCallback = std::move(callback);
  }
  
  std::vector<std::string> const& GetCommandHistory() const override {
    return m_commandHistory;
  }
  
  void ClearCommandHistory() override {
    m_commandHistory.clear();
  }
  
  // === Statistics ===
  
  size_t GetCountByLevel(LogLevel level) const override {
    size_t idx = static_cast<size_t>(level);
    return idx < 4 ? m_levelCounts[idx] : 0;
  }
  
  // === Visibility ===
  
  void SetVisible(bool visible) override {
    m_visible = visible;
  }
  
  bool IsVisible() const override {
    return m_visible;
  }

private:
  void DrawToolbar() {
    // Clear button
    if (ImGui::Button("Clear")) {
      Clear();
    }
    ImGui::SameLine();
    
    // Collapse toggle
    if (ImGui::Checkbox("Collapse", &m_collapseEnabled)) {
      // Re-process entries for collapse
    }
    ImGui::SameLine();
    
    // Log level filters
    ImGui::Text("Show:");
    ImGui::SameLine();
    
    struct LevelInfo { const char* name; ImVec4 color; };
    LevelInfo levels[] = {
      {"Info", ImVec4(0.8f, 0.8f, 0.8f, 1.0f)},
      {"Warning", ImVec4(1.0f, 1.0f, 0.0f, 1.0f)},
      {"Error", ImVec4(1.0f, 0.3f, 0.3f, 1.0f)},
      {"Fatal", ImVec4(1.0f, 0.0f, 0.0f, 1.0f)}
    };
    
    for (int i = 0; i < 4; ++i) {
      ImGui::PushStyleColor(ImGuiCol_Text, levels[i].color);
      if (ImGui::Checkbox(levels[i].name, &m_levelEnabled[i])) {
        // Level filter changed
      }
      ImGui::PopStyleColor();
      ImGui::SameLine();
    }
    
    // Search filter
    ImGui::SameLine();
    ImGui::Text("Search:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(150);
    ImGui::InputText("##SearchFilter", m_searchBuffer, sizeof(m_searchBuffer));
    m_searchFilter = m_searchBuffer;
    
    // Entry counts
    ImGui::SameLine(ImGui::GetWindowWidth() - 200);
    ImGui::Text("Info:%zu Warn:%zu Err:%zu", 
                m_levelCounts[0], m_levelCounts[1], m_levelCounts[2]);
  }
  
  void DrawLogArea() {
    ImGui::BeginChild("LogArea", ImVec2(0, -30), true);
    
    ImGuiListClipper clipper;
    int visibleCount = 0;
    
    // Count visible entries for clipper
    for (auto const& entry : m_entries) {
      if (ShouldShowEntry(entry)) {
        visibleCount++;
      }
    }
    
    clipper.Begin(visibleCount);
    
    int visibleIndex = 0;
    for (size_t i = 0; i < m_entries.size() && clipper.Step(); ++i) {
      auto const& entry = m_entries[i];
      if (!ShouldShowEntry(entry)) continue;
      
      if (visibleIndex >= clipper.DisplayStart && visibleIndex < clipper.DisplayEnd) {
        DrawLogEntry(entry);
      }
      visibleIndex++;
    }
    
    clipper.End();
    
    if (m_scrollToBottom) {
      ImGui::SetScrollHereY(1.0f);
      m_scrollToBottom = false;
    }
    
    ImGui::EndChild();
  }
  
  void DrawLogEntry(ConsoleEntry const& entry) {
    // Get color based on level
    ImVec4 color;
    switch (entry.level) {
      case LogLevel::Info:    color = ImVec4(0.9f, 0.9f, 0.9f, 1.0f); break;
      case LogLevel::Warning: color = ImVec4(1.0f, 1.0f, 0.3f, 1.0f); break;
      case LogLevel::Error:   color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f); break;
      case LogLevel::Fatal:   color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); break;
      default:                color = ImVec4(0.9f, 0.9f, 0.9f, 1.0f); break;
    }
    
    ImGui::PushStyleColor(ImGuiCol_Text, color);
    
    // Format timestamp
    char timeBuf[32];
    FormatTimestamp(entry.timestamp, timeBuf, sizeof(timeBuf));
    
    // Draw entry
    if (entry.count > 1) {
      ImGui::Text("[%s] [%s] %s (x%d)", 
                  timeBuf, 
                  GetLevelString(entry.level),
                  entry.message.c_str(),
                  entry.count);
    } else {
      ImGui::Text("[%s] [%s] %s", 
                  timeBuf, 
                  GetLevelString(entry.level),
                  entry.message.c_str());
    }
    
    ImGui::PopStyleColor();
  }
  
  void DrawCommandInput() {
    ImGui::Text("> ");
    ImGui::SameLine();
    
    ImGui::SetNextItemWidth(-50);
    bool submitted = ImGui::InputText("##CommandInput", m_inputBuffer, sizeof(m_inputBuffer), 
                                       ImGuiInputTextFlags_EnterReturnsTrue);
    
    ImGui::SameLine();
    if (ImGui::Button("Submit") || submitted) {
      if (m_inputBuffer[0] != '\0') {
        // Add to history
        m_commandHistory.push_back(m_inputBuffer);
        
        // Execute command
        if (m_commandCallback) {
          m_commandCallback(m_inputBuffer);
        }
        
        // Clear input
        m_inputBuffer[0] = '\0';
      }
    }
  }
  
  bool ShouldShowEntry(ConsoleEntry const& entry) const {
    // Check level filter
    if (!m_levelEnabled[static_cast<size_t>(entry.level)]) {
      return false;
    }
    
    // Check search filter
    if (!m_searchFilter.empty()) {
      if (entry.message.find(m_searchFilter) == std::string::npos) {
        return false;
      }
    }
    
    return true;
  }
  
  static uint64_t GetCurrentTimestamp() {
    return static_cast<uint64_t>(
      std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
      ).count()
    );
  }
  
  static void FormatTimestamp(uint64_t timestamp, char* buffer, size_t size) {
    time_t time = static_cast<time_t>(timestamp / 1000);
    int ms = static_cast<int>(timestamp % 1000);
    
    struct tm* tm_info = localtime(&time);
    if (tm_info) {
      snprintf(buffer, size, "%02d:%02d:%02d.%03d",
               tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec, ms);
    } else {
      buffer[0] = '\0';
    }
  }
  
  static const char* GetLevelString(LogLevel level) {
    switch (level) {
      case LogLevel::Info:    return "Info";
      case LogLevel::Warning: return "Warn";
      case LogLevel::Error:   return "Error";
      case LogLevel::Fatal:   return "Fatal";
      default:                return "Unknown";
    }
  }
  
  // Log entries
  std::vector<ConsoleEntry> m_entries;
  size_t m_levelCounts[4] = {0, 0, 0, 0};
  
  // Filtering
  std::string m_searchFilter;
  char m_searchBuffer[256] = "";
  LogLevel m_minLogLevel;
  bool m_levelEnabled[4];
  
  // Collapse
  bool m_collapseEnabled;
  
  // Scroll
  bool m_autoScroll;
  bool m_scrollToBottom;
  
  // Command input
  char m_inputBuffer[512] = "";
  std::vector<std::string> m_commandHistory;
  ConsoleCommandCallback m_commandCallback;
  
  // Visibility
  bool m_visible;
};

IConsolePanel* CreateConsolePanel() {
  return new ConsolePanelImpl();
}

}  // namespace editor
}  // namespace te
