/**
 * @file main_editor.cpp
 * @brief tenengine_editor entry point.
 */
#include <te/editor/Editor.h>
#include <te/application/Application.h>
#include <te/resource/ResourceManager.h>

int main(int argc, char const** argv) {
  te::application::IApplication* app = te::application::CreateApplication();
  if (!app) return 1;

  te::application::InitParams initParams;
  initParams.argc = argc;
  initParams.argv = argv;
  if (!app->Initialize(&initParams)) return 1;

  te::editor::EditorContext ctx;
  ctx.projectRootPath = "./assets";
  ctx.application = app;
  ctx.resourceManager = te::resource::GetResourceManager();

  te::editor::IEditor* editor = te::editor::CreateEditor(ctx);
  if (!editor) return 1;

  editor->Run(ctx);

  delete editor;
  delete app;
  return 0;
}
