/**
 * @file Editor.h
 * @brief Editor interface (contract: specs/_contracts/024-editor-ABI.md).
 */
#ifndef TE_EDITOR_EDITOR_H
#define TE_EDITOR_EDITOR_H

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

class ISceneView;
class IResourceView;
class IPropertyPanel;
class IViewport;
class IRenderingSettingsPanel;

class IEditor {
public:
  virtual ~IEditor() = default;
  virtual void Run(EditorContext const& ctx) = 0;
  virtual ISceneView* GetSceneView() = 0;
  virtual IResourceView* GetResourceView() = 0;
  virtual IPropertyPanel* GetPropertyPanel() = 0;
  virtual IViewport* GetRenderViewport() = 0;
  virtual IRenderingSettingsPanel* GetRenderingSettingsPanel() = 0;
};

IEditor* CreateEditor(EditorContext const& ctx);

}  // namespace editor
}  // namespace te

#endif  // TE_EDITOR_EDITOR_H
