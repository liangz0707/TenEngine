/**
 * @file PassBuilders.cpp
 * @brief 020-Pipeline: Standard render pass implementations.
 */

#include <te/pipeline/PassBuilders.h>
#include <te/pipelinecore/LogicalCommandBuffer.h>
#include <te/rendercore/types.hpp>
#include <cstdio>

namespace te {
namespace pipeline {

pipelinecore::IPassBuilder* CreateGBufferPass(
    pipelinecore::IFrameGraph* fg,
    GBufferPassConfig const& config) {
  if (!fg) return nullptr;

  auto* builder = fg->AddPass(PassNames::GBuffer, pipelinecore::PassKind::Scene);
  if (!builder) return nullptr;

  // Configure output
  pipelinecore::PassOutputDesc output{};
  output.width = config.width;
  output.height = config.height;
  output.colorAttachmentCount = 1;  // Albedo
  if (config.useNormalBuffer) output.colorAttachmentCount = 2;
  if (config.useMaterialBuffer) output.colorAttachmentCount = 3;
  output.useDepthStencil = config.useDepthBuffer;
  builder->SetOutput(output);

  // Set culling mode
  builder->SetCullMode(pipelinecore::CullMode::FrustumCull);
  builder->SetRenderType(pipelinecore::RenderType::Opaque);

  return builder;
}

pipelinecore::IPassBuilder* CreateLightingPass(
    pipelinecore::IFrameGraph* fg,
    LightingPassConfig const& config) {
  if (!fg) return nullptr;

  auto* builder = fg->AddPass(PassNames::Lighting, pipelinecore::PassKind::Light);
  if (!builder) return nullptr;

  // Configure output
  pipelinecore::PassOutputDesc output{};
  output.width = config.width;
  output.height = config.height;
  output.colorAttachmentCount = 1;
  output.useDepthStencil = false;
  builder->SetOutput(output);

  builder->SetContentSource(pipelinecore::PassContentSource::FromLightComponent);

  return builder;
}

pipelinecore::IPassBuilder* CreatePostProcessPass(
    pipelinecore::IFrameGraph* fg,
    PostProcessPassConfig const& config) {
  if (!fg) return nullptr;

  auto* builder = static_cast<pipelinecore::IPostProcessPassBuilder*>(
      fg->AddPass(PassNames::PostProcess, pipelinecore::PassKind::PostProcess));
  if (!builder) return nullptr;

  // Configure output
  pipelinecore::PassOutputDesc output{};
  output.width = config.width;
  output.height = config.height;
  output.colorAttachmentCount = 1;
  output.useDepthStencil = false;
  builder->SetOutput(output);

  builder->SetContentSource(pipelinecore::PassContentSource::FromPassDefined);
  builder->SetFullscreenQuad();

  return builder;
}

pipelinecore::IPassBuilder* CreatePresentPass(pipelinecore::IFrameGraph* fg) {
  if (!fg) return nullptr;

  auto* builder = static_cast<pipelinecore::IPostProcessPassBuilder*>(
      fg->AddPass(PassNames::Present, pipelinecore::PassKind::PostProcess));
  if (!builder) return nullptr;

  builder->SetContentSource(pipelinecore::PassContentSource::FromPassDefined);
  builder->SetFullscreenQuad();
  builder->SetMaterial(MaterialNames::PresentCopy);

  return builder;
}

void ExecuteScenePass(
    pipelinecore::PassContext& ctx,
    rhi::ICommandList* cmd,
    rhi::IDevice* device,
    uint32_t frameSlot,
    pipelinecore::ILogicalCommandBuffer* logicalCb) {
  if (!cmd || !device) return;

  if (logicalCb) {
    // Use pre-built command buffer
    pipelinecore::ExecuteLogicalCommandBufferOnDeviceThread(cmd, logicalCb, device, frameSlot);
  } else {
    // Build on-the-fly from render items
    pipelinecore::IRenderItemList const* items = ctx.GetRenderItemList(0);
    if (!items || items->Size() == 0) return;

    for (size_t i = 0; i < items->Size(); ++i) {
      pipelinecore::RenderItem const* item = items->At(i);
      if (!item || !item->element) continue;

      auto* mesh = item->element->GetMesh();
      auto* mat = item->element->GetMaterial();
      if (!mesh || !mat) continue;

      // Update material
      mat->UpdateDeviceResource(device, frameSlot);

      // Set PSO
      auto* pso = mat->GetGraphicsPSO(0);
      if (pso) cmd->SetGraphicsPSO(pso);

      // Bind descriptor set
      auto* descSet = mat->GetDescriptorSet();
      if (descSet) cmd->BindDescriptorSet(0u, descSet);

      // Set vertex buffer
      auto* vb = mesh->GetVertexBuffer();
      if (vb) cmd->SetVertexBuffer(0, vb, 0, 32);  // TODO: get stride from mesh

      // Set index buffer
      auto* ib = mesh->GetIndexBuffer();
      if (ib) cmd->SetIndexBuffer(ib, 0, 1);  // TODO: get format from mesh

      // Get submesh range and draw
      rendercore::SubmeshRange range;
      if (mesh->GetSubmesh(item->submeshIndex, &range)) {
        cmd->DrawIndexed(range.indexCount, item->instanceCount > 0 ? item->instanceCount : 1,
                         range.indexOffset, static_cast<int32_t>(range.vertexOffset), 0);
      }
    }
  }
}

void ExecuteLightingPass(
    pipelinecore::PassContext& ctx,
    rhi::ICommandList* cmd,
    rhi::IDevice* /*device*/) {
  if (!cmd) return;

  // Get light items
  auto* lights = ctx.GetLightItemList();
  if (!lights || lights->Size() == 0) {
    // No lights - skip or use ambient only
    return;
  }

  // For deferred lighting, we would:
  // 1. Bind GBuffer textures as inputs
  // 2. Draw a fullscreen quad for each light type
  // 3. Accumulate lighting contributions

  // Placeholder: simple ambient pass
  // In a real implementation, this would use light clustering, tiled deferred, etc.
}

void ExecutePostProcessPass(
    pipelinecore::PassContext& /*ctx*/,
    rhi::ICommandList* cmd,
    rhi::IDevice* /*device*/,
    char const* /*materialName*/) {
  if (!cmd) return;

  // For post-process, we draw a fullscreen quad
  // The material should be bound by the frame graph execute callback

  // Simple fullscreen triangle/quad draw
  // In real implementation, this would use the PostProcess material's PSO and textures
  cmd->Draw(3, 1, 0, 0);  // Fullscreen triangle
}

}  // namespace pipeline
}  // namespace te
