/**
 * @file RenderPipeline.cpp
 * @brief Implementation of IRenderPipeline.
 */

#include <te/pipeline/RenderPipeline.h>
#include <te/pipeline/PipelineContext.h>
#include <te/pipeline/LogicalCommandBufferExecutor.h>

#include <te/pipelinecore/FrameGraph.h>
#include <te/pipelinecore/LogicalPipeline.h>
#include <te/pipelinecore/LogicalCommandBuffer.h>
#include <te/pipelinecore/ResourceManager.h>
#include <te/pipelinecore/SubmitContext.h>
#include <te/rhi/device.hpp>
#include <te/rhi/swapchain.hpp>
#include <te/rhi/sync.hpp>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <deque>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace te::pipeline {

// === Utility Functions ===

char const* GetPhaseName(RenderPhase phase) {
  switch (phase) {
    case RenderPhase::Idle: return "Idle";
    case RenderPhase::GameUpdate: return "GameUpdate";
    case RenderPhase::BuildPipeline: return "BuildPipeline";
    case RenderPhase::Collect: return "Collect";
    case RenderPhase::Prepare: return "Prepare";
    case RenderPhase::Record: return "Record";
    case RenderPhase::Submit: return "Submit";
    case RenderPhase::Present: return "Present";
    default: return "Unknown";
  }
}

bool IsGPUPound(RenderPhase phase) {
  return phase >= RenderPhase::Prepare && phase <= RenderPhase::Present;
}

// === RenderPipelineImpl ===

class RenderPipelineImpl : public IRenderPipeline {
public:
  RenderPipelineImpl() = default;
  ~RenderPipelineImpl() override { Shutdown(); }

  // === Configuration ===

  void SetDevice(rhi::IDevice* device) override {
    device_ = device;
    if (pipelineCtx_) {
      pipelineCtx_->SetDevice(device);
    }
    // Create frame fences for synchronization
    if (device) {
      InitFrameFences();
    }
  }

  rhi::IDevice* GetDevice() const override {
    return device_;
  }

  void SetRenderingConfig(RenderingConfig const* config) override {
    config_ = config ? config : &defaultConfig_;
    if (pipelineCtx_) {
      pipelineCtx_->SetRenderingConfig(config_);
    }
  }

  RenderingConfig const* GetRenderingConfig() const override {
    return config_;
  }

  void SetSwapChain(rhi::ISwapChain* swapChain) override {
    swapChain_ = swapChain;
    if (pipelineCtx_) {
      pipelineCtx_->SetSwapChain(swapChain);
    }
  }

  rhi::ISwapChain* GetSwapChain() const override {
    return swapChain_;
  }

  // === Frame Graph ===

  void SetFrameGraph(pipelinecore::IFrameGraph* graph) override {
    frameGraph_ = graph;
    if (pipelineCtx_) {
      pipelineCtx_->SetFrameGraph(graph);
    }
  }

  pipelinecore::IFrameGraph* GetFrameGraph() override {
    return frameGraph_;
  }

  pipelinecore::ILogicalPipeline* GetLogicalPipeline() override {
    return pipelineCtx_ ? pipelineCtx_->GetLogicalPipeline() : nullptr;
  }

  // === Rendering ===

  void RenderFrame(pipelinecore::FrameContext const& ctx) override {
    if (!isInitialized_ || !device_) return;

    auto startTime = std::chrono::high_resolution_clock::now();

    // Get current frame slot
    uint32_t slot = GetCurrentSlot().value;

    // Wait for previous frame at this slot to complete
    WaitForSlot(slot);

    // Phase B: Build logical pipeline
    pipelineCtx_->BeginFrame(ctx);
    pipelineCtx_->BuildLogicalPipeline();

    // Phase C: Collect visible objects
    pipelineCtx_->CollectVisibleObjects();

    // Phase D: Prepare resources
    pipelineCtx_->PrepareResources();

    // Phase D: Build batches
    pipelineCtx_->BuildBatches();

    // Phase D: Convert to logical command buffer
    pipelineCtx_->ConvertToLogicalCommandBuffer();

    // Phase D: Execute passes and record commands
    pipelineCtx_->ExecutePasses();

    // Phase D: Submit
    pipelineCtx_->Submit();

    // Signal fence for this slot
    SignalSlot(slot);

    // Phase D: Present
    pipelineCtx_->Present();

    // End frame
    pipelineCtx_->EndFrame();

    auto endTime = std::chrono::high_resolution_clock::now();
    double frameTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();

    UpdateStats(frameTime);
    frameIndex_++;
  }

