/**
 * @file PipelineContext.cpp
 * @brief Implementation of PipelineContext.
 */

#include <te/pipeline/PipelineContext.h>
#include <te/pipeline/LogicalCommandBufferExecutor.h>

#include <te/pipelinecore/FrameGraph.h>
#include <te/pipelinecore/LogicalPipeline.h>
#include <te/pipelinecore/RenderItem.h>
#include <te/pipelinecore/LogicalCommandBuffer.h>
#include <te/pipelinecore/ResourceManager.h>
#include <te/pipelinecore/SubmitContext.h>
#include <te/rhi/device.hpp>
#include <te/rhi/command_list.hpp>
#include <te/rhi/swapchain.hpp>

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstring>

namespace te::pipeline {

// === PipelineContext::Impl ===

struct RenderBatchData {
  // Placeholder for batch data
  uint32_t startVertex{0};
  uint32_t vertexCount{0};
  uint32_t startIndex{0};
  uint32_t indexCount{0};
  uint32_t instanceCount{1};
};

struct PipelineContext::Impl {
  rhi::IDevice* device{nullptr};
  RenderingConfig const* config{nullptr};
  pipelinecore::IFrameGraph* frameGraph{nullptr};
  rhi::ISwapChain* swapChain{nullptr};

  pipelinecore::FrameContext frameCtx;
  pipelinecore::ILogicalPipeline* logicalPipeline{nullptr};
  pipelinecore::ILogicalCommandBuffer* logicalCB{nullptr};

  std::unique_ptr<pipelinecore::TransientResourcePool> resourcePool;
  std::unique_ptr<pipelinecore::SubmitContext> submitCtx;

  std::vector<pipelinecore::IRenderItemList*> renderItemsPerPass;
  pipelinecore::ILightItemList* lights{nullptr};
  pipelinecore::ICameraItemList* cameras{nullptr};

  RenderTarget renderTarget;
  std::vector<RenderBatchData> batches;

  FrameStats stats;
  uint64_t frameIndex{0};

  bool resourcesReady{false};

