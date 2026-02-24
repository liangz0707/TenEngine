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
    // Use standard ImGui toolbar rendering with buttons in same line
    ImGui::PushID("MainToolbar");

    DrawTransformTools();
    ImGui::SameLine();
    ImGui::Text("|");
    ImGui::SameLine();

    DrawPlayControls();
    ImGui::SameLine();
    ImGui::Text("|");
    ImGui::SameLine();

    DrawSnapControls();
    ImGui::SameLine();
    ImGui::Text("|");
    ImGui::SameLine();

    DrawViewControls();

    DrawCustomButtons();

    ImGui::PopID();
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
      if (b.id == button.id) {
        b = button;
        return;
      }
    }
    m_customButtons.push_back(button);
  }

  void RemoveButton(const char* id) override {
    if (!id) return;

    std::string idStr(id);
    m_customButtons.erase(
      std::remove_if(m_customButtons.begin(), m_customButtons.end(),
        [&idStr](ToolbarButton const& b) { return b.id == idStr; }),
      m_customButtons.end());
  }

  void SetButtonToggled(const char* id, bool toggled) override {
    std::string idStr(id);
    for (auto& b : m_customButtons) {
      if (b.id == idStr) {
        b.toggled = toggled;
        return;
      }
    }
  }

  void SetButtonEnabled(const char* id, bool enabled) override {
    std::string idStr(id);
    for (auto& b : m_customButtons) {
      if (b.id == idStr) {
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
    if (ImGui::Button(translateActive ? "[T]" : "T")) {
      SetTransformTool(GizmoMode::Translate);
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Translate (W)");
    ImGui::SameLine();

    // Rotate button
    bool rotateActive = (m_transformTool == GizmoMode::Rotate);
    if (ImGui::Button(rotateActive ? "[R]" : "R")) {
      SetTransformTool(GizmoMode::Rotate);
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Rotate (E)");
    ImGui::SameLine();

    // Scale button
    bool scaleActive = (m_transformTool == GizmoMode::Scale);
    if (ImGui::Button(scaleActive ? "[S]" : "S")) {
      SetTransformTool(GizmoMode::Scale);
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Scale (R)");
  }

  void DrawPlayControls() {
    // Play button
    bool isPlaying = (m_playModeState == PlayModeState::Playing);
    if (ImGui::Button(isPlaying ? "||" : ">")) {
      if (m_onPlayClicked) {
        m_onPlayClicked();
      }
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip(isPlaying ? "Playing" : "Play (Ctrl+P)");
    ImGui::SameLine();

    // Pause button
    bool isPaused = (m_playModeState == PlayModeState::Paused);
    if (ImGui::Button(isPaused ? "[||]" : "||")) {
      if (m_onPauseClicked) {
        m_onPauseClicked();
      }
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Pause (Ctrl+Shift+P)");
    ImGui::SameLine();

    // Stop button
    bool isStopped = (m_playModeState == PlayModeState::Stopped);
    if (ImGui::Button("[]")) {
      if (!isStopped && m_onStopClicked) {
        m_onStopClicked();
      }
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Stop (Ctrl+Shift+Q)");
    ImGui::SameLine();

    // Step button
    if (ImGui::Button("|>")) {
      if (m_onStepClicked) {
        m_onStepClicked();
      }
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Step Frame (F10)");
  }

  void DrawSnapControls() {
    // Snap toggle
    if (ImGui::Button(m_snapEnabled ? "[Snap]" : "Snap")) {
      SetSnapEnabled(!m_snapEnabled);
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Toggle Snapping");
    ImGui::SameLine();

    // Grid visibility
    if (ImGui::Button(m_gridVisible ? "[Grid]" : "Grid")) {
      SetGridVisible(!m_gridVisible);
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Toggle Grid");
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
    ImGui::SameLine();

    // Coordinate space toggle
    const char* spaceLabel = (m_gizmoSpace == GizmoSpace::World) ? "World" : "Local";
    if (ImGui::Button(spaceLabel)) {
      SetGizmoSpace(m_gizmoSpace == GizmoSpace::World ? GizmoSpace::Local : GizmoSpace::World);
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Toggle World/Local Space");
  }

  void DrawCustomButtons() {
    for (auto& button : m_customButtons) {
      if (button.id.empty()) continue;

      const char* label = button.icon ? button.icon : button.id.c_str();
      bool clicked = false;

      if (!button.enabled) {
        ImGui::BeginDisabled();
      }

      if (button.toggle) {
        std::string btnLabel = button.toggled ? std::string("[") + label + "]" : label;
        clicked = ImGui::Button(btnLabel.c_str());
      } else {
        clicked = ImGui::Button(label);
      }

      if (!button.enabled) {
        ImGui::EndDisabled();
      }

      if (!button.tooltip.empty() && ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s", button.tooltip.c_str());
      }

      if (clicked && m_onButtonClicked) {
        m_onButtonClicked(button.id.c_str());
      }

      ImGui::SameLine();
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
