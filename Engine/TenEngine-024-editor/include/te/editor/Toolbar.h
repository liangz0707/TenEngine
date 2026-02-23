/**
 * @file Toolbar.h
 * @brief Editor toolbar interface (ABI IToolbar).
 */
#ifndef TE_EDITOR_TOOLBAR_H
#define TE_EDITOR_TOOLBAR_H

#include <te/editor/EditorTypes.h>
#include <functional>

namespace te {
namespace editor {

// Note: ToolbarButton is defined in EditorTypes.h

/**
 * @brief Editor toolbar interface.
 *
 * Provides the main toolbar for quick access to common editor functions
 * like transform tools, play controls, and view options.
 */
class IToolbar {
public:
  virtual ~IToolbar() = default;

  // === Drawing ===

  /**
   * @brief Draw the toolbar.
   */
  virtual void OnDraw() = 0;
  
  // === Transform Tools ===
  
  /**
   * @brief Set the active transform tool.
   */
  virtual void SetTransformTool(GizmoMode tool) = 0;
  
  /**
   * @brief Get the active transform tool.
   */
  virtual GizmoMode GetTransformTool() const = 0;
  
  /**
   * @brief Set callback for transform tool change.
   */
  virtual void SetOnTransformToolChanged(std::function<void(GizmoMode)> callback) = 0;
  
  // === Play Mode ===
  
  /**
   * @brief Set the current play mode state.
   */
  virtual void SetPlayModeState(PlayModeState state) = 0;
  
  /**
   * @brief Get the current play mode state.
   */
  virtual PlayModeState GetPlayModeState() const = 0;
  
  /**
   * @brief Set callback for play button click.
   */
  virtual void SetOnPlayClicked(std::function<void()> callback) = 0;
  
  /**
   * @brief Set callback for pause button click.
   */
  virtual void SetOnPauseClicked(std::function<void()> callback) = 0;
  
  /**
   * @brief Set callback for stop button click.
   */
  virtual void SetOnStopClicked(std::function<void()> callback) = 0;
  
  /**
   * @brief Set callback for step button click (frame advance).
   */
  virtual void SetOnStepClicked(std::function<void()> callback) = 0;
  
  // === Snap Controls ===
  
  /**
   * @brief Set snap enabled state.
   */
  virtual void SetSnapEnabled(bool enabled) = 0;
  
  /**
   * @brief Get snap enabled state.
   */
  virtual bool IsSnapEnabled() const = 0;
  
  /**
   * @brief Set callback for snap toggle.
   */
  virtual void SetOnSnapToggled(std::function<void(bool)> callback) = 0;
  
  // === View Mode ===
  
  /**
   * @brief Set viewport rendering mode.
   */
  virtual void SetViewportMode(ViewportMode mode) = 0;
  
  /**
   * @brief Get viewport rendering mode.
   */
  virtual ViewportMode GetViewportMode() const = 0;
  
  /**
   * @brief Set callback for view mode change.
   */
  virtual void SetOnViewportModeChanged(std::function<void(ViewportMode)> callback) = 0;
  
  // === Grid ===
  
  /**
   * @brief Set grid visibility.
   */
  virtual void SetGridVisible(bool visible) = 0;
  
  /**
   * @brief Get grid visibility.
   */
  virtual bool IsGridVisible() const = 0;
  
  /**
   * @brief Set callback for grid toggle.
   */
  virtual void SetOnGridToggled(std::function<void(bool)> callback) = 0;
  
  // === Coordinate Space ===
  
  /**
   * @brief Set gizmo coordinate space.
   */
  virtual void SetGizmoSpace(GizmoSpace space) = 0;
  
  /**
   * @brief Get gizmo coordinate space.
   */
  virtual GizmoSpace GetGizmoSpace() const = 0;
  
  /**
   * @brief Set callback for coordinate space change.
   */
  virtual void SetOnGizmoSpaceChanged(std::function<void(GizmoSpace)> callback) = 0;
  
  // === Custom Buttons ===
  
  /**
   * @brief Add a custom button to the toolbar.
   */
  virtual void AddButton(ToolbarButton const& button) = 0;
  
  /**
   * @brief Remove a button by ID.
   */
  virtual void RemoveButton(const char* id) = 0;
  
  /**
   * @brief Set button toggle state.
   */
  virtual void SetButtonToggled(const char* id, bool toggled) = 0;
  
  /**
   * @brief Set button enabled state.
   */
  virtual void SetButtonEnabled(const char* id, bool enabled) = 0;
  
  /**
   * @brief Set callback for custom button click.
   */
  virtual void SetOnButtonClicked(std::function<void(const char*)> callback) = 0;
};

/**
 * @brief Factory function to create toolbar.
 */
IToolbar* CreateToolbar();

}  // namespace editor
}  // namespace te

#endif  // TE_EDITOR_TOOLBAR_H
