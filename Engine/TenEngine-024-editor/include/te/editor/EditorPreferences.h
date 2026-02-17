/**
 * @file EditorPreferences.h
 * @brief Editor preferences/settings interface (ABI IEditorPreferences).
 */
#ifndef TE_EDITOR_EDITOR_PREFERENCES_H
#define TE_EDITOR_EDITOR_PREFERENCES_H

#include <te/editor/EditorTypes.h>
#include <functional>

namespace te {
namespace editor {

/**
 * @brief Auto-save settings.
 */
struct AutoSaveSettings {
  bool enabled = true;
  int intervalSeconds = 300;  ///< 5 minutes default
  int maxBackups = 10;
};

/**
 * @brief Viewport preferences.
 */
struct ViewportPreferences {
  float fov = 60.0f;
  float nearClip = 0.1f;
  float farClip = 1000.0f;
  float cameraSpeed = 10.0f;
  float rotationSpeed = 0.3f;
  bool showGrid = true;
  float gridSize = 1.0f;
  bool showStats = false;
};

/**
 * @brief UI preferences.
 */
struct UIPreferences {
  EditorTheme theme = EditorTheme::Dark;
  float fontSize = 14.0f;
  float uiScale = 1.0f;
  bool showTooltips = true;
  int undoHistorySize = 100;
};

/**
 * @brief External tools configuration.
 */
struct ExternalTool {
  std::string name;
  std::string path;
  std::string arguments;
};

/**
 * @brief Editor preferences interface.
 * 
 * Manages all editor settings including theme, key bindings,
 * auto-save, viewport settings, and external tools.
 */
class IEditorPreferences {
public:
  virtual ~IEditorPreferences() = default;
  
  // === UI Settings ===
  
  /**
   * @brief Set the editor theme.
   */
  virtual void SetTheme(EditorTheme theme) = 0;
  
  /**
   * @brief Get the current editor theme.
   */
  virtual EditorTheme GetTheme() const = 0;
  
  /**
   * @brief Set the font size.
   */
  virtual void SetFontSize(float size) = 0;
  
  /**
   * @brief Get the font size.
   */
  virtual float GetFontSize() const = 0;
  
  /**
   * @brief Set the UI scale.
   */
  virtual void SetUIScale(float scale) = 0;
  
  /**
   * @brief Get the UI scale.
   */
  virtual float GetUIScale() const = 0;
  
  /**
   * @brief Set all UI preferences.
   */
  virtual void SetUIPreferences(UIPreferences const& prefs) = 0;
  
  /**
   * @brief Get all UI preferences.
   */
  virtual UIPreferences GetUIPreferences() const = 0;
  
  // === Viewport Settings ===
  
  /**
   * @brief Set viewport FOV.
   */
  virtual void SetViewportFOV(float fov) = 0;
  
  /**
   * @brief Get viewport FOV.
   */
  virtual float GetViewportFOV() const = 0;
  
  /**
   * @brief Set viewport near clip plane.
   */
  virtual void SetViewportNearClip(float nearClip) = 0;
  
  /**
   * @brief Get viewport near clip plane.
   */
  virtual float GetViewportNearClip() const = 0;
  
  /**
   * @brief Set viewport far clip plane.
   */
  virtual void SetViewportFarClip(float farClip) = 0;
  
  /**
   * @brief Get viewport far clip plane.
   */
  virtual float GetViewportFarClip() const = 0;
  
  /**
   * @brief Set all viewport preferences.
   */
  virtual void SetViewportPreferences(ViewportPreferences const& prefs) = 0;
  
  /**
   * @brief Get all viewport preferences.
   */
  virtual ViewportPreferences GetViewportPreferences() const = 0;
  
  // === Auto-save Settings ===
  
  /**
   * @brief Enable/disable auto-save.
   */
  virtual void SetAutoSaveEnabled(bool enabled) = 0;
  
  /**
   * @brief Check if auto-save is enabled.
   */
  virtual bool IsAutoSaveEnabled() const = 0;
  
  /**
   * @brief Set auto-save interval in seconds.
   */
  virtual void SetAutoSaveInterval(int seconds) = 0;
  
  /**
   * @brief Get auto-save interval in seconds.
   */
  virtual int GetAutoSaveInterval() const = 0;
  
  /**
   * @brief Set all auto-save settings.
   */
  virtual void SetAutoSaveSettings(AutoSaveSettings const& settings) = 0;
  
  /**
   * @brief Get all auto-save settings.
   */
  virtual AutoSaveSettings GetAutoSaveSettings() const = 0;
  
  // === Key Bindings ===
  
  /**
   * @brief Set a key binding.
   * @param actionId Action identifier string
   * @param binding Key binding
   */
  virtual void SetKeyBinding(const char* actionId, KeyBinding const& binding) = 0;
  
  /**
   * @brief Get a key binding.
   * @param actionId Action identifier string
   * @return Key binding or default if not found
   */
  virtual KeyBinding GetKeyBinding(const char* actionId) const = 0;
  
  /**
   * @brief Reset key binding to default.
   */
  virtual void ResetKeyBinding(const char* actionId) = 0;
  
  /**
   * @brief Reset all key bindings to defaults.
   */
  virtual void ResetAllKeyBindings() = 0;
  
  // === External Tools ===
  
  /**
   * @brief Add an external tool.
   */
  virtual void AddExternalTool(ExternalTool const& tool) = 0;
  
  /**
   * @brief Remove an external tool by name.
   */
  virtual void RemoveExternalTool(const char* name) = 0;
  
  /**
   * @brief Get all external tools.
   */
  virtual std::vector<ExternalTool> const& GetExternalTools() const = 0;
  
  /**
   * @brief Get external tool by name.
   */
  virtual ExternalTool const* GetExternalTool(const char* name) const = 0;
  
  // === Persistence ===
  
  /**
   * @brief Save preferences to file.
   */
  virtual bool Save(const char* path = nullptr) = 0;
  
  /**
   * @brief Load preferences from file.
   */
  virtual bool Load(const char* path = nullptr) = 0;
  
  /**
   * @brief Reset all preferences to defaults.
   */
  virtual void ResetToDefaults() = 0;
  
  // === Change Notification ===
  
  /**
   * @brief Set callback for preference changes.
   */
  virtual void SetOnPreferencesChanged(std::function<void()> callback) = 0;
};

/**
 * @brief Factory function to create editor preferences.
 */
IEditorPreferences* CreateEditorPreferences();

}  // namespace editor
}  // namespace te

#endif  // TE_EDITOR_EDITOR_PREFERENCES_H