  bool TickPipeline(pipelinecore::FrameContext const& ctx) override {
    if (!isInitialized_ || !device_) return false;

    switch (currentPhase_.load()) {
      case RenderPhase::Idle:
        // Start new frame
        frameCtx_ = ctx;
        frameSlot_ = GetCurrentSlot().value;
        WaitForSlot(frameSlot_);
        currentPhase_ = RenderPhase::BuildPipeline;
        pipelineCtx_->BeginFrame(ctx);
        [[fallthrough]];

      case RenderPhase::BuildPipeline:
        pipelineCtx_->BuildLogicalPipeline();
        currentPhase_ = RenderPhase::Collect;
        [[fallthrough]];

      case RenderPhase::Collect:
        pipelineCtx_->CollectVisibleObjects();
        currentPhase_ = RenderPhase::Prepare;
        [[fallthrough]];

      case RenderPhase::Prepare:
        pipelineCtx_->PrepareResources();
        currentPhase_ = RenderPhase::Record;
        [[fallthrough]];

      case RenderPhase::Record:
        pipelineCtx_->BuildBatches();
        pipelineCtx_->ConvertToLogicalCommandBuffer();
        pipelineCtx_->ExecutePasses();
        currentPhase_ = RenderPhase::Submit;
        [[fallthrough]];

      case RenderPhase::Submit:
        pipelineCtx_->Submit();
        SignalSlot(frameSlot_);
        currentPhase_ = RenderPhase::Present;
        [[fallthrough]];

      case RenderPhase::Present:
        pipelineCtx_->Present();
        pipelineCtx_->EndFrame();
        currentPhase_ = RenderPhase::Idle;
        frameIndex_++;

        UpdateStats(0.0); // Would need proper timing
        return true;

      default:
        return false;
    }
  }

  // === Multi-threaded Control ===

  void TriggerRender(pipelinecore::FrameContext const& ctx) override {
    // Start the render process
    frameCtx_ = ctx;
    frameSlot_ = GetCurrentSlot().value;
    WaitForSlot(frameSlot_);
    currentPhase_ = RenderPhase::BuildPipeline;
    pipelineCtx_->BeginFrame(ctx);
  }

  bool IsFrameComplete() const override {
    return currentPhase_ == RenderPhase::Idle;
  }

  void WaitForFrame() override {
    // Wait for frame completion
    while (!IsFrameComplete()) {
      std::this_thread::yield();
    }
  }

  // === Frame Slot Management ===

  pipelinecore::FrameSlotId GetCurrentSlot() const override {
    return pipelinecore::FrameSlotId(frameIndex_ % config_->maxFramesInFlight);
  }

  uint32_t GetFramesInFlight() const override {
    return config_->maxFramesInFlight;
  }

  void SetFramesInFlight(uint32_t count) override {
    uint32_t newCount = std::clamp(count, 1u, 4u);
    if (newCount != config_->maxFramesInFlight) {
      const_cast<RenderingConfig*>(config_)->maxFramesInFlight = newCount;
      // Recreate fences for new frame count
      InitFrameFences();
    }
  }

  // === Resource Management ===

  pipelinecore::TransientResourcePool* GetTransientResourcePool() override {
    return pipelineCtx_ ? pipelineCtx_->GetTransientResourcePool() : nullptr;
  }

  pipelinecore::SubmitContext* GetSubmitContext() override {
    return pipelineCtx_ ? pipelineCtx_->GetSubmitContext() : nullptr;
  }

  PipelineContext* GetPipelineContext() override {
    return pipelineCtx_.get();
  }

  // === Submission ===

  void SubmitLogicalCommandBuffer(pipelinecore::ILogicalCommandBuffer* cb) override {
    if (!cb || !device_) return;

    auto* cmd = pipelineCtx_->BeginCommandList();
    if (!cmd) return;

    uint32_t slot = GetCurrentSlot().value;

    // Execute logical command buffer using the new executor
    ExecuteLogicalCommandBufferOnDeviceThread(cmd, cb, slot);

    pipelineCtx_->EndCommandList(cmd);
    pipelineCtx_->Submit();
  }

