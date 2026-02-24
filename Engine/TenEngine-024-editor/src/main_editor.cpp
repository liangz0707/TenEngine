/**
 * @file main_editor.cpp
 * @brief tenengine_editor entry point.
 */
#include <te/editor/Editor.h>
#include <te/application/Application.h>
#include <te/resource/ResourceManager.h>
#include <te/texture/TextureModuleInit.h>
#include <te/material/MaterialModuleInit.h>
#include <te/mesh/MeshModuleInit.h>
#include <te/entity/PropertyReflection.h>
#include <te/world/WorldModuleInit.h>

int main(int argc, char const** argv) {
  te::application::IApplication* app = te::application::CreateApplication();
  if (!app) return 1;

  te::application::InitParams initParams;
  initParams.argc = argc;
  initParams.argv = argv;
  if (!app->Initialize(&initParams)) return 1;

  // Initialize property registry before registering components
  te::entity::InitializePropertyRegistry();

  // Register world module (components with properties)
  te::world::RegisterWorldModule();

  te::editor::EditorContext ctx;
  ctx.projectRootPath = "./assets";
  ctx.application = app;
  te::resource::IResourceManager* resMgr = te::resource::GetResourceManager();
  ctx.resourceManager = resMgr;

  if (resMgr) {
    te::texture::InitializeTextureModule(resMgr);
    te::mesh::InitializeMeshModule(resMgr);
    te::material::InitializeMaterialModule(resMgr);
  }

  te::editor::IEditor* editor = te::editor::CreateEditor(ctx);
  if (!editor) return 1;

  editor->Run(ctx);

  delete editor;

  // Shutdown property registry
  te::entity::ShutdownPropertyRegistry();

  delete app;
  return 0;
}
