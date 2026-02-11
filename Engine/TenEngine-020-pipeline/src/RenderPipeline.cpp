/**
 * @file RenderPipeline.cpp
 * @brief 020-Pipeline implementation: RenderPipelineImpl, CreateRenderPipeline, Device thread dispatch.
 */

#include <te/pipeline/RenderPipeline.h>
#include <te/pipeline/FrameContext.h>
#include <te/pipeline/RenderingConfig.h>
#include <te/pipeline/detail/PipelineImpl.h>
#include <te/pipeline/detail/RenderableCollector.h>
#include <te/pipelinecore/FrameGraph.h>
#include <te/pipelinecore/LogicalPipeline.h>
#include <te/pipelinecore/LogicalCommandBuffer.h>
#include <te/pipelinecore/RenderItem.h>
#include <te/rendercore/uniform_buffer.hpp>
#include <te/material/MaterialResource.h>
#include <te/resource/Resource.h>
#include <te/resource/ResourceManager.h>
#include <te/rhi/device.hpp>
#include <te/rhi/command_list.hpp>
#include <te/rhi/queue.hpp>
#include <te/rhi/sync.hpp>
#include <te/rhi/swapchain.hpp>
#include <te/scene/SceneTypes.h>
#include <te/world/WorldManager.h>
#include <te/mesh/Mesh.h>
#include <te/mesh/MeshResource.h>
#include <te/mesh/MeshDevice.h>
#include <te/pipelinecore/LogicalCommandBuffer.h>
#include <te/rendercore/types.hpp>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace te {
namespace pipeline {

/// 通用单线程任务队列（Render / Device 共用模式）
class SingleThreadQueue {
 public:
  SingleThreadQueue() {
    thread_ = std::thread([this]() {
      for (;;) {
        std::function<void()> task;
        {
          std::unique_lock<std::mutex> lock(mutex_);
          cond_.wait(lock, [this]() { return stop_ || !queue_.empty(); });
          if (stop_ && queue_.empty()) break;
          if (!queue_.empty()) {
            task = std::move(queue_.front());
            queue_.pop();
          }
        }
        if (task) task();
      }
    });
  }
  ~SingleThreadQueue() {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      stop_ = true;
    }
    cond_.notify_all();
    if (thread_.joinable()) thread_.join();
  }
  void Post(std::function<void()> task) {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      queue_.push(std::move(task));
    }
    cond_.notify_one();
  }
 private:
  std::mutex mutex_;
  std::condition_variable cond_;
  std::queue<std::function<void()>> queue_;
  bool stop_{false};
  std::thread thread_;
};

class RenderPipelineImpl : public IRenderPipeline {
 public:
  explicit RenderPipelineImpl(RenderPipelineDesc const& desc)
      : frameInFlightCount_(desc.frameInFlightCount),
        device_(desc.device),
        swapChain_(desc.swapChain),
        resourceManager_(desc.resourceManager),
        currentSlot_(0u),
        renderQueue_(std::make_unique<SingleThreadQueue>()),
        deviceQueue_(std::make_unique<SingleThreadQueue>()) {
    te::rhi::IDevice* rhiDevice = static_cast<te::rhi::IDevice*>(device_);
    if (rhiDevice && frameInFlightCount_ > 0u) {
      slotFences_.resize(frameInFlightCount_, nullptr);
      for (uint32_t i = 0; i < frameInFlightCount_; ++i)
        slotFences_[i] = rhiDevice->CreateFence(i == 0);
    }
  }

  ~RenderPipelineImpl() override {
    te::rhi::IDevice* rhiDevice = static_cast<te::rhi::IDevice*>(device_);
    if (rhiDevice)
      for (te::rhi::IFence* f : slotFences_)
        if (f) rhiDevice->DestroyFence(f);
    slotFences_.clear();
    if (ownFrameGraph_ && frameGraph_) {
      pipelinecore::DestroyFrameGraph(static_cast<pipelinecore::IFrameGraph*>(frameGraph_));
      frameGraph_ = nullptr;
      ownFrameGraph_ = false;
    }
  }

  void RenderFrame(FrameContext const& ctx) override {
    TriggerRender(ctx);
  }

  void TickPipeline(FrameContext const& ctx) override {
    TriggerRender(ctx);
  }