  void ExecutePass(size_t passIndex, pipelinecore::IFrameGraph* graph,
                   rhi::ICommandList* cmd) override {
    if (!graph || !cmd) return;

    pipelinecore::PassContext ctx;
    graph->ExecutePass(passIndex, ctx, cmd);
  }

  // === Statistics ===

  uint64_t GetFrameIndex() const override {
    return frameIndex_;
  }

  double GetAverageFPS() const override {
    return averageFPS_;
  }

  double GetFrameTime() const override {
    return lastFrameTime_;
  }

  // === Lifecycle ===

  bool Initialize() override {
    if (isInitialized_) return true;

    pipelineCtx_ = std::unique_ptr<PipelineContext>(CreatePipelineContext());
    if (!pipelineCtx_) return false;

    pipelineCtx_->SetDevice(device_);
    pipelineCtx_->SetRenderingConfig(config_);
    pipelineCtx_->SetSwapChain(swapChain_);
    pipelineCtx_->SetFrameGraph(frameGraph_);

    isInitialized_ = true;
    return true;
  }

  void Shutdown() override {
    if (!isInitialized_) return;

    WaitForFrame();

    // Wait for all fences and destroy them
    if (device_) {
      for (auto* fence : slotFences_) {
        if (fence) {
          fence->Wait();
          device_->DestroyFence(fence);
        }
      }
      slotFences_.clear();
    }

    pipelineCtx_.reset();
    isInitialized_ = false;
  }

  bool IsInitialized() const override {
    return isInitialized_;
  }

private:
  void UpdateStats(double frameTime) {
    lastFrameTime_ = frameTime;

    // Update FPS (exponential moving average)
    if (frameTime > 0) {
      double instantFPS = 1000.0 / frameTime;
      averageFPS_ = averageFPS_ * 0.95 + instantFPS * 0.05;
    }

    frameTimeHistory_.push_back(frameTime);
    if (frameTimeHistory_.size() > 60) {
      frameTimeHistory_.pop_front();
    }
  }

  void InitFrameFences() {
    // Clean up existing fences
    for (auto* fence : slotFences_) {
      if (fence && device_) {
        fence->Wait();
        device_->DestroyFence(fence);
      }
    }
    slotFences_.clear();

    // Create new fences (signaled = true means initially ready)
    slotFences_.resize(config_->maxFramesInFlight);
    for (uint32_t i = 0; i < config_->maxFramesInFlight; ++i) {
      slotFences_[i] = device_->CreateFence(true);
    }
  }

  void WaitForSlot(uint32_t slot) {
    if (slot < slotFences_.size() && slotFences_[slot]) {
      slotFences_[slot]->Wait();
      slotFences_[slot]->Reset();
    }
  }

  void SignalSlot(uint32_t slot) {
    if (slot < slotFences_.size() && slotFences_[slot]) {
      slotFences_[slot]->Signal();
    }
  }

private:
  rhi::IDevice* device_{nullptr};
  RenderingConfig const* config_{&defaultConfig_};
  rhi::ISwapChain* swapChain_{nullptr};
  pipelinecore::IFrameGraph* frameGraph_{nullptr};

  std::unique_ptr<PipelineContext> pipelineCtx_;
  RenderingConfig defaultConfig_;

  pipelinecore::FrameContext frameCtx_;
  std::atomic<RenderPhase> currentPhase_{RenderPhase::Idle};
  uint64_t frameIndex_{0};
  uint32_t frameSlot_{0};

  // Frame synchronization fences
  std::vector<rhi::IFence*> slotFences_;

  double lastFrameTime_{0.0};
  double averageFPS_{0.0};
  std::deque<double> frameTimeHistory_;

  bool isInitialized_{false};
};

// === Factory Functions ===

IRenderPipeline* CreateRenderPipeline() {
  auto* pipeline = new RenderPipelineImpl();
  pipeline->Initialize();
  return pipeline;
}

void DestroyRenderPipeline(IRenderPipeline* pipeline) {
  delete pipeline;
}

}  // namespace te::pipeline
