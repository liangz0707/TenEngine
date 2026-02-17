/**
 * @file ConsolePanel.h
 * @brief Console log panel interface (ABI IConsolePanel).
 */
#ifndef TE_EDITOR_CONSOLE_PANEL_H
#define TE_EDITOR_CONSOLE_PANEL_H

#include <te/editor/EditorTypes.h>
#include <functional>
#include <vector>

namespace te {
namespace editor {

/**
 * @brief Console log entry with stored data.
 */
struct ConsoleEntry {
  std::string message;
  LogLevel level = LogLevel::Info;
  uint64_t timestamp = 0;
  std::string category;
  int frameCount = 0;
  int count = 1;  ///< Collapse count
};

/**
 * @brief Console command callback type.
 */
using ConsoleCommandCallback = std::function<void(const char* command)>;

/**
 * @brief Console log panel interface.
 * 
 * Displays log messages with filtering, search, and command input.
 */
class IConsolePanel {
public:
  virtual ~IConsolePanel() = default;
  
  // === Drawing ===
  
  /**
   * @brief Draw the console panel.
   */
  virtual void OnDraw() = 0;
  
  // === Log Operations ===
  
  /**
   * @brief Add a log entry.
   */
  virtual void Log(const char* message, LogLevel level = LogLevel::Info, 
                   const char* category = nullptr) = 0;
  
  /**
   * @brief Log an info message.
   */
  virtual void LogInfo(const char* message) = 0;
  
  /**
   * @brief Log a warning message.
   */
  virtual void LogWarning(const char* message) = 0;
  
  /**
   * @brief Log an error message.
   */
  virtual void LogError(const char* message) = 0;
  
  /**
   * @brief Clear all log entries.
   */
  virtual void Clear() = 0;
  
  /**
   * @brief Get all log entries.
   */
  virtual std::vector<ConsoleEntry> const& GetEntries() const = 0;
  
  /**
   * @brief Get entry count.
   */
  virtual size_t GetEntryCount() const = 0;
  
  // === Filtering ===
  
  /**
   * @brief Set the search filter string.
   */
  virtual void SetSearchFilter(const char* filter) = 0;
  
  /**
   * @brief Get the current search filter.
   */
  virtual const char* GetSearchFilter() const = 0;
  
  /**
   * @brief Set log level filter (only show entries >= this level).
   */
  virtual void SetLogLevelFilter(LogLevel minLevel) = 0;
  
  /**
   * @brief Get the current log level filter.
   */
  virtual LogLevel GetLogLevelFilter() const = 0;
  
  /**
   * @brief Enable/disable log level in filter.
   */
  virtual void SetLogLevelEnabled(LogLevel level, bool enabled) = 0;
  
  /**
   * @brief Check if log level is enabled in filter.
   */
  virtual bool IsLogLevelEnabled(LogLevel level) const = 0;
  
  // === Collapse ===
  
  /**
   * @brief Enable/disable log collapse (combine duplicate entries).
   */
  virtual void SetCollapseEnabled(bool enabled) = 0;
  
  /**
   * @brief Check if collapse is enabled.
   */
  virtual bool IsCollapseEnabled() const = 0;
  
  // === Scroll ===
  
  /**
   * @brief Scroll to bottom of log.
   */
  virtual void ScrollToBottom() = 0;
  
  /**
   * @brief Enable auto-scroll (scroll to bottom on new entries).
   */
  virtual void SetAutoScroll(bool enabled) = 0;
  
  /**
   * @brief Check if auto-scroll is enabled.
   */
  virtual bool IsAutoScrollEnabled() const = 0;
  
  // === Command Input ===
  
  /**
   * @brief Set command callback for console input.
   */
  virtual void SetCommandCallback(ConsoleCommandCallback callback) = 0;
  
  /**
   * @brief Get command history.
   */
  virtual std::vector<std::string> const& GetCommandHistory() const = 0;
  
  /**
   * @brief Clear command history.
   */
  virtual void ClearCommandHistory() = 0;
  
  // === Statistics ===
  
  /**
   * @brief Get count of entries by log level.
   */
  virtual size_t GetCountByLevel(LogLevel level) const = 0;
  
  // === Visibility ===
  
  /**
   * @brief Show/hide the console panel.
   */
  virtual void SetVisible(bool visible) = 0;
  
  /**
   * @brief Check if console panel is visible.
   */
  virtual bool IsVisible() const = 0;
};

/**
 * @brief Factory function to create console panel.
 */
IConsolePanel* CreateConsolePanel();

}  // namespace editor
}  // namespace te

#endif  // TE_EDITOR_CONSOLE_PANEL_H
