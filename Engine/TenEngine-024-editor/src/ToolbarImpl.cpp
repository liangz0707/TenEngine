/**
 * @file ToolbarImpl.cpp
 * @brief Editor toolbar implementation (024-Editor).
 */
#include <te/editor/Toolbar.h>
#include <imgui.h>
#include <cstring>

namespace te {
namespace editor {

class ToolbarImpl : public IToolbar {
public:
  ToolbarImpl()
    : m_transformTool(GizmoMode::Translate)
    , m_playModeState(PlayModeState::Stopped)
    , m_snapEnabled(false)
    , m_viewportMode(ViewportMode::Shaded)
    , m_gridVisible(true)
    , m_gizmoSpace(GizmoSpace::World)
  {
  }

  // === Drawing ===
  
  void OnDraw() override {
    if (!ImGui::BeginToolbar("MainToolbar")) return;
    
    DrawTransformTools();
    ImGui::Separator();
    
    DrawPlayControls();
    ImGui::Separator();
    
    DrawSnapControls();
    ImGui::Separator();
    
    DrawViewControls();
    
    DrawCustomButtons();
    
    ImGui::EndToolbar();
  }
  
  // === Transform Tools ===
  
  void SetTransformTool(GizmoMode tool) override {
    m_transformTool = tool;
    if (m_onTransformToolChanged) {
      m_onTransformToolChanged(tool);
    }
  }
  
  GizmoMode GetTransformTool() const override {
    return m_transformTool;
  }
  
  void SetOnTransformToolChanged(std::function<void(GizmoMode)> callback) override {
    m_onTransformToolChanged = std::move(callback);
  }
  
  // === Play Mode ===
  
  void SetPlayModeState(PlayModeState state) override {
    m_playModeState = state;
  }
  
  PlayModeState GetPlayModeState() const override {
    return m_playModeState;
  }
  
  void SetOnPlayClicked(std::function<void()> callback) override {
    m_onPlayClicked = std::move(callback);
  }
  
  void SetOnPauseClicked(std::function<void()> callback) override {
    m_onPauseClicked = std::move(callback);
  }
  
  void SetOnStopClicked(std::function<void()> callback) override {
    m_onStopClicked = std::move(callback);
  }
  
  void SetOnStepClicked(std::function<void()> callback) override {
    m_onStepClicked = std::move(callback);
  }
  
  // === Snap Controls ===
  
  void SetSnapEnabled(bool enabled) override {
    m_snapEnabled = enabled;
    if (m_onSnapToggled) {
      m_onSnapToggled(enabled);
    }
  }
  
  bool IsSnapEnabled() const override {
    return m_snapEnabled;
  }
  
  void SetOnSnapToggled(std::function<void(bool)> callback) override {
    m_onSnapToggled = std::move(callback);
  }
  
  // === View Mode ===
  
  void SetViewportMode(ViewportMode mode) override {
    m_viewportMode = mode;
    if (m_onViewportModeChanged) {
      m_onViewportModeChanged(mode);
    }
  }
  
  ViewportMode GetViewportMode() const override {
    return m_viewportMode;
  }
  
  void SetOnViewportModeChanged(std::function<void(ViewportMode)> callback) override {
    m_onViewportModeChanged = std::move(callback);
  }
  
  // === Grid ===
  
  void SetGridVisible(bool visible) override {
    m_gridVisible = visible;
    if (m_onGridToggled) {
      m_onGridToggled(visible);
    }
  }
  
  bool IsGridVisible() const override {
    return m_gridVisible;
  }
  
  void SetOnGridToggled(std::function<void(bool)> callback) override {
    m_onGridToggled = std::move(callback);
  }
  
  // === Coordinate Space ===
  
  void SetGizmoSpace(GizmoSpace space) override {
    m_gizmoSpace = space;
    if (m_onGizmoSpaceChanged) {
      m_onGizmoSpaceChanged(space);
    }
  }
  
  GizmoSpace GetGizmoSpace() const override {
    return m_gizmoSpace;
  }
  
  void SetOnGizmoSpaceChanged(std::function<void(GizmoSpace)> callback) override {
    m_onGizmoSpaceChanged = std::move(callback);
  }
  
  // === Custom Buttons ===
  
  void AddButton(ToolbarButton const& button) override {
    // Check if button already exists
    for (auto& b : m_customButtons) {
      if (b.id && button.id && std::string(b.id) == std::string(button.id)) {
        b = button;
        return;
      }
    }
    m_customButtons.push_back(button);
  }
  
  void RemoveButton(const char* id) override {
    if (!id) return;
    
    m_customButtons.erase(
      std::remove_if(m_customButtons.begin(), m_customButtons.end(),
        [id](ToolbarButton const& b) { return b.id && std::string(b.id) == std::string(id); }),
      m_customButtons.end());
  }
  
  void SetButtonToggled(const char* id, bool toggled) override {
    for (auto& b : m_customButtons) {
      if (b.id && std::string(b.id) == std::string(id)) {
        b.toggled = toggled;
        return;
      }
    }
  }
  
  void SetButtonEnabled(const char* id, bool enabled) override {
    for (auto& b : m_customButtons) {
      if (b.id && std::string(b.id) == std::string(id)) {
        b.enabled = enabled;
        return;
      }
    }
  }
  
