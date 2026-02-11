/**
 * @file BuiltinMaterials.cpp
 * @brief 020-Pipeline: Built-in materials implementation (loads from builtin/ assets).
 */

#include <te/pipeline/BuiltinMaterials.h>
#include <te/pipelinecore/RenderItem.h>
#include <te/resource/ResourceManager.h>
#include <te/resource/ResourceTypes.h>
#include <cstring>

namespace te {
namespace pipeline {

namespace {
  pipelinecore::IMaterialHandle const* g_postProcessPlaceholder = nullptr;
  pipelinecore::IMaterialHandle const* g_lightMaterials[3] = {};
  char const* const kBuiltinShaderPrefix = "builtin/shaders/";
}

pipelinecore::IMaterialHandle const* GetPostProcessMaterial(char const* name) {
  if (!name) return nullptr;
  (void)kBuiltinShaderPrefix;
  // Load via 013: path = builtin/shaders/<name>.shader or .material
  // IResource* r = GetResourceManager()->LoadSync(path, ResourceType::Material);
  // return r ? reinterpret_cast<pipelinecore::IMaterialHandle const*>(r) : nullptr;
  return g_postProcessPlaceholder;
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