  void TriggerRender(FrameContext const& ctx) override {
    // 帧 slot 推进（主线程）
    currentSlot_ = (currentSlot_ + 1u) % (frameInFlightCount_ > 0u ? frameInFlightCount_ : 1u);

    pipelinecore::IFrameGraph* graph = static_cast<pipelinecore::IFrameGraph*>(frameGraph_);
    if (!graph) {
      graph = detail::CreateDeferredFrameGraph(ctx.viewport.width, ctx.viewport.height);
      if (graph) {
        frameGraph_ = graph;
        ownFrameGraph_ = true;
      }
    }
    if (!graph) return;

    // 阶段 A 投递到 Render 线程：BuildLogicalPipeline、CollectRenderablesToRenderItemList；完成后投递阶段 B 到 Device 线程
    FrameContext ctxCopy = ctx;
    void* dev = device_;
    uint32_t slot = currentSlot_;
    uint32_t vpW = ctx.viewport.width;
    uint32_t vpH = ctx.viewport.height;
    void* resMgr = resourceManager_;
    pipelinecore::IFrameGraph* graphCapture = graph;

    renderQueue_->Post([this, ctxCopy, dev, slot, vpW, vpH, resMgr, graphCapture]() {
      detail::SceneWorldAdapter sceneAdapter;
      sceneAdapter.sceneRefOrLevelHandle = ctxCopy.sceneRoot;
      sceneAdapter.useLevelHandle = false;
      pipelinecore::FrameContext coreCtx;
      detail::ToPipelinecoreFrameContext(ctxCopy, &sceneAdapter, coreCtx);
      coreCtx.frameSlotId = slot;

      pipelinecore::ILogicalPipeline* pipeline = pipelinecore::BuildLogicalPipeline(graphCapture, coreCtx);
      if (!pipeline) return;

      pipelinecore::IRenderItemList* itemList = pipelinecore::CreateRenderItemList();
      if (!itemList) {
        pipelinecore::DestroyLogicalPipeline(pipeline);
        return;
      }

      te::scene::SceneRef sceneRef;
      if (ctxCopy.sceneRoot) {
        sceneRef = *static_cast<te::scene::SceneRef const*>(ctxCopy.sceneRoot);
      } else {
        te::world::WorldManager& world = te::world::WorldManager::GetInstance();
        sceneRef = world.GetCurrentLevelScene();
      }
      if (sceneRef.IsValid()) {
        CollectRenderablesToRenderItemList(sceneRef,
            static_cast<te::resource::IResourceManager*>(resMgr), itemList, ctxCopy.frustum,
            static_cast<float const*>(ctxCopy.camera));
      }
      te::resource::IResourceManager* rm = static_cast<te::resource::IResourceManager*>(resMgr);
      for (size_t i = 0; rm && i < itemList->Size(); ++i) {
        pipelinecore::RenderItem const* r = itemList->At(i);
        if (!r || !r->mesh) continue;
        te::mesh::MeshResource const* mesh012 = reinterpret_cast<te::mesh::MeshResource const*>(r->mesh);
        if (mesh012) rm->RequestStreaming(mesh012->GetResourceId(), 0);
      }

      // 阶段 B：投递到 Device 线程
      deviceQueue_->Post([dev, itemList, pipeline, graphCapture, slot, vpW, vpH, this]() {
      te::rhi::IDevice* rhiDevice = static_cast<te::rhi::IDevice*>(dev);
      if (rhiDevice && itemList && pipeline) {
        if (slot < static_cast<uint32_t>(slotFences_.size()) && slotFences_[slot])
          te::rhi::Wait(slotFences_[slot]);
        // Mesh/Material Ensure 由 019 PrepareRenderResources 统一完成，020 不再单独对 mesh 做 Ensure
        pipelinecore::PrepareRenderResources(itemList, rhiDevice);
        pipelinecore::IRenderItemList* readyList = pipelinecore::CreateRenderItemList();
        if (readyList) {
          for (size_t i = 0; i < itemList->Size(); ++i) {
            pipelinecore::RenderItem const* r = itemList->At(i);
            if (!r) continue;
            te::resource::IResource const* meshRes = reinterpret_cast<te::resource::IResource const*>(r->mesh);
            te::resource::IResource const* matRes = reinterpret_cast<te::resource::IResource const*>(r->material);
            if (meshRes && matRes && meshRes->IsDeviceReady() && matRes->IsDeviceReady())
              readyList->Push(*r);
          }
        }
        pipelinecore::ILogicalCommandBuffer* logicalCB = nullptr;
        pipelinecore::IRenderItemList* listToConvert = (readyList && readyList->Size() > 0u) ? readyList : itemList;
        if (pipelinecore::ConvertToLogicalCommandBuffer(listToConvert, pipeline, &logicalCB) != te::rendercore::ResultCode::Success || !logicalCB) {
          if (readyList) pipelinecore::DestroyRenderItemList(readyList);
          pipelinecore::DestroyRenderItemList(itemList);
          pipelinecore::DestroyLogicalPipeline(pipeline);
          return;
        }
        if (readyList) pipelinecore::DestroyRenderItemList(readyList);
        te::rhi::ICommandList* cmd = rhiDevice->CreateCommandList();
        if (cmd) {
          te::rhi::Begin(cmd);
          if (vpW > 0u && vpH > 0u) {
            te::rhi::Viewport vp = {0.f, 0.f, static_cast<float>(vpW), static_cast<float>(vpH), 0.f, 1.f};
            cmd->SetViewport(0, 1, &vp);
            te::rhi::ScissorRect scissor = {0, 0, vpW, vpH};
            cmd->SetScissor(0, 1, &scissor);
          }
          // 按 Pass：BeginRenderPass 内录制 Draw（主几何在 pass 0）；有 SwapChain 时填充 RenderPassDesc 使 Vulkan 等后端正确进入 RenderPass
          if (graphCapture && graphCapture->GetPassCount() > 0u) {
            struct Adapter : pipelinecore::IRenderObjectList {
              pipelinecore::IRenderItemList const* list{nullptr};
              size_t Size() const override { return list ? list->Size() : 0; }
            };
            Adapter adapter;
            adapter.list = itemList;
            pipelinecore::PassContext passCtx;
            passCtx.SetCollectedObjects(&adapter);
            te::rhi::RenderPassDesc rpDesc = {};
            if (swapChain_) {
              te::rhi::ISwapChain* sc = static_cast<te::rhi::ISwapChain*>(swapChain_);
              te::rhi::ITexture* backBuffer = sc ? sc->GetCurrentBackBuffer() : nullptr;
              if (backBuffer) {
                rpDesc.colorAttachmentCount = 1u;
                rpDesc.colorAttachments[0].texture = backBuffer;
                rpDesc.colorAttachments[0].loadOp = te::rhi::LoadOp::Clear;
                rpDesc.colorAttachments[0].storeOp = te::rhi::StoreOp::Store;
                rpDesc.colorAttachments[0].clearColor[0] = 0.f;
                rpDesc.colorAttachments[0].clearColor[1] = 0.f;
                rpDesc.colorAttachments[0].clearColor[2] = 0.f;
                rpDesc.colorAttachments[0].clearColor[3] = 1.f;
              }
            }
            for (size_t i = 0; i < graphCapture->GetPassCount(); ++i) {
              cmd->BeginRenderPass(rpDesc);
              // Vulkan：viewport/scissor 必须在 render pass 内设置才会对本 pass 的 draw 生效
              if (vpW > 0u && vpH > 0u) {
                te::rhi::Viewport vp = {0.f, 0.f, static_cast<float>(vpW), static_cast<float>(vpH), 0.f, 1.f};
                cmd->SetViewport(0, 1, &vp);
                te::rhi::ScissorRect scissor = {0, 0, vpW, vpH};
                cmd->SetScissor(0, 1, &scissor);
              }
              cmd->BeginOcclusionQuery(0);
              // 主几何绘制放在第一个 Pass（如 GBuffer）的 RenderPass 内，符合 RHI 要求
              if (i == 0u)
                ExecuteLogicalCommandBufferOnDeviceThread(cmd, logicalCB, slot);
              graphCapture->ExecutePass(i, passCtx, cmd);
              cmd->EndOcclusionQuery(0);
              cmd->EndRenderPass();
            }
          } else {
            // 无 FrameGraph 时退化为单 Pass：Begin -> 录制 Draw -> End；有 SwapChain 时填充 RenderPassDesc
            te::rhi::RenderPassDesc rpDesc = {};
            if (swapChain_) {
              te::rhi::ISwapChain* sc = static_cast<te::rhi::ISwapChain*>(swapChain_);
              te::rhi::ITexture* backBuffer = sc ? sc->GetCurrentBackBuffer() : nullptr;
              if (backBuffer) {
                rpDesc.colorAttachmentCount = 1u;
                rpDesc.colorAttachments[0].texture = backBuffer;
                rpDesc.colorAttachments[0].loadOp = te::rhi::LoadOp::Clear;
                rpDesc.colorAttachments[0].storeOp = te::rhi::StoreOp::Store;
                rpDesc.colorAttachments[0].clearColor[0] = 0.f;
                rpDesc.colorAttachments[0].clearColor[1] = 0.f;
                rpDesc.colorAttachments[0].clearColor[2] = 0.f;
                rpDesc.colorAttachments[0].clearColor[3] = 1.f;
              }
            }
            cmd->BeginRenderPass(rpDesc);
            if (vpW > 0u && vpH > 0u) {
              te::rhi::Viewport vp = {0.f, 0.f, static_cast<float>(vpW), static_cast<float>(vpH), 0.f, 1.f};
              cmd->SetViewport(0, 1, &vp);
              te::rhi::ScissorRect scissor = {0, 0, vpW, vpH};
              cmd->SetScissor(0, 1, &scissor);
            }
            ExecuteLogicalCommandBufferOnDeviceThread(cmd, logicalCB, slot);
            cmd->EndRenderPass();
          }
          te::rhi::End(cmd);
          te::rhi::IQueue* queue = rhiDevice->GetQueue(te::rhi::QueueType::Graphics, 0);
          if (queue) te::rhi::Submit(cmd, queue);
          rhiDevice->DestroyCommandList(cmd);
        }
        pipelinecore::DestroyLogicalCommandBuffer(logicalCB);
        if (slot < static_cast<uint32_t>(slotFences_.size()) && slotFences_[slot])
          te::rhi::Signal(slotFences_[slot]);
        if (swapChain_) {
          te::rhi::ISwapChain* sc = static_cast<te::rhi::ISwapChain*>(swapChain_);
          if (sc) sc->Present();
        }
      }
      pipelinecore::DestroyRenderItemList(itemList);
      pipelinecore::DestroyLogicalPipeline(pipeline);
      });
    });
  }

