/**
 * @file RenderableCollector.h
 * @brief 020-Pipeline: Collect 029 renderables into 019 IRenderItemList (only when loaded).
 */

#ifndef TE_PIPELINE_DETAIL_RENDERABLE_COLLECTOR_H
#define TE_PIPELINE_DETAIL_RENDERABLE_COLLECTOR_H

namespace te {
namespace scene {
struct SceneRef;
}
namespace resource {
struct IResourceManager;
}
namespace pipelinecore {
struct IRenderItemList;
}
namespace pipeline {

/// frustum 可选；非空时视为 te::scene::Frustum const*，对提供 HasAABB/GetAABB 的节点做视锥剔除
/// cameraPositionWorld 可选；非空时视为 float[3] 世界坐标，用于 012 SelectLOD 按距离选 LOD；仅对选中 LOD 的 submesh 生成 RenderItem
void CollectRenderablesToRenderItemList(te::scene::SceneRef sceneRef,
                                         te::resource::IResourceManager* resourceManager,
                                         pipelinecore::IRenderItemList* out,
                                         void* frustum = nullptr,
                                         float const* cameraPositionWorld = nullptr);
}
}

#endif  // TE_PIPELINE_DETAIL_RENDERABLE_COLLECTOR_H
