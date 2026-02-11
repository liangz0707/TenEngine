/**
 * @file RenderableCollector.h
 * @brief 020-Pipeline: Collect 029 renderables into 019 IRenderItemList (all from ModelComponent).
 * Each collected item is one RenderElement (mesh+material+submesh); during collection we set
 * element data: submeshIndex, transform, bounds on the RenderItem.
 */

#ifndef TE_PIPELINE_DETAIL_RENDERABLE_COLLECTOR_H
#define TE_PIPELINE_DETAIL_RENDERABLE_COLLECTOR_H

#include <te/scene/SceneTypes.h>

namespace te {
namespace resource {
struct IResourceManager;
}
namespace rhi {
struct IDevice;
}
namespace pipelinecore {
struct IRenderItemList;
struct ILightItemList;
struct ICameraItemList;
struct IReflectionProbeItemList;
struct IDecalItemList;
}
namespace pipeline {

/// frustum 可选；非空时视为 te::scene::Frustum const*，对提供 HasAABB/GetAABB 的节点做视锥剔除
/// cameraPositionWorld 可选；非空时视为 float[3] 世界坐标，用于 012 SelectLOD 按距离选 LOD；仅对选中 LOD 的 submesh 生成 RenderItem
/// CPU-only: list is cleared only (no scene collection).
void CollectRenderablesToRenderItemList(te::scene::SceneRef sceneRef,
                                         te::resource::IResourceManager* resourceManager,
                                         pipelinecore::IRenderItemList* out,
                                         void* frustum = nullptr,
                                         float const* cameraPositionWorld = nullptr,
                                         te::rhi::IDevice* device = nullptr);

/// 收集场景内灯光到 ILightItemList；需 029 LightComponent，无 Light 时仅清空 out
void CollectLightsToLightItemList(te::scene::SceneRef sceneRef,
                                   pipelinecore::ILightItemList* out);

/// 收集场景内相机到 ICameraItemList；需 029 CameraComponent
void CollectCamerasToCameraItemList(te::scene::SceneRef sceneRef,
                                    pipelinecore::ICameraItemList* out);

/// 收集场景内反射探针到 IReflectionProbeItemList；需 029 ReflectionProbeComponent
void CollectReflectionProbesToReflectionProbeItemList(te::scene::SceneRef sceneRef,
                                                      pipelinecore::IReflectionProbeItemList* out);

/// 收集场景内贴花到 IDecalItemList；需 029 DecalComponent；albedoTexture 暂不解析，填 nullptr
void CollectDecalsToDecalItemList(te::scene::SceneRef sceneRef,
                                 pipelinecore::IDecalItemList* out);
}
}

#endif  // TE_PIPELINE_DETAIL_RENDERABLE_COLLECTOR_H
