/**
 * @file MaterialModuleInit.cpp
 * @brief Register Material resource factory; optional engine init (Shader + Material + LoadAllShaders).
 */
#include <te/material/MaterialModuleInit.h>
#include <te/material/MaterialResource.h>
#include <te/shader/ShaderModuleInit.h>
#include <te/resource/ResourceManager.h>
#include <te/resource/ResourceTypes.h>

namespace te {
namespace material {

namespace {
resource::IResource* CreateMaterialResource(resource::ResourceType type) {
  if (type == resource::ResourceType::Material) {
    return new MaterialResource();
  }
  return nullptr;
}
}  // namespace

void InitializeMaterialModule(resource::IResourceManager* manager) {
  if (!manager) return;
  manager->RegisterResourceFactory(resource::ResourceType::Material, CreateMaterialResource);
}

bool InitializeResourceModulesForEngine(resource::IResourceManager* manager, char const* shaderManifestPath) {
  if (!manager) return false;
  shader::InitializeShaderModule(manager);
  if (shaderManifestPath && *shaderManifestPath) {
    if (!shader::LoadAllShaders(manager, shaderManifestPath)) return false;
  }
  InitializeMaterialModule(manager);
  return true;
}

}  // namespace material
}  // namespace te
