/**
 * @file RenderingSettingsPanel.h
 * @brief Rendering settings panel (ABI).
 */
#ifndef TE_EDITOR_RENDERING_SETTINGS_PANEL_H
#define TE_EDITOR_RENDERING_SETTINGS_PANEL_H

#include "RenderingConfig.h"

namespace te {
namespace editor {

class IRenderingSettingsPanel {
public:
  virtual ~IRenderingSettingsPanel() = default;
  virtual void Show() = 0;
  virtual void Hide() = 0;
  virtual bool IsVisible() const = 0;
  virtual RenderingConfig GetConfig() const = 0;
  virtual void SetConfig(RenderingConfig const& config) = 0;
  virtual bool SaveConfig(char const* path) = 0;
  virtual bool LoadConfig(char const* path) = 0;
};

IRenderingSettingsPanel* CreateRenderingSettingsPanel();

}  // namespace editor
}  // namespace te

#endif  // TE_EDITOR_RENDERING_SETTINGS_PANEL_H
