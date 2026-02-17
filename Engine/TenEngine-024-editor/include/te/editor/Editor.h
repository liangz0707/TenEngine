/**
 * @file Editor.h
 * @brief Editor interface (contract: specs/_contracts/024-editor-ABI.md).
 */
#ifndef TE_EDITOR_EDITOR_H
#define TE_EDITOR_EDITOR_H

#include <te/editor/EditorTypes.h>

namespace te {
namespace application {
class IApplication;
}  // namespace application
namespace resource {
class IResourceManager;
}  // namespace resource

namespace editor {

struct EditorContext {
  char const* projectRootPath = "./assets";
  void* windowHandle = nullptr;
  te::application::IApplication* application = nullptr;
  te::resource::IResourceManager* resourceManager = nullptr;
};

// Forward declarations for existing interfaces
class ISceneView;
class IResourceView;
class IPropertyPanel;
class IViewport;
class IRenderingSettingsPanel;

// Forward declarations for new interfaces
class IGizmo;
class IEditorCamera;
class ISelectionManager;
class ISnapSettings;
class IMainMenu;
class IToolbar;
class IStatusBar;
class IConsolePanel;
class IEditorPreferences;
class IProfilerPanel;
class IStatisticsPanel;
class ILayoutManager;

/**
 * @brief Editor main interface.
 * 
 * Provides access to all editor subsystems including:
 * - Scene tree (left panel)
 * - Resource browser (bottom panel)
 * - Property panel (right panel)
 * - Render viewport (center)
 * - Gizmo, camera, selection systems
 * - Menu, toolbar, status bar
 * - Console, profiler, statistics panels
 */
class IEditor {
public:
  virtual ~IEditor() = default;
  
  // === Core Lifecycle ===
  
  /**
   * @brief Run the editor main loop.
   */
  virtual void Run(EditorContext const& ctx) = 0;
  
  // === Existing Panel Accessors ===
  
  /**
   * @brief Get the scene tree view (left panel).
   */
  virtual ISceneView* GetSceneView() = 0;
  
  /**
   * @brief Get the resource browser (bottom panel).
   */
  virtual IResourceView* GetResourceView() = 0;
  
  /**
   * @brief Get the property panel (right panel).
   */
  virtual IPropertyPanel* GetPropertyPanel() = 0;
  
  /**
   * @brief Get the render viewport (center panel).
   */
  virtual IViewport* GetRenderViewport() = 0;
  
  /**
   * @brief Get the rendering settings panel.
   */
  virtual IRenderingSettingsPanel* GetRenderingSettingsPanel() = 0;
  
  // === New Component Accessors ===
  
  /**
   * @brief Get the gizmo transformation tool.
   */
  virtual IGizmo* GetGizmo() = 0;
  
  /**
   * @brief Get the editor camera controller.
   */
  virtual IEditorCamera* GetEditorCamera() = 0;
  
  /**
   * @brief Get the selection manager.
   */
  virtual ISelectionManager* GetSelectionManager() = 0;
  
  /**
   * @brief Get the snap settings manager.
   */
  virtual ISnapSettings* GetSnapSettings() = 0;
  
  /**
   * @brief Get the main menu.
   */
  virtual IMainMenu* GetMainMenu() = 0;
  
  /**
   * @brief Get the toolbar.
   */
  virtual IToolbar* GetToolbar() = 0;
  
  /**
   * @brief Get the status bar.
   */
  virtual IStatusBar* GetStatusBar() = 0;
  
  /**
   * @brief Get the console log panel.
   */
  virtual IConsolePanel* GetConsolePanel() = 0;
  
  /**
   * @brief Get the editor preferences.
   */
  virtual IEditorPreferences* GetPreferences() = 0;
  
  /**
   * @brief Get the profiler panel.
   */
  virtual IProfilerPanel* GetProfilerPanel() = 0;
  
  /**
   * @brief Get the statistics panel.
   */
  virtual IStatisticsPanel* GetStatisticsPanel() = 0;
  
  /**
   * @brief Get the layout manager.
   */
  virtual ILayoutManager* GetLayoutManager() = 0;
  
  // === Play Mode Control ===
  
  /**
   * @brief Enter play mode (run the game).
   */
  virtual void EnterPlayMode() = 0;
  
  /**
   * @brief Pause play mode.
   */
  virtual void PausePlayMode() = 0;
  
  /**
   * @brief Stop play mode and return to edit mode.
   */
  virtual void StopPlayMode() = 0;
  
  /**
   * @brief Step a single frame in play mode.
   */
  virtual void StepFrame() = 0;
  
  /**
   * @brief Check if editor is in play mode.
   */
  virtual bool IsInPlayMode() const = 0;
  
  /**
   * @brief Get current play mode state.
   */
  virtual PlayModeState GetPlayModeState() const = 0;
  
  // === Layout Management ===
  
  /**
   * @brief Save current layout to file.
   */
  virtual void SaveLayout(char const* path) = 0;
  
  /**
   * @brief Load layout from file.
   */
  virtual void LoadLayout(char const* path) = 0;
  
  /**
   * @brief Reset layout to default.
   */
  virtual void ResetLayout() = 0;
};

/**
 * @brief Factory function to create an editor instance.
 */
IEditor* CreateEditor(EditorContext const& ctx);

}  // namespace editor
}  // namespace te

#endif  // TE_EDITOR_EDITOR_H
