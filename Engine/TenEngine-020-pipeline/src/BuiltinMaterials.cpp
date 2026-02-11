/**
 * @file BuiltinMaterials.cpp
 * @brief 020-Pipeline: Built-in materials implementation (loads from builtin/ assets).
 */

#include <te/pipeline/BuiltinMaterials.h>
#include <te/pipelinecore/RenderItem.h>
#include <te/resource/ResourceManager.h>
#include <te/resource/ResourceTypes.h>
#include <cstring>
#include <string>
#include <unordered_map>

namespace te {
namespace pipeline {

namespace {
  te::resource::IResourceManager* g_builtinResourceManager = nullptr;
  std::unordered_map<std::string, pipelinecore::IMaterialHandle const*> g_postProcessMaterialCache;
  pipelinecore::IMaterialHandle const* g_lightMaterials[3] = {};
  char const* const kBuiltinShaderPrefix = "builtin/shaders/";
}

void SetBuiltinMaterialResourceManager(te::resource::IResourceManager* manager) {
  g_builtinResourceManager = manager;
}

pipelinecore::IMaterialHandle const* GetPostProcessMaterial(char const* name) {
  if (!name) return nullptr;
  auto it = g_postProcessMaterialCache.find(name);
  if (it != g_postProcessMaterialCache.end())
    return it->second;
  pipelinecore::IMaterialHandle const* result = nullptr;
  if (g_builtinResourceManager) {
    std::string path = kBuiltinShaderPrefix;
    path += name;
    path += ".material";
    te::resource::IResource* r = g_builtinResourceManager->LoadSync(path.c_str(), te::resource::ResourceType::Material);
    if (r)
      result = reinterpret_cast<pipelinecore::IMaterialHandle const*>(r);
  }
  g_postProcessMaterialCache[name] = result;
  return result;
}

pipelinecore::IMaterialHandle const* GetLightMaterial(unsigned lightTypeIndex) {
  if (lightTypeIndex >= 3) return nullptr;
  pipelinecore::IMaterialHandle const* m = g_lightMaterials[lightTypeIndex];
  if (m) return m;
  // Load builtin/shaders/light_point.shader, light_directional.shader, light_spot.shader
  return nullptr;
}

}  // namespace pipeline
}  // namespace te