  void SetOnButtonClicked(std::function<void(const char*)> callback) override {
    m_onButtonClicked = std::move(callback);
  }

private:
  void DrawTransformTools() {
    // Translate button
    bool translateActive = (m_transformTool == GizmoMode::Translate);
    if (ImGui::ToolbarButton("T", "Translate (W)", translateActive)) {
      SetTransformTool(GizmoMode::Translate);
    }
    
    // Rotate button
    bool rotateActive = (m_transformTool == GizmoMode::Rotate);
    if (ImGui::ToolbarButton("R", "Rotate (E)", rotateActive)) {
      SetTransformTool(GizmoMode::Rotate);
    }
    
    // Scale button
    bool scaleActive = (m_transformTool == GizmoMode::Scale);
    if (ImGui::ToolbarButton("S", "Scale (R)", scaleActive)) {
      SetTransformTool(GizmoMode::Scale);
    }
  }
  
  void DrawPlayControls() {
    // Play button
    bool isPlaying = (m_playModeState == PlayModeState::Playing);
    if (ImGui::ToolbarButton(isPlaying ? "||" : ">", 
                             isPlaying ? "Playing" : "Play (Ctrl+P)", 
                             isPlaying)) {
      if (m_onPlayClicked) {
        m_onPlayClicked();
      }
    }
    
    // Pause button
    bool isPaused = (m_playModeState == PlayModeState::Paused);
    if (ImGui::ToolbarButton("||", "Pause (Ctrl+Shift+P)", isPaused)) {
      if (m_onPauseClicked) {
        m_onPauseClicked();
      }
    }
    
    // Stop button
    bool isStopped = (m_playModeState == PlayModeState::Stopped);
    if (ImGui::ToolbarButton("[]", "Stop (Ctrl+Shift+Q)", false, !isStopped)) {
      if (m_onStopClicked) {
        m_onStopClicked();
      }
    }
    
    // Step button
    if (ImGui::ToolbarButton("|>", "Step Frame (F10)", false, isPaused)) {
      if (m_onStepClicked) {
        m_onStepClicked();
      }
    }
  }
  
  void DrawSnapControls() {
    // Snap toggle
    if (ImGui::ToolbarButton("Snap", "Toggle Snapping", m_snapEnabled)) {
      SetSnapEnabled(!m_snapEnabled);
    }
    
    // Grid visibility
    if (ImGui::ToolbarButton("Grid", "Toggle Grid", m_gridVisible)) {
      SetGridVisible(!m_gridVisible);
    }
  }
  
  void DrawViewControls() {
    // Viewport mode dropdown
    const char* viewModes[] = { "Shaded", "Wireframe", "Shaded+Wire", "Depth", "Normals", "Albedo" };
    int currentMode = static_cast<int>(m_viewportMode);
    
    ImGui::SetNextItemWidth(100);
    if (ImGui::BeginCombo("##ViewMode", viewModes[currentMode])) {
      for (int i = 0; i < 6; ++i) {
        bool selected = (i == currentMode);
        if (ImGui::Selectable(viewModes[i], selected)) {
          SetViewportMode(static_cast<ViewportMode>(i));
        }
        if (selected) {
          ImGui::SetItemDefaultFocus();
        }
      }
      ImGui::EndCombo();
    }
    
    // Coordinate space toggle
    const char* spaceLabel = (m_gizmoSpace == GizmoSpace::World) ? "World" : "Local";
    if (ImGui::ToolbarButton(spaceLabel, "Toggle World/Local Space", false)) {
      SetGizmoSpace(m_gizmoSpace == GizmoSpace::World ? GizmoSpace::Local : GizmoSpace::World);
    }
  }
  
  void DrawCustomButtons() {
    for (auto& button : m_customButtons) {
      if (!button.id) continue;
      
      const char* label = button.icon ? button.icon : button.id;
      bool clicked = false;
      
      if (button.toggle) {
        clicked = ImGui::ToolbarButton(label, button.tooltip ? button.tooltip : "", button.toggled, button.enabled);
      } else {
        clicked = ImGui::ToolbarButton(label, button.tooltip ? button.tooltip : "", false, button.enabled);
      }
      
      if (clicked && m_onButtonClicked) {
        m_onButtonClicked(button.id);
      }
    }
  }
  
  // Transform tool state
  GizmoMode m_transformTool;
  
  // Play mode state
  PlayModeState m_playModeState;
  
  // Snap state
  bool m_snapEnabled;
  
  // View state
  ViewportMode m_viewportMode;
  bool m_gridVisible;
  GizmoSpace m_gizmoSpace;
  
  // Custom buttons
  std::vector<ToolbarButton> m_customButtons;
  
  // Callbacks
  std::function<void(GizmoMode)> m_onTransformToolChanged;
  std::function<void()> m_onPlayClicked;
  std::function<void()> m_onPauseClicked;
  std::function<void()> m_onStopClicked;
  std::function<void()> m_onStepClicked;
  std::function<void(bool)> m_onSnapToggled;
  std::function<void(ViewportMode)> m_onViewportModeChanged;
  std::function<void(bool)> m_onGridToggled;
  std::function<void(GizmoSpace)> m_onGizmoSpaceChanged;
  std::function<void(const char*)> m_onButtonClicked;
};

IToolbar* CreateToolbar() {
  return new ToolbarImpl();
}

}  // namespace editor
}  // namespace te
