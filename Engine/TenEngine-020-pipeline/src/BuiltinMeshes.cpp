/**
 * @file BuiltinMeshes.cpp
 * @brief 020-Pipeline: Thin wrapper delegating to 012-Mesh BuiltinMeshes.
 */
#include <te/pipeline/BuiltinMeshes.h>
#include <te/pipelinecore/RenderItem.h>
#include <te/mesh/BuiltinMeshes.h>

namespace te {
namespace pipeline {

pipelinecore::IMeshHandle const* GetFullscreenQuadMesh() {
  return reinterpret_cast<pipelinecore::IMeshHandle const*>(
      mesh::GetFullscreenQuadMesh());
}

pipelinecore::IMeshHandle const* GetSphereMesh(float radius, uint32_t segments) {
  return reinterpret_cast<pipelinecore::IMeshHandle const*>(
      mesh::GetSphereMesh(radius, segments));
}

pipelinecore::IMeshHandle const* GetConeMesh(float radius, float height, uint32_t segments) {
  return reinterpret_cast<pipelinecore::IMeshHandle const*>(
      mesh::GetConeMesh(radius, height, segments));
}

}  // namespace pipeline
}  // namespace te
