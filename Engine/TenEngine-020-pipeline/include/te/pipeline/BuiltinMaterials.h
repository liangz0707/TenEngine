/**
 * @file BuiltinMaterials.h
 * @brief 020-Pipeline: Built-in materials for post-process and light passes.
 * Shader source: engine asset files under builtin/ (e.g. builtin/shaders/postprocess.shader).
 */

#ifndef TE_PIPELINE_BUILTIN_MATERIALS_H
#define TE_PIPELINE_BUILTIN_MATERIALS_H

namespace te {
namespace pipelinecore {
struct IMaterialHandle;
}
namespace pipeline {

/// Post-process material by name (e.g. "blit", "tonemap"). Loads from builtin/shaders/<name>.shader / .material
pipelinecore::IMaterialHandle const* GetPostProcessMaterial(char const* name);

/// Light pass material by type (point, directional, spot). Loads from builtin/shaders/light_<type>.shader
pipelinecore::IMaterialHandle const* GetLightMaterial(unsigned lightTypeIndex);

}  // namespace pipeline
}  // namespace te

#endif  // TE_PIPELINE_BUILTIN_MATERIALS_H
