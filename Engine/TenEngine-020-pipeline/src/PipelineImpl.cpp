/**
 * @file PipelineImpl.cpp
 * @brief 020-Pipeline: SceneWorldAdapter, ToPipelinecoreFrameContext, CreateDeferredFrameGraph.
 */

#include <te/pipeline/detail/PipelineImpl.h>
#include <te/pipelinecore/Config.h>
#include <te/rhi/command_list.hpp>

namespace te {
namespace pipeline {
namespace detail {

namespace {
/// 占位 Pass 执行回调；实际绘制仍由 ExecuteLogicalCommandBufferOnDeviceThread 统一录制，后续可改为按 Pass 录制
void NoOpPassExecute(pipelinecore::PassContext& /*ctx*/, te::rhi::ICommandList* /*cmd*/) {}
}  // namespace

void ToPipelinecoreFrameContext(pipeline::FrameContext const& ctx,
                                pipelinecore::ISceneWorld const* sceneAdapter,
                                pipelinecore::FrameContext& out) {
  out.scene = sceneAdapter;
  out.camera = ctx.camera;
  out.viewport.width = ctx.viewport.width;
  out.viewport.height = ctx.viewport.height;
  out.frameSlotId = ctx.frameSlotId;
}

pipelinecore::IFrameGraph* CreateDeferredFrameGraph(uint32_t viewportWidth, uint32_t viewportHeight) {
  pipelinecore::IFrameGraph* graph = pipelinecore::CreateFrameGraph();
  if (!graph) return nullptr;

  pipelinecore::IPassBuilder* gbuffer = graph->AddPass("GBuffer", pipelinecore::PassKind::Scene);
  if (gbuffer) {
    gbuffer->SetCullMode(pipelinecore::CullMode::FrustumCull);
    gbuffer->SetRenderType(pipelinecore::RenderType::Opaque);
    pipelinecore::PassOutputDesc out;
    out.width = viewportWidth;
    out.height = viewportHeight;
    gbuffer->SetOutput(out);
    gbuffer->SetExecuteCallback(NoOpPassExecute);
  }

  pipelinecore::IPassBuilder* lighting = graph->AddPass("Lighting", pipelinecore::PassKind::Light);
  if (lighting) {
    lighting->SetCullMode(pipelinecore::CullMode::None);
    lighting->SetRenderType(pipelinecore::RenderType::Opaque);
    pipelinecore::PassOutputDesc out;
    out.width = viewportWidth;
    out.height = viewportHeight;
    lighting->SetOutput(out);
    lighting->SetExecuteCallback(NoOpPassExecute);
  }

  if (!graph->Compile()) {
    pipelinecore::DestroyFrameGraph(graph);
    return nullptr;
  }
  return graph;
}

}  // namespace detail
}  // namespace pipeline
}  // namespace te
