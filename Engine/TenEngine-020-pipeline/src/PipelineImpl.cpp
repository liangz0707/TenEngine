/**
 * @file PipelineImpl.cpp
 * @brief 020-Pipeline: SceneWorldAdapter, ToPipelinecoreFrameContext, CreateDeferredFrameGraph.
 */

#include <te/pipeline/detail/PipelineImpl.h>
#include <te/pipelinecore/Config.h>
#include <te/pipelinecore/FrameGraph.h>
#include <te/rendercore/types.hpp>
#include <te/rhi/command_list.hpp>
#include <cstdint>

namespace te {
namespace pipeline {
namespace detail {

namespace {
/// 占位 Pass 执行回调；实际绘制仍由 ExecuteLogicalCommandBufferOnDeviceThread 或各 Pass 内逻辑录制
void NoOpPassExecute(pipelinecore::PassContext& /*ctx*/, te::rhi::ICommandList* /*cmd*/) {}

/// Deferred FrameGraph 资源 ID：0=BackBuffer，1=BaseColor，2=Normal，3=PBR，4=Depth，5=LightingResult
constexpr uint64_t kDeferredResIdGBufferBaseColor = 1u;
constexpr uint64_t kDeferredResIdGBufferNormal = 2u;
constexpr uint64_t kDeferredResIdGBufferPBR = 3u;
constexpr uint64_t kDeferredResIdGBufferDepth = 4u;
constexpr uint64_t kDeferredResIdLightingResult = 5u;

te::rendercore::ResourceHandle MakeHandle(uint64_t id) {
  te::rendercore::ResourceHandle h;
  h.id = id;
  return h;
}

/// 填充 PassAttachmentDesc 为 RT（无 sourcePass）
void FillAttachmentDesc(pipelinecore::PassAttachmentDesc& desc,
                       uint64_t handleId,
                       uint32_t w,
                       uint32_t h,
                       uint32_t format,
                       bool isDepthStencil,
                       pipelinecore::AttachmentLoadOp loadOp,
                       pipelinecore::AttachmentStoreOp storeOp) {
  desc.handle = MakeHandle(handleId);
  desc.width = w;
  desc.height = h;
  desc.format = format;
  desc.isDepthStencil = isDepthStencil;
  desc.loadOp = loadOp;
  desc.storeOp = storeOp;
  desc.sourcePassIndex = static_cast<size_t>(-1);
  desc.sourceAttachmentIndex = 0;
}

void ConfigureGBufferPass(pipelinecore::IScenePassBuilder* gbuffer, uint32_t vpW, uint32_t vpH) {
  if (!gbuffer) return;
  gbuffer->SetPassKind(pipelinecore::PassKind::Scene);
  gbuffer->SetContentSource(pipelinecore::PassContentSource::FromModelComponent);
  gbuffer->SetCullMode(pipelinecore::CullMode::FrustumCull);
  gbuffer->SetRenderType(pipelinecore::RenderType::Opaque);

  pipelinecore::PassAttachmentDesc colorAtt;
  FillAttachmentDesc(colorAtt, kDeferredResIdGBufferBaseColor, vpW, vpH, 0u, false,
                     pipelinecore::AttachmentLoadOp::Clear, pipelinecore::AttachmentStoreOp::StoreOp_Store);
  colorAtt.lifetime = pipelinecore::AttachmentLifetime::Persistent;
  gbuffer->AddColorAttachment(colorAtt);
  FillAttachmentDesc(colorAtt, kDeferredResIdGBufferNormal, vpW, vpH, 0u, false,
                     pipelinecore::AttachmentLoadOp::Clear, pipelinecore::AttachmentStoreOp::StoreOp_Store);
  colorAtt.lifetime = pipelinecore::AttachmentLifetime::Persistent;
  gbuffer->AddColorAttachment(colorAtt);
  FillAttachmentDesc(colorAtt, kDeferredResIdGBufferPBR, vpW, vpH, 0u, false,
                     pipelinecore::AttachmentLoadOp::Clear, pipelinecore::AttachmentStoreOp::StoreOp_Store);
  colorAtt.lifetime = pipelinecore::AttachmentLifetime::Persistent;
  gbuffer->AddColorAttachment(colorAtt);

  pipelinecore::PassAttachmentDesc depthAtt;
  FillAttachmentDesc(depthAtt, kDeferredResIdGBufferDepth, vpW, vpH, 0u, true,
                     pipelinecore::AttachmentLoadOp::Clear, pipelinecore::AttachmentStoreOp::StoreOp_Store);
  depthAtt.lifetime = pipelinecore::AttachmentLifetime::Persistent;
  gbuffer->SetDepthStencilAttachment(depthAtt);

  gbuffer->DeclareWrite(MakeHandle(kDeferredResIdGBufferBaseColor));
  gbuffer->DeclareWrite(MakeHandle(kDeferredResIdGBufferNormal));
  gbuffer->DeclareWrite(MakeHandle(kDeferredResIdGBufferPBR));
  gbuffer->DeclareWrite(MakeHandle(kDeferredResIdGBufferDepth));

  pipelinecore::PassOutputDesc out;
  out.width = vpW;
  out.height = vpH;
  out.colorAttachmentCount = 3u;
  out.useDepthStencil = true;
  gbuffer->SetOutput(out);
  gbuffer->SetExecuteCallback(NoOpPassExecute);
}

void ConfigureLightingPass(pipelinecore::ILightPassBuilder* lighting, uint32_t vpW, uint32_t vpH) {
  if (!lighting) return;
  lighting->SetPassKind(pipelinecore::PassKind::Light);
  lighting->SetContentSource(pipelinecore::PassContentSource::FromLightComponent);
  lighting->SetCullMode(pipelinecore::CullMode::None);
  lighting->SetRenderType(pipelinecore::RenderType::Opaque);

  lighting->DeclareRead(MakeHandle(kDeferredResIdGBufferBaseColor));
  lighting->DeclareRead(MakeHandle(kDeferredResIdGBufferNormal));
  lighting->DeclareRead(MakeHandle(kDeferredResIdGBufferPBR));
  lighting->DeclareRead(MakeHandle(kDeferredResIdGBufferDepth));
  lighting->DeclareWrite(MakeHandle(kDeferredResIdLightingResult));

  pipelinecore::PassAttachmentDesc lightingResultAtt;
  FillAttachmentDesc(lightingResultAtt, kDeferredResIdLightingResult, vpW, vpH, 0u, false,
                     pipelinecore::AttachmentLoadOp::Clear, pipelinecore::AttachmentStoreOp::StoreOp_Store);
  lighting->AddColorAttachment(lightingResultAtt);

  pipelinecore::PassOutputDesc out;
  out.width = vpW;
  out.height = vpH;
  out.colorAttachmentCount = 1u;
  out.useDepthStencil = false;
  lighting->SetOutput(out);
  lighting->SetExecuteCallback(NoOpPassExecute);
}

void ConfigureColorGradingPass(pipelinecore::IPostProcessPassBuilder* colorGrading, uint32_t vpW, uint32_t vpH) {
  if (!colorGrading) return;
  colorGrading->SetPassKind(pipelinecore::PassKind::PostProcess);
  colorGrading->SetContentSource(pipelinecore::PassContentSource::FromPassDefined);
  colorGrading->SetCullMode(pipelinecore::CullMode::None);
  colorGrading->SetRenderType(pipelinecore::RenderType::Opaque);

  colorGrading->DeclareRead(MakeHandle(kDeferredResIdLightingResult));
  colorGrading->DeclareWrite(MakeHandle(pipelinecore::kResourceHandleIdBackBuffer));

  pipelinecore::PassAttachmentDesc backBufferAtt;
  FillAttachmentDesc(backBufferAtt, pipelinecore::kResourceHandleIdBackBuffer, vpW, vpH, 0u, false,
                     pipelinecore::AttachmentLoadOp::Clear, pipelinecore::AttachmentStoreOp::StoreOp_Store);
  colorGrading->AddColorAttachment(backBufferAtt);

  pipelinecore::PassOutputDesc out;
  out.width = vpW;
  out.height = vpH;
  out.colorAttachmentCount = 1u;
  out.useDepthStencil = false;
  colorGrading->SetOutput(out);
  colorGrading->SetExecuteCallback(NoOpPassExecute);

  colorGrading->SetMaterial("color_grading");
  colorGrading->SetFullscreenQuad();
}

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

  pipelinecore::IScenePassBuilder* gbuffer =
      static_cast<pipelinecore::IScenePassBuilder*>(graph->AddPass("GBuffer", pipelinecore::PassKind::Scene));
  ConfigureGBufferPass(gbuffer, viewportWidth, viewportHeight);

  pipelinecore::ILightPassBuilder* lighting =
      static_cast<pipelinecore::ILightPassBuilder*>(graph->AddPass("Lighting", pipelinecore::PassKind::Light));
  ConfigureLightingPass(lighting, viewportWidth, viewportHeight);

  pipelinecore::IPostProcessPassBuilder* colorGrading =
      static_cast<pipelinecore::IPostProcessPassBuilder*>(graph->AddPass("ColorGrading", pipelinecore::PassKind::PostProcess));
  ConfigureColorGradingPass(colorGrading, viewportWidth, viewportHeight);

  if (!graph->Compile()) {
    pipelinecore::DestroyFrameGraph(graph);
    return nullptr;
  }
  return graph;
}

}  // namespace detail
}  // namespace pipeline
}  // namespace te
