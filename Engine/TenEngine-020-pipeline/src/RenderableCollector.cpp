/**
 * @file RenderableCollector.cpp
 * @brief 020-Pipeline: Stub collect (CPU-only 028/011/012/029; no GPU collection).
 */

#include <te/pipeline/detail/RenderableCollector.h>
#include <te/pipelinecore/RenderItem.h>
#include <te/scene/SceneTypes.h>

namespace te {
namespace pipeline {

void CollectRenderablesToRenderItemList(te::scene::SceneRef /*sceneRef*/,
                                         te::resource::IResourceManager* /*resourceManager*/,
                                         pipelinecore::IRenderItemList* out,
                                         void* /*frustum*/,
                                         float const* /*cameraPositionWorld*/,
                                         te::rhi::IDevice* /*device*/) {
  if (!out) return;
  out->Clear();
}

void CollectLightsToLightItemList(te::scene::SceneRef /*sceneRef*/,
                                   pipelinecore::ILightItemList* out) {
  if (!out) return;
  out->Clear();
}

void CollectCamerasToCameraItemList(te::scene::SceneRef /*sceneRef*/,
                                    pipelinecore::ICameraItemList* out) {
  if (!out) return;
  out->Clear();
}

void CollectReflectionProbesToReflectionProbeItemList(te::scene::SceneRef /*sceneRef*/,
                                                      pipelinecore::IReflectionProbeItemList* out) {
  if (!out) return;
  out->Clear();
}

void CollectDecalsToDecalItemList(te::scene::SceneRef /*sceneRef*/,
                                 pipelinecore::IDecalItemList* out) {
  if (!out) return;
  out->Clear();
}

}  // namespace pipeline
}  // namespace te