  uint32_t GetCurrentSlot() const override { return currentSlot_; }

  void SetRenderingConfig(RenderingConfig const& config) override { config_ = config; }
  RenderingConfig GetRenderingConfig() const override { return config_; }

  void* GetFrameGraph() override { return frameGraph_; }
  void SetFrameGraph(void* graph) override {
    if (ownFrameGraph_ && frameGraph_) {
      pipelinecore::DestroyFrameGraph(static_cast<pipelinecore::IFrameGraph*>(frameGraph_));
      ownFrameGraph_ = false;
    }
    frameGraph_ = graph;
  }

  void SubmitLogicalCommandBuffer(void* logical_cb) override {
    uint32_t slot = currentSlot_;
    deviceQueue_->Post([this, logical_cb, slot]() {
      ExecuteLogicalCommandBufferOnDeviceThread(static_cast<pipelinecore::ILogicalCommandBuffer*>(logical_cb), 0u, 0u, slot);
    });
  }

  /// 将 logicalCB 的 Draw 录制到已 Begin 的 cmd（viewport 由调用方设置）；不 End/Submit。每条 Draw 前按 material 先 UpdateDescriptorSetForFrame 再 SetGraphicsPSO、BindDescriptorSet。
  void ExecuteLogicalCommandBufferOnDeviceThread(te::rhi::ICommandList* cmd,
                                                 pipelinecore::ILogicalCommandBuffer* logicalCB,
                                                 uint32_t frameSlot) {
    if (!cmd || !logicalCB) return;
    te::rhi::IDevice* rhiDevice = device_ ? static_cast<te::rhi::IDevice*>(device_) : nullptr;
    size_t drawCount = logicalCB->GetDrawCount();
    for (size_t i = 0; i < drawCount; ++i) {
      pipelinecore::LogicalDraw d;
      logicalCB->GetDraw(i, &d);
      if (d.material && rhiDevice) {
        te::material::MaterialResource* matRes =
            const_cast<te::material::MaterialResource*>(reinterpret_cast<te::material::MaterialResource const*>(d.material));
        matRes->UpdateDescriptorSetForFrame(rhiDevice, frameSlot);
        te::rhi::IPSO* pso = matRes->GetGraphicsPSO();
        if (pso) cmd->SetGraphicsPSO(pso);
        te::rhi::IDescriptorSet* ds = matRes->GetDescriptorSet();
        if (ds) cmd->BindDescriptorSet(ds);
      }
      if (!d.mesh) continue;
      te::mesh::MeshResource const* meshRes = reinterpret_cast<te::mesh::MeshResource const*>(d.mesh);
      te::mesh::MeshHandle mh = meshRes->GetMeshHandle();
      if (!mh) continue;
      te::mesh::SubmeshDesc const* sub = te::mesh::GetSubmesh(mh, d.submeshIndex);
      if (!sub) continue;
      te::rhi::IBuffer* vb = te::mesh::GetVertexBufferHandle(mh);
      te::rhi::IBuffer* ib = te::mesh::GetIndexBufferHandle(mh);
      if (!vb || !ib) continue;
      uint32_t vertexStride = te::mesh::GetVertexStride(mh);
      uint32_t indexFormat = te::mesh::GetIndexFormat(mh);
      cmd->SetVertexBuffer(0, vb, 0, vertexStride);
      cmd->SetIndexBuffer(ib, 0, indexFormat);
      cmd->DrawIndexed(sub->count, d.instanceCount, sub->offset, 0, d.firstInstance);
    }
  }

