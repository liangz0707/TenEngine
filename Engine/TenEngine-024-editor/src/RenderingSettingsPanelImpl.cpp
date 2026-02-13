/**
 * @file RenderingSettingsPanelImpl.cpp
 * @brief Rendering settings panel (024-Editor).
 */
#include <te/editor/RenderingSettingsPanel.h>
#include <te/core/platform.h>

namespace te {
namespace editor {

class RenderingSettingsPanelImpl : public IRenderingSettingsPanel {
public:
  void Show() override { m_visible = true; }
  void Hide() override { m_visible = false; }
  bool IsVisible() const override { return m_visible; }
  RenderingConfig GetConfig() const override { return m_config; }
  void SetConfig(RenderingConfig const& config) override { m_config = config; }
  bool SaveConfig(char const* path) override { (void)path; return true; }
  bool LoadConfig(char const* path) override { (void)path; return true; }
private:
  bool m_visible = false;
  RenderingConfig m_config;
};

IRenderingSettingsPanel* CreateRenderingSettingsPanel() {
  return new RenderingSettingsPanelImpl();
}

}  // namespace editor
}  // namespace te
