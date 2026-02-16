/**
 * @file RenderPipeline.cpp
 * @brief 020-Pipeline: RenderPipeline implementation.
 */

#include <te/pipeline/RenderPipeline.h>
#include <te/pipelinecore/FrameContext.h>
#include <te/rendercore/types.hpp>
#include <chrono>
#include <cstdio>

namespace te {
namespace pipeline {

RenderPipeline::RenderPipeline() = default;

RenderPipeline::~RenderPipeline() {
  Shutdown();
}

bool RenderPipeline::Initialize(rhi::IDevice* device, rhi::ISwapChain* swapChain) {
  if (!device || !swapChain) {
    CheckError("RenderPipeline::Initialize: device or swapChain is null");
    return false;
  }

  device_ = device;
  swapChain_ = swapChain;

  // Get graphics queue
  graphicsQueue_ = device_->GetQueue(rhi::QueueType::Graphics, 0);
  if (!graphicsQueue_) {
    CheckError("RenderPipeline::Initialize: failed to get graphics queue");
    return false;
  }

  // Create command list
  commandList_ = device_->CreateCommandList();
  if (!commandList_) {
    CheckError("RenderPipeline::Initialize: failed to create command list");
    return false;
  }

  // Create frame graph
  frameGraph_ = pipelinecore::CreateFrameGraph();
  if (!frameGraph_) {
    CheckError("RenderPipeline::Initialize: failed to create frame graph");
    return false;
  }

  // Create render item lists
  renderItemList_ = pipelinecore::CreateRenderItemList();
  lightItemList_ = pipelinecore::CreateLightItemList();
  if (!renderItemList_ || !lightItemList_) {
    CheckError("RenderPipeline::Initialize: failed to create item lists");
    return false;
  }

  initialized_ = true;
  CheckWarning("RenderPipeline initialized successfully");
  return true;
}

void RenderPipeline::Shutdown() {
  if (!initialized_) return;

  if (logicalCommandBuffer_) {
    pipelinecore::DestroyLogicalCommandBuffer(logicalCommandBuffer_);
    logicalCommandBuffer_ = nullptr;
  }

  if (logicalPipeline_) {
    pipelinecore::DestroyLogicalPipeline(logicalPipeline_);
    logicalPipeline_ = nullptr;
  }

  if (frameGraph_) {
    pipelinecore::DestroyFrameGraph(frameGraph_);
    frameGraph_ = nullptr;
  }

  if (renderItemList_) {
    pipelinecore::DestroyRenderItemList(renderItemList_);
    renderItemList_ = nullptr;
  }

  if (lightItemList_) {
    pipelinecore::DestroyLightItemList(lightItemList_);
    lightItemList_ = nullptr;
  }

  if (commandList_ && device_) {
    device_->DestroyCommandList(commandList_);
    commandList_ = nullptr;
  }

  device_ = nullptr;
  swapChain_ = nullptr;
  graphicsQueue_ = nullptr;
  initialized_ = false;
}

void RenderPipeline::SetConfig(RenderingConfig const& config) {
  config_ = config;
}

void RenderPipeline::SetCollectRenderablesCallback(CollectRenderablesCallback callback) {
  collectCallback_ = std::move(callback);
}

pipelinecore::IPassBuilder* RenderPipeline::AddPass(char const* name) {
  if (!frameGraph_) return nullptr;
  return frameGraph_->AddPass(name);
}

pipelinecore::IPassBuilder* RenderPipeline::AddPass(char const* name, pipelinecore::PassKind kind) {
  if (!frameGraph_) return nullptr;
  return frameGraph_->AddPass(name, kind);
}

bool RenderPipeline::CompileFrameGraph() {
  if (!frameGraph_) {
    CheckError("RenderPipeline::CompileFrameGraph: frame graph is null");
    return false;
  }

  // If no passes were added, create default frame graph
  if (frameGraph_->GetPassCount() == 0) {
    CreateDefaultFrameGraph();
  }

  bool success = frameGraph_->Compile();
  if (!success) {
    CheckError("RenderPipeline::CompileFrameGraph: failed to compile frame graph");
    return false;
  }

  CheckWarning("Frame graph compiled successfully");
  return true;
}

void RenderPipeline::RenderFrame() {
  if (!initialized_) {
    CheckError("RenderPipeline::RenderFrame: not initialized");
    return;
  }

  auto startTime = std::chrono::high_resolution_clock::now();

  // Phase A: Build logical pipeline and collect renderables
  BuildAndCollect();

  // Phase B: Prepare device resources
  PrepareResources();

  // Phase C: Execute passes and record commands
  ExecutePasses();

  // Phase D: Submit and present
  SubmitAndPresent();

  // Update frame stats
  auto endTime = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
  frameStats_.frameTimeMs = static_cast<float>(duration.count()) / 1000.0f;
  frameStats_.frameNumber++;
}

void RenderPipeline::BuildAndCollect() {
  // Build logical pipeline from frame graph
  if (frameGraph_) {
    pipelinecore::FrameContext ctx;
    ctx.viewport.width = swapChain_ ? swapChain_->GetWidth() : 800;
    ctx.viewport.height = swapChain_ ? swapChain_->GetHeight() : 600;
    ctx.frameSlotId = currentFrameSlot_;

    if (logicalPipeline_) {
      pipelinecore::DestroyLogicalPipeline(logicalPipeline_);
    }
    logicalPipeline_ = pipelinecore::BuildLogicalPipeline(frameGraph_, ctx);
  }

  // Collect renderables from scene via callback
  if (renderItemList_) {
    renderItemList_->Clear();
  }
  if (lightItemList_) {
    lightItemList_->Clear();
  }

  if (collectCallback_ && renderItemList_ && lightItemList_) {
    collectCallback_(renderItemList_, lightItemList_);
  }

  frameStats_.visibleObjectCount = renderItemList_ ? static_cast<uint32_t>(renderItemList_->Size()) : 0;
}

void RenderPipeline::PrepareResources() {
  if (!device_ || !renderItemList_) return;

  // Prepare render resources (create/update GPU resources)
  pipelinecore::PrepareRenderResources(renderItemList_, device_);
}

void RenderPipeline::ExecutePasses() {
  if (!frameGraph_ || !commandList_ || !device_) return;

  // Begin command list
  commandList_->Begin();

  // Get back buffer for rendering
  rhi::ITexture* backBuffer = swapChain_ ? swapChain_->GetCurrentBackBuffer() : nullptr;
  uint32_t width = swapChain_ ? swapChain_->GetWidth() : 800;
  uint32_t height = swapChain_ ? swapChain_->GetHeight() : 600;

  // Set viewport
  rhi::Viewport viewport{0.f, 0.f, static_cast<float>(width), static_cast<float>(height), 0.f, 1.f};
  commandList_->SetViewport(0, 1, &viewport);

  // Set scissor
  rhi::ScissorRect scissor{0, 0, width, height};
  commandList_->SetScissor(0, 1, &scissor);

  // Begin render pass with back buffer
  rhi::RenderPassDesc rpDesc{};
  rpDesc.colorAttachmentCount = 1;
  rpDesc.colorAttachments[0].texture = backBuffer;
  rpDesc.colorAttachments[0].loadOp = rhi::LoadOp::Clear;
  rpDesc.colorAttachments[0].storeOp = rhi::StoreOp::Store;
  rpDesc.colorAttachments[0].clearColor[0] = 0.1f;
  rpDesc.colorAttachments[0].clearColor[1] = 0.1f;
  rpDesc.colorAttachments[0].clearColor[2] = 0.2f;
  rpDesc.colorAttachments[0].clearColor[3] = 1.0f;
  rpDesc.depthStencilAttachment.texture = nullptr;
  rpDesc.colorLoadOp = rhi::LoadOp::Clear;
  rpDesc.colorStoreOp = rhi::StoreOp::Store;
  rpDesc.subpassCount = 0;

  commandList_->BeginRenderPass(rpDesc, nullptr);

  // Execute each pass
  size_t passCount = frameGraph_->GetPassCount();
  for (size_t i = 0; i < passCount; ++i) {
    pipelinecore::PassContext ctx;

    // For scene passes, collect and convert to logical command buffer
    pipelinecore::PassCollectConfig passConfig;
    frameGraph_->GetPassCollectConfig(i, &passConfig);

    if (passConfig.passKind == pipelinecore::PassKind::Scene) {
      // Convert render items to logical command buffer
      if (logicalCommandBuffer_) {
        pipelinecore::DestroyLogicalCommandBuffer(logicalCommandBuffer_);
        logicalCommandBuffer_ = nullptr;
      }

      if (renderItemList_ && logicalPipeline_) {
        pipelinecore::ConvertToLogicalCommandBuffer(renderItemList_, logicalPipeline_, &logicalCommandBuffer_);
        ctx.SetRenderItemList(0, renderItemList_);
      }

      // Execute logical command buffer
      if (logicalCommandBuffer_) {
        pipelinecore::ExecuteLogicalCommandBufferOnDeviceThread(commandList_, logicalCommandBuffer_, device_, currentFrameSlot_);
        frameStats_.drawCallCount = static_cast<uint32_t>(logicalCommandBuffer_->GetDrawCount());
      }
    } else {
      // For other pass types, call the frame graph's ExecutePass
      frameGraph_->ExecutePass(i, ctx, commandList_);
    }
  }

  commandList_->EndRenderPass();
  commandList_->End();
}

void RenderPipeline::SubmitAndPresent() {
  if (!commandList_ || !graphicsQueue_) return;

  // Submit command buffer
  pipelinecore::SubmitLogicalCommandBuffer(commandList_, graphicsQueue_);

  // Present
  if (swapChain_) {
    pipelinecore::PresentSwapChain(swapChain_);
  }

  // Advance frame slot
  currentFrameSlot_ = (currentFrameSlot_ + 1) % config_.frameSlotCount;
}

void RenderPipeline::CreateDefaultFrameGraph() {
  // Create a simple default frame graph: GBuffer -> Lighting -> Present

  // Pass 1: GBuffer (scene geometry)
  auto* gbufferPass = AddPass("GBuffer", pipelinecore::PassKind::Scene);
  if (gbufferPass) {
    pipelinecore::PassOutputDesc output{};
    output.width = 1280;
    output.height = 720;
    output.colorAttachmentCount = 1;
    output.useDepthStencil = true;
    gbufferPass->SetOutput(output);
    gbufferPass->SetCullMode(pipelinecore::CullMode::FrustumCull);
    gbufferPass->SetRenderType(pipelinecore::RenderType::Opaque);
  }

  // Pass 2: Lighting
  auto* lightingPass = AddPass("Lighting", pipelinecore::PassKind::Light);
  if (lightingPass) {
    lightingPass->SetContentSource(pipelinecore::PassContentSource::FromLightComponent);
  }

  // Pass 3: PostProcess
  auto* postProcessPass = AddPass("PostProcess", pipelinecore::PassKind::PostProcess);
  if (postProcessPass) {
    postProcessPass->SetContentSource(pipelinecore::PassContentSource::FromPassDefined);
    auto* ppBuilder = static_cast<pipelinecore::IPostProcessPassBuilder*>(postProcessPass);
    if (ppBuilder) {
      ppBuilder->SetFullscreenQuad();
    }
  }

  CheckWarning("Created default frame graph with GBuffer, Lighting, and PostProcess passes");
}

void RenderPipeline::CheckWarning(char const* message) {
  CheckWarningGlobal(config_, message);
}

void RenderPipeline::CheckError(char const* message) {
  CheckErrorGlobal(config_, message);
}

void CheckWarningGlobal(RenderingConfig const& config, char const* message) {
  if (config.validationLevel >= RenderingConfig::ValidationLevel::Hybrid) {
    std::printf("[RenderPipeline Warning] %s\n", message);
  }
}

void CheckErrorGlobal(RenderingConfig const& config, char const* message) {
  if (config.validationLevel >= RenderingConfig::ValidationLevel::Debug) {
    std::printf("[RenderPipeline Error] %s\n", message);
  }
}

}  // namespace pipeline
}  // namespace te