  /// 在线程 D 执行逻辑命令缓冲到 RHI 的录制与提交（仅内部/SubmitLogicalCommandBuffer 等调用）
  void ExecuteLogicalCommandBufferOnDeviceThread(pipelinecore::ILogicalCommandBuffer* logicalCB,
                                                 uint32_t viewportWidth = 0, uint32_t viewportHeight = 0,
                                                 uint32_t frameSlot = 0) {
    if (!logicalCB || !device_) return;
    te::rhi::IDevice* rhiDevice = static_cast<te::rhi::IDevice*>(device_);
    te::rhi::IQueue* queue = rhiDevice->GetQueue(te::rhi::QueueType::Graphics, 0);
    if (!queue) return;
    te::rhi::ICommandList* cmd = rhiDevice->CreateCommandList();
    if (!cmd) return;
    te::rhi::Begin(cmd);
    if (viewportWidth > 0u && viewportHeight > 0u) {
      te::rhi::Viewport vp = {0.f, 0.f, static_cast<float>(viewportWidth), static_cast<float>(viewportHeight), 0.f, 1.f};
      cmd->SetViewport(0, 1, &vp);
      te::rhi::ScissorRect scissor = {0, 0, viewportWidth, viewportHeight};
      cmd->SetScissor(0, 1, &scissor);
    }
    ExecuteLogicalCommandBufferOnDeviceThread(cmd, logicalCB, frameSlot);
    te::rhi::End(cmd);
    te::rhi::Submit(cmd, queue);
    rhiDevice->DestroyCommandList(cmd);
  }

 private:
  uint32_t frameInFlightCount_;
  void* device_;
  void* swapChain_;
  void* resourceManager_{nullptr};
  uint32_t currentSlot_;
  RenderingConfig config_;
  void* frameGraph_{nullptr};
  bool ownFrameGraph_{false};
  std::vector<te::rhi::IFence*> slotFences_;
  std::unique_ptr<SingleThreadQueue> renderQueue_;
  std::unique_ptr<SingleThreadQueue> deviceQueue_;
};

IRenderPipeline* CreateRenderPipeline(RenderPipelineDesc const& desc) {
  return new RenderPipelineImpl(desc);
}

IRenderPipeline* CreateRenderPipeline(void* device) {
  RenderPipelineDesc desc;
  desc.device = device;
  return CreateRenderPipeline(desc);
}

}  // namespace pipeline
}  // namespace te
