/**
 * @file RenderableCollector.cpp
 * @brief 020-Pipeline: Collect renderables/lights from 029 World to 019 RenderItem/LightItem lists.
 */

#include <te/pipeline/detail/RenderableCollector.h>
#include <te/pipelinecore/RenderItem.h>
#include <te/world/WorldManager.h>
#include <te/world/WorldTypes.h>
#include <te/world/ModelResource.h>
#include <te/world/LightComponent.h>
#include <te/world/CameraComponent.h>
#include <te/world/ReflectionProbeComponent.h>
#include <te/world/DecalComponent.h>
#include <te/scene/ISceneNode.h>
#include <te/resource/ResourceManager.h>
#include <te/resource/MeshResource.h>
#include <te/resource/MaterialResource.h>
#include <te/scene/SceneTypes.h>
#include <te/scene/SpatialQuery.h>
#include <te/scene/ISceneNode.h>
#include <te/mesh/Mesh.h>
#include <te/mesh/MeshResource.h>
#include <te/core/math.h>
#include <te/entity/Entity.h>
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

static pipelinecore::LightType ToPipelineLightType(te::world::LightType t) {
  switch (t) {
    case te::world::LightType::Point: return pipelinecore::LightType::Point;
    case te::world::LightType::Directional: return pipelinecore::LightType::Directional;
    case te::world::LightType::Spot: return pipelinecore::LightType::Spot;
    default: return pipelinecore::LightType::Point;
  }
}

void CollectLightsToLightItemList(te::scene::SceneRef sceneRef,
                                   pipelinecore::ILightItemList* out) {
  if (!out) return;
  out->Clear();
  if (!sceneRef.IsValid()) return;
  te::world::WorldManager& world = te::world::WorldManager::GetInstance();
  world.CollectLights(sceneRef, [out](te::scene::ISceneNode* node, te::world::LightComponent const& comp) {
    pipelinecore::LightItem item;
    item.type = ToPipelineLightType(comp.type);
    item.color[0] = comp.color[0];
    item.color[1] = comp.color[1];
    item.color[2] = comp.color[2];
    item.intensity = comp.intensity;
    item.range = comp.range;
    item.direction[0] = comp.direction[0];
    item.direction[1] = comp.direction[1];
    item.direction[2] = comp.direction[2];
    item.spotAngle = comp.spotAngle;
    if (node) {
      te::core::Matrix4 const& w = node->GetWorldMatrix();
      item.position[0] = w.m[0][3];
      item.position[1] = w.m[1][3];
      item.position[2] = w.m[2][3];
      item.transform = const_cast<te::core::Matrix4*>(&w);
    }
    out->Push(item);
  });
}

static pipelinecore::ReflectionProbeItemType ToPipelineReflectionProbeType(te::world::ReflectionProbeType t) {
  switch (t) {
    case te::world::ReflectionProbeType::Box: return pipelinecore::ReflectionProbeItemType::Box;
    case te::world::ReflectionProbeType::Sphere: return pipelinecore::ReflectionProbeItemType::Sphere;
    default: return pipelinecore::ReflectionProbeItemType::Sphere;
  }
}

void CollectCamerasToCameraItemList(te::scene::SceneRef sceneRef,
                                    pipelinecore::ICameraItemList* out) {
  if (!out) return;
  out->Clear();
  if (!sceneRef.IsValid()) return;
  te::world::WorldManager& world = te::world::WorldManager::GetInstance();
  world.CollectCameras(sceneRef, [out](te::scene::ISceneNode* node, te::world::CameraComponent const& comp) {
    pipelinecore::CameraItem item;
    item.fovY = comp.fovY;
    item.nearZ = comp.nearZ;
    item.farZ = comp.farZ;
    item.isActive = comp.isActive;
    if (node)
      item.transform = const_cast<te::core::Matrix4*>(&node->GetWorldMatrix());
    out->Push(item);
  });
}

void CollectReflectionProbesToReflectionProbeItemList(te::scene::SceneRef sceneRef,
                                                      pipelinecore::IReflectionProbeItemList* out) {
  if (!out) return;
  out->Clear();
  if (!sceneRef.IsValid()) return;
  te::world::WorldManager& world = te::world::WorldManager::GetInstance();
  world.CollectReflectionProbes(sceneRef, [out](te::scene::ISceneNode* node, te::world::ReflectionProbeComponent const& comp) {
    pipelinecore::ReflectionProbeItem item;
    item.type = ToPipelineReflectionProbeType(comp.type);
    item.extent[0] = comp.extent[0];
    item.extent[1] = comp.extent[1];
    item.extent[2] = comp.extent[2];
    item.resolution = comp.resolution;
    if (node)
      item.transform = const_cast<te::core::Matrix4*>(&node->GetWorldMatrix());
    out->Push(item);
  });
}

void CollectDecalsToDecalItemList(te::scene::SceneRef sceneRef,
                                 pipelinecore::IDecalItemList* out) {
  if (!out) return;
  out->Clear();
  if (!sceneRef.IsValid()) return;
  te::world::WorldManager& world = te::world::WorldManager::GetInstance();
  world.CollectDecals(sceneRef, [out](te::scene::ISceneNode* node, te::world::DecalComponent const& comp) {
    pipelinecore::DecalItem item;
    item.albedoTexture = nullptr;  // 020 可后续按 comp.albedoTextureId 解析
    item.size[0] = comp.size[0];
    item.size[1] = comp.size[1];
    item.size[2] = comp.size[2];
    item.blend = comp.blend;
    if (node)
      item.transform = const_cast<te::core::Matrix4*>(&node->GetWorldMatrix());
    out->Push(item);
  });
}

}  // namespace pipeline
}  // namespace te
