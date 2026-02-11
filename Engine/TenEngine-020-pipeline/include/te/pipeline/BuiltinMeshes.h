/**
 * @file BuiltinMeshes.h
 * @brief 020-Pipeline: Procedural built-in meshes (fullscreen quad, sphere, cone).
 * Ownership: returned pointers are to static/cached resources; do not delete.
 */

#ifndef TE_PIPELINE_BUILTIN_MESHES_H
#define TE_PIPELINE_BUILTIN_MESHES_H

namespace te {
namespace pipelinecore {
struct IMeshHandle;
}
namespace pipeline {

/// Fullscreen quad (NDC -1..1, UV 0..1); for post-process passes
pipelinecore::IMeshHandle const* GetFullscreenQuadMesh();

/// Sphere (radius, segments); for point light volume
pipelinecore::IMeshHandle const* GetSphereMesh(float radius = 1.f, uint32_t segments = 16);

/// Cone (radius, height, segments); for spot light volume
pipelinecore::IMeshHandle const* GetConeMesh(float radius = 1.f, float height = 1.f, uint32_t segments = 16);

}  // namespace pipeline
}  // namespace te

#endif  // TE_PIPELINE_BUILTIN_MESHES_H
