/**
 * @file BuiltinMaterials.h
 * @brief 020-Pipeline: Built-in materials for post-process and light passes.
 * Shader source: engine asset files under builtin/ (e.g. builtin/shaders/postprocess.shader).
 */

#ifndef TE_PIPELINE_BUILTIN_MATERIALS_H
#define TE_PIPELINE_BUILTIN_MATERIALS_H

namespace te {
namespace resource {
class IResourceManager;
}
namespace pipelinecore {
struct IMaterialHandle;
}
namespace pipeline {

/// Optional: set resource manager so GetPostProcessMaterial can load from builtin/shaders/<name>.material
void SetBuiltinMaterialResourceManager(te::resource::IResourceManager* manager);

/// Post-process material by name (e.g. "color_grading"). Loads from builtin/shaders/<name>.material when manager set
pipelinecore::IMaterialHandle const* GetPostProcessMaterial(char const* name);

/// Light pass material by type (point, directional, spot). Loads from builtin/shaders/light_<type>.shader
pipelinecore::IMaterialHandle const* GetLightMaterial(unsigned lightTypeIndex);

}  // namespace pipeline
}  // namespace te

#endif  // TE_PIPELINE_BUILTIN_MATERIALS_H
