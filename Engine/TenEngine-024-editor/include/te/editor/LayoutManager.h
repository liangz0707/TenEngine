/**
 * @file LayoutManager.h
 * @brief Dock layout manager interface (ABI ILayoutManager).
 */
#ifndef TE_EDITOR_LAYOUT_MANAGER_H
#define TE_EDITOR_LAYOUT_MANAGER_H

#include <te/editor/EditorTypes.h>
#include <functional>
#include <vector>

namespace te {
namespace editor {

// Note: PanelVisibility and LayoutDef are defined in EditorTypes.h

/**
 * @brief Layout manager interface.
 *
 * Manages editor panel docking, layout presets, and layout persistence.
 */
class ILayoutManager {
public:
  virtual ~ILayoutManager() = default;

  // === Current Layout ===

  /**
   * @brief Apply a layout by name.
   */
  virtual bool ApplyLayout(const char* name) = 0;
  
  /**
   * @brief Get the current layout name.
   */
  virtual const char* GetCurrentLayoutName() const = 0;
  
  /**
   * @brief Save the current layout.
   */
  virtual void SaveCurrentLayout() = 0;
  
  /**
   * @brief Save current layout with a new name.
   */
  virtual void SaveCurrentLayoutAs(const char* name) = 0;
  
  // === Layout Management ===
  
  /**
   * @brief Add a custom layout.
   */
  virtual void AddLayout(LayoutDef const& layout) = 0;
  
  /**
   * @brief Remove a custom layout by name.
   */
  virtual bool RemoveLayout(const char* name) = 0;
  
  /**
   * @brief Get all available layouts.
   */
  virtual std::vector<LayoutDef> const& GetLayouts() const = 0;
  
  /**
   * @brief Get a specific layout by name.
   */
  virtual LayoutDef const* GetLayout(const char* name) const = 0;
  
  /**
   * @brief Check if a layout exists.
   */
  virtual bool HasLayout(const char* name) const = 0;
  
  // === Panel Management ===
  
  /**
   * @brief Set panel visibility.
   */
  virtual void SetPanelVisible(const char* panelId, bool visible) = 0;
  
  /**
   * @brief Check if panel is visible.
   */
  virtual bool IsPanelVisible(const char* panelId) const = 0;
  
  /**
   * @brief Toggle panel visibility.
   */
  virtual void TogglePanel(const char* panelId) = 0;
  
  /**
   * @brief Dock a panel.
   */
  virtual void DockPanel(const char* panelId) = 0;
  
  /**
   * @brief Float a panel.
   */
  virtual void FloatPanel(const char* panelId) = 0;
  
  /**
   * @brief Check if panel is docked.
   */
  virtual bool IsPanelDocked(const char* panelId) const = 0;
  
  /**
   * @brief Set panel dock position.
   */
  virtual void SetPanelDockRect(const char* panelId, EditorRect const& rect) = 0;
  
  /**
   * @brief Get all panel states.
   */
  virtual std::vector<PanelVisibility> const& GetPanelStates() const = 0;
  
  // === Viewport Layout ===
  
  /**
   * @brief Set viewport layout type.
   */
  virtual void SetViewportLayout(ViewportLayout layout) = 0;
  
  /**
   * @brief Get viewport layout type.
   */
  virtual ViewportLayout GetViewportLayout() const = 0;
  
  // === Persistence ===
  
  /**
   * @brief Save layout to file.
   */
  virtual bool SaveToFile(const char* path) = 0;
  
  /**
   * @brief Load layout from file.
   */
  virtual bool LoadFromFile(const char* path) = 0;
  
  /**
   * @brief Reset to default layout.
   */
  virtual void ResetToDefault() = 0;
  
  // === Events ===
  
  /**
   * @brief Set callback for layout changes.
   */
  virtual void SetOnLayoutChanged(std::function<void(const char*)> callback) = 0;
  
  /**
   * @brief Set callback for panel visibility changes.
   */
  virtual void SetOnPanelVisibilityChanged(
      std::function<void(const char*, bool)> callback) = 0;
};

/**
 * @brief Factory function to create layout manager.
 */
ILayoutManager* CreateLayoutManager();

}  // namespace editor
}  // namespace te

#endif  // TE_EDITOR_LAYOUT_MANAGER_H