  void Reset() {
    logicalPipeline = nullptr;
    logicalCB = nullptr;
    resourcesReady = false;

    for (auto* list : renderItemsPerPass) {
      if (list) {
        list->Clear();
      }
    }
    if (lights) lights->Clear();
    if (cameras) cameras->Clear();

    batches.clear();
    stats = FrameStats{};
    stats.frameIndex = frameIndex;
  }
};

// === PipelineContext ===

PipelineContext::PipelineContext()
  : impl_(std::make_unique<Impl>()) {
  impl_->resourcePool = std::unique_ptr<pipelinecore::TransientResourcePool>(
    pipelinecore::CreateTransientResourcePool());
  impl_->submitCtx = std::unique_ptr<pipelinecore::SubmitContext>(
    pipelinecore::CreateSubmitContext());

  impl_->renderItemsPerPass.resize(8); // Reserve for 8 passes
  for (auto& list : impl_->renderItemsPerPass) {
    list = pipelinecore::CreateRenderItemList();
  }
  impl_->lights = pipelinecore::CreateLightItemList();
  impl_->cameras = pipelinecore::CreateCameraItemList();
}

PipelineContext::~PipelineContext() {
  for (auto* list : impl_->renderItemsPerPass) {
    if (list) {
      pipelinecore::DestroyRenderItemList(list);
    }
  }
  if (impl_->lights) {
    pipelinecore::DestroyLightItemList(impl_->lights);
  }
  if (impl_->cameras) {
    pipelinecore::DestroyCameraItemList(impl_->cameras);
  }
}

PipelineContext::PipelineContext(PipelineContext&& other) noexcept
  : impl_(std::move(other.impl_)) {
}

PipelineContext& PipelineContext::operator=(PipelineContext&& other) noexcept {
  if (this != &other) {
    impl_ = std::move(other.impl_);
  }
  return *this;
}

void PipelineContext::SetDevice(rhi::IDevice* device) {
  impl_->device = device;
  if (impl_->resourcePool) {
    impl_->resourcePool->SetDevice(device);
  }
  if (impl_->submitCtx) {
    impl_->submitCtx->SetDevice(device);
  }
}

rhi::IDevice* PipelineContext::GetDevice() const {
  return impl_->device;
}

void PipelineContext::SetRenderingConfig(RenderingConfig const* config) {
  impl_->config = config;
}

RenderingConfig const* PipelineContext::GetRenderingConfig() const {
  return impl_->config;
}

void PipelineContext::SetFrameGraph(pipelinecore::IFrameGraph* graph) {
  impl_->frameGraph = graph;
}

pipelinecore::IFrameGraph* PipelineContext::GetFrameGraph() const {
  return impl_->frameGraph;
}

void PipelineContext::SetSwapChain(rhi::ISwapChain* swapChain) {
  impl_->swapChain = swapChain;
}

rhi::ISwapChain* PipelineContext::GetSwapChain() const {
  return impl_->swapChain;
}

void PipelineContext::BeginFrame(pipelinecore::FrameContext const& frameCtx) {
  impl_->frameCtx = frameCtx;
  impl_->frameIndex++;
  impl_->Reset();

  if (impl_->resourcePool) {
    impl_->resourcePool->BeginFrame();
  }

  impl_->stats.frameIndex = impl_->frameIndex;
}

void PipelineContext::EndFrame() {
  if (impl_->resourcePool) {
    impl_->resourcePool->EndFrame();
  }

  if (impl_->submitCtx) {
    impl_->submitCtx->AdvanceFrame();
  }
}

pipelinecore::FrameContext const& PipelineContext::GetFrameContext() const {
  return impl_->frameCtx;
}

pipelinecore::FrameSlotId PipelineContext::GetFrameSlot() const {
  return impl_->frameCtx.frameSlotId;
}

uint64_t PipelineContext::GetFrameIndex() const {
  return impl_->frameIndex;
}

void PipelineContext::BuildLogicalPipeline() {
  if (impl_->frameGraph) {
    impl_->frameGraph->Compile();
    impl_->logicalPipeline = pipelinecore::BuildLogicalPipeline(
      impl_->frameGraph, impl_->frameCtx);
    impl_->stats.passCount = impl_->logicalPipeline ?
      static_cast<uint32_t>(impl_->logicalPipeline->GetPassCount()) : 0;
  }
}

pipelinecore::ILogicalPipeline* PipelineContext::GetLogicalPipeline() const {
  return impl_->logicalPipeline;
}

void PipelineContext::CollectVisibleObjects() {
  // This would normally query the scene/world for visible objects
  // For now, this is a placeholder - actual collection happens via
  // SetRenderItems() from the RenderableCollector

  // Count visible objects
  impl_->stats.visibleObjectCount = 0;
  for (auto* list : impl_->renderItemsPerPass) {
    if (list) {
      impl_->stats.visibleObjectCount += static_cast<uint32_t>(list->Size());
    }
  }
}

pipelinecore::IRenderItemList* PipelineContext::GetVisibleRenderItems(size_t passIndex) const {
  if (passIndex < impl_->renderItemsPerPass.size()) {
    return impl_->renderItemsPerPass[passIndex];
  }
  return nullptr;
}

pipelinecore::ILightItemList* PipelineContext::GetVisibleLights() const {
  return impl_->lights;
}

pipelinecore::ICameraItemList* PipelineContext::GetActiveCameras() const {
  return impl_->cameras;
}

void PipelineContext::SetRenderItems(size_t passIndex, pipelinecore::IRenderItemList* items) {
  if (passIndex < impl_->renderItemsPerPass.size()) {
    impl_->renderItemsPerPass[passIndex] = items;
  }
}

void PipelineContext::SetLights(pipelinecore::ILightItemList* lights) {
  impl_->lights = lights;
}

void PipelineContext::PrepareResources() {
  if (!impl_->device || !impl_->logicalPipeline) {
    return;
  }

  // Prepare resources for all visible items
  for (size_t i = 0; i < impl_->renderItemsPerPass.size(); ++i) {
    auto* items = impl_->renderItemsPerPass[i];
    if (items && items->Size() > 0) {
      pipelinecore::PrepareRenderResources(items, impl_->device);
    }
  }

  impl_->resourcesReady = true;

  if (impl_->resourcePool) {
    impl_->resourcePool->Compile();
    impl_->stats.resourceCount = static_cast<uint32_t>(
      impl_->resourcePool->GetAllocatedTextureCount() +
      impl_->resourcePool->GetAllocatedBufferSize());
    impl_->stats.memoryUsed = impl_->resourcePool->GetTotalMemoryUsed();
  }
}

bool PipelineContext::AreResourcesReady() const {
  return impl_->resourcesReady;
}

pipelinecore::TransientResourcePool* PipelineContext::GetTransientResourcePool() {
  return impl_->resourcePool.get();
}

rhi::ICommandList* PipelineContext::BeginCommandList() {
  if (impl_->submitCtx) {
    return impl_->submitCtx->BeginCommandList(pipelinecore::QueueId::Graphics);
  }
  return nullptr;
}

void PipelineContext::EndCommandList(rhi::ICommandList* cmd) {
  if (impl_->submitCtx && cmd) {
    cmd->End();
    impl_->submitCtx->EndCommandList(cmd);
  }
}

void PipelineContext::ConvertToLogicalCommandBuffer() {
  if (!impl_->logicalPipeline) return;

  // Get render items from first pass (simplified)
  auto* items = impl_->renderItemsPerPass[0];
  if (items && items->Size() > 0) {
    impl_->logicalCB = pipelinecore::ConvertToLogicalCommandBuffer(
      items, impl_->logicalPipeline);
  }
}

void PipelineContext::ExecutePasses() {
  if (!impl_->frameGraph || !impl_->device) return;

  auto* cmd = BeginCommandList();
  if (!cmd) return;

  // Get viewport dimensions
  uint32_t width = GetWidth();
  uint32_t height = GetHeight();
  if (width == 0 || height == 0) {
    width = 800;
    height = 600;
  }

  // Setup viewport and scissor
  rhi::Viewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(width);
  viewport.height = static_cast<float>(height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  cmd->SetViewport(0, 1, &viewport);

  rhi::ScissorRect scissor{};
  scissor.x = 0;
  scissor.y = 0;
  scissor.width = width;
  scissor.height = height;
  cmd->SetScissor(0, 1, &scissor);

  // Get back buffer for render target
  rhi::ITexture* backBuffer = GetBackBuffer();

  // Insert barriers for each pass
  if (impl_->resourcePool) {
    for (size_t i = 0; i < impl_->stats.passCount; ++i) {
      impl_->resourcePool->InsertBarriersForPass(static_cast<uint32_t>(i), cmd);
    }
  }

  // Get frame slot for resource updates
  uint32_t frameSlot = impl_->frameCtx.frameSlotId;

  // Execute each pass
  pipelinecore::PassContext passCtx;
  for (size_t i = 0; i < impl_->stats.passCount; ++i) {
    // Set render items for this pass
    if (i < impl_->renderItemsPerPass.size()) {
      passCtx.SetRenderItemList(0, impl_->renderItemsPerPass[i]);
    }
    passCtx.SetLightItemList(impl_->lights);

    // Build RenderPass description
    rhi::RenderPassDesc rpDesc{};
    rpDesc.colorAttachmentCount = 1;
    rpDesc.colorAttachments[0].texture = backBuffer;
    rpDesc.colorAttachments[0].loadOp = rhi::LoadOp::Clear;
    rpDesc.colorAttachments[0].storeOp = rhi::StoreOp::Store;
    rpDesc.colorAttachments[0].clearColor[0] = 0.1f;
    rpDesc.colorAttachments[0].clearColor[1] = 0.1f;
    rpDesc.colorAttachments[0].clearColor[2] = 0.2f;
    rpDesc.colorAttachments[0].clearColor[3] = 1.0f;
    // Set format to 0 (auto-infer from texture)
    rpDesc.colorAttachments[0].format = 0;
    rpDesc.subpassCount = 0;  // Single subpass mode

    // Begin render pass
    cmd->BeginRenderPass(rpDesc, nullptr);

    // For the first pass (main geometry pass), execute logical command buffer
    if (i == 0 && impl_->logicalCB) {
      ExecutionStats execStats{};
      ExecuteLogicalCommandBufferOnDeviceThreadWithStats(cmd, impl_->logicalCB, frameSlot, &execStats);

      // Update stats
      impl_->stats.drawCallCount += execStats.drawCalls;
      impl_->stats.instanceCount += execStats.instanceCount;
      impl_->stats.triangleCount += execStats.triangleCount;
      impl_->stats.vertexCount += execStats.vertexCount;
    }

    // Execute pass callback from FrameGraph
    impl_->frameGraph->ExecutePass(i, passCtx, cmd);

    // End render pass
    cmd->EndRenderPass();
  }

  // If no passes defined, do a simple clear pass
  if (impl_->stats.passCount == 0 && backBuffer) {
    rhi::RenderPassDesc rpDesc{};
    rpDesc.colorAttachmentCount = 1;
    rpDesc.colorAttachments[0].texture = backBuffer;
    rpDesc.colorAttachments[0].loadOp = rhi::LoadOp::Clear;
    rpDesc.colorAttachments[0].storeOp = rhi::StoreOp::Store;
    rpDesc.colorAttachments[0].clearColor[0] = 0.1f;
    rpDesc.colorAttachments[0].clearColor[1] = 0.1f;
    rpDesc.colorAttachments[0].clearColor[2] = 0.2f;
    rpDesc.colorAttachments[0].clearColor[3] = 1.0f;
    rpDesc.subpassCount = 0;

    cmd->BeginRenderPass(rpDesc, nullptr);

    // Execute logical command buffer if we have one
    if (impl_->logicalCB) {
      ExecutionStats execStats{};
      ExecuteLogicalCommandBufferOnDeviceThreadWithStats(cmd, impl_->logicalCB, frameSlot, &execStats);

      impl_->stats.drawCallCount += execStats.drawCalls;
      impl_->stats.instanceCount += execStats.instanceCount;
      impl_->stats.triangleCount += execStats.triangleCount;
      impl_->stats.vertexCount += execStats.vertexCount;
    }

    cmd->EndRenderPass();
  }

  EndCommandList(cmd);
}

void PipelineContext::Submit() {
  if (impl_->submitCtx) {
    impl_->submitCtx->SubmitQueue(pipelinecore::QueueId::Graphics);
  }
}

void PipelineContext::Present() {
  if (impl_->swapChain) {
    impl_->swapChain->Present();
  }
}

pipelinecore::SubmitContext* PipelineContext::GetSubmitContext() {
  return impl_->submitCtx.get();
}

void PipelineContext::SetRenderTarget(RenderTarget const& target) {
  impl_->renderTarget = target;
}

RenderTarget const& PipelineContext::GetRenderTarget() const {
  return impl_->renderTarget;
}

rhi::ITexture* PipelineContext::GetBackBuffer() const {
  return impl_->swapChain ? impl_->swapChain->GetBackBuffer() : nullptr;
}

void PipelineContext::BuildBatches() {
  // Build batches from render items
  // Simplified implementation - actual would sort and group by material/mesh
  impl_->batches.clear();

  for (size_t i = 0; i < impl_->renderItemsPerPass.size(); ++i) {
    auto* items = impl_->renderItemsPerPass[i];
    if (!items) continue;

    for (size_t j = 0; j < items->Size(); ++j) {
      auto const* item = items->At(j);
      if (item && item->element) {
        RenderBatchData batch{};
        batch.instanceCount = 1;
        impl_->batches.push_back(batch);
      }
    }
  }

  impl_->stats.batchCount = static_cast<uint32_t>(impl_->batches.size());
}

size_t PipelineContext::GetBatchCount() const {
  return impl_->batches.size();
}

RenderBatch const* PipelineContext::GetBatch(size_t index) const {
  if (index < impl_->batches.size()) {
    // Cast to public type
    return reinterpret_cast<RenderBatch const*>(&impl_->batches[index]);
  }
  return nullptr;
}

FrameStats const& PipelineContext::GetFrameStats() const {
  return impl_->stats;
}

void PipelineContext::ResetFrameStats() {
  impl_->stats = FrameStats{};
  impl_->stats.frameIndex = impl_->frameIndex;
}

bool PipelineContext::IsValid() const {
  return impl_->device != nullptr;
}

uint32_t PipelineContext::GetWidth() const {
  return impl_->frameCtx.viewport.width;
}

uint32_t PipelineContext::GetHeight() const {
  return impl_->frameCtx.viewport.height;
}

// === Free Functions ===

PipelineContext* CreatePipelineContext() {
  return new PipelineContext();
}

void DestroyPipelineContext(PipelineContext* ctx) {
  delete ctx;
}

}  // namespace te::pipeline
