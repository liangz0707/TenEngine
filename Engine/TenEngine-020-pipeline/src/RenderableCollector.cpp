/**
 * @file RenderableCollector.cpp
 * @brief 020-Pipeline: Collect renderables from 029 World, convert to 019 RenderItem (only when loaded).
 */

#include <te/pipeline/detail/RenderableCollector.h>
#include <te/pipelinecore/RenderItem.h>
#include <te/world/WorldManager.h>
#include <te/world/WorldTypes.h>
#include <te/world/ModelResource.h>
#include <te/resource/ResourceManager.h>
#include <te/resource/MeshResource.h>
#include <te/resource/MaterialResource.h>
#include <te/scene/SceneTypes.h>
#include <te/scene/SpatialQuery.h>
#include <te/scene/ISceneNode.h>
#include <te/mesh/Mesh.h>
#include <te/mesh/MeshResource.h>
#include <te/core/math.h>
#include <cstdint>
#include <cmath>

namespace te {
namespace pipeline {

static float distance3(float const* a, te::core::Vector3 const& b) {
  float dx = a[0] - b.x, dy = a[1] - b.y, dz = a[2] - b.z;
  return std::sqrt(dx*dx + dy*dy + dz*dz);
}

/// 仅当 modelResource 已加载（非空）时转为 RenderItem；mesh/material 句柄由 011/012 提供
/// frustum 非空时视锥剔除；cameraPositionWorld 非空时调用 012 SelectLOD，仅对选中 LOD 的 submesh 生成 RenderItem
void CollectRenderablesToRenderItemList(te::scene::SceneRef sceneRef,
                                         te::resource::IResourceManager* resourceManager,
                                         pipelinecore::IRenderItemList* out,
                                         void* frustum,
                                         float const* cameraPositionWorld) {
  if (!out) return;
  out->Clear();

  te::scene::Frustum const* fp = frustum ? static_cast<te::scene::Frustum const*>(frustum) : nullptr;
  te::world::WorldManager& world = te::world::WorldManager::GetInstance();
  world.CollectRenderables(sceneRef, resourceManager,
    [out, fp, cameraPositionWorld](te::scene::ISceneNode* node, te::world::RenderableItem const& item) {
      if (fp && node && node->HasAABB()) {
        if (!te::scene::SpatialQuery::FrustumIntersectsAABB(*fp, node->GetAABB()))
          return;
      }
      if (!item.modelResource) return;
      te::world::IModelResource* model = static_cast<te::world::IModelResource*>(item.modelResource);
      te::resource::IMeshResource* meshRes = model->GetMesh();
      if (!meshRes) return;
      te::mesh::MeshResource const* mesh012 = dynamic_cast<te::mesh::MeshResource const*>(meshRes);
      te::mesh::MeshHandle mh = mesh012 ? mesh012->GetMeshHandle() : te::mesh::MeshHandle{};
      if (mh) {
        uint32_t lodCount = te::mesh::GetLODCount(mh);
        if (lodCount > 0u) {
          float dist = 0.f;
          if (cameraPositionWorld && node)
            dist = distance3(cameraPositionWorld, node->GetWorldTransform().position);
          uint32_t selectedLod = te::mesh::SelectLOD(mh, dist);
          te::mesh::LODLevel level;
          if (te::mesh::GetLODLevel(mh, selectedLod, &level)) {
            if (item.submeshIndex < level.submeshStartIndex ||
                item.submeshIndex >= level.submeshStartIndex + level.submeshCount)
              return;
          }
        }
      }
      std::size_t matCount = model->GetMaterialCount();
      if (matCount == 0) return;
      uint32_t matIndex = model->GetSubmeshMaterialIndex(item.submeshIndex);
      if (matIndex >= static_cast<uint32_t>(matCount)) return;
      te::resource::IMaterialResource* matRes = model->GetMaterial(static_cast<std::size_t>(matIndex));
      if (!matRes) return;

      pipelinecore::RenderItem ri;
      ri.mesh = reinterpret_cast<pipelinecore::IMeshHandle const*>(meshRes);
      ri.material = reinterpret_cast<pipelinecore::IMaterialHandle const*>(matRes);
      ri.sortKey = (static_cast<uint64_t>(matIndex) << 32u) | item.submeshIndex;
      out->Push(ri);
    });
}

}  // namespace pipeline
}  // namespace te
