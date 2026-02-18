/**
 * @file PipelineContext.h
 * @brief 020-Pipeline: Pipeline execution context.
 *
 * PipelineContext holds all state for a single frame's rendering,
 * including visible sets, batch lists, frame graph, and coordination
 * with PipelineCore (019).
 */

#pragma once

#include <te/pipeline/RenderingConfig.h>
#include <te/pipelinecore/Config.h>
#include <te/pipelinecore/FrameContext.h>
#include <te/rendercore/types.hpp>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace te::rhi {
struct IDevice;
struct ICommandList;
struct ITexture;
struct ISwapChain;
}

namespace te::pipelinecore {
struct IFrameGraph;
struct ILogicalPipeline;
struct IRenderItemList;
struct ILightItemList;
struct ICameraItemList;
class TransientResourcePool;
class SubmitContext;
}

namespace te::pipeline {

// Forward declarations
struct RenderBatch;
struct VisibleSet;

/// Target render surface
struct RenderTarget {
  rhi::ITexture* colorTarget{nullptr};
  rhi::ITexture* depthTarget{nullptr};
  uint32_t width{0};
  uint32_t height{0};
  uint32_t sampleCount{1};
  bool isSwapChain{false};
};

/// Frame statistics for profiling
struct FrameStats {
  uint64_t frameIndex{0};
  double frameTime{0.0};           // milliseconds
  double cpuTime{0.0};             // milliseconds
  double gpuTime{0.0};             // milliseconds

  uint32_t drawCallCount{0};
  uint32_t instanceCount{0};
  uint32_t triangleCount{0};
  uint32_t vertexCount{0};

  uint32_t visibleObjectCount{0};
  uint32_t culledObjectCount{0};
  uint32_t batchCount{0};

  uint32_t passCount{0};
  uint32_t resourceCount{0};
  size_t memoryUsed{0};            // bytes
};

/**
 * @brief PipelineContext coordinates all state for a single frame render.
 *
 * Manages:
 * - Frame graph and logical pipeline
 * - Visible sets and culling results
 * - Batch lists and draw call generation
 * - Transient resources
 * - Submission and synchronization
 *
 * Lifecycle:
 * 1. BeginFrame() - Reset for new frame
 * 2. Collect phase - Gather visible objects
 * 3. Prepare phase - Create/prepare GPU resources
 * 4. Record phase - Build command buffers
 * 5. Submit phase - Submit to GPU
 * 6. EndFrame() - Present and advance
 */
class PipelineContext {
public:
  PipelineContext();
  ~PipelineContext();

  PipelineContext(PipelineContext const&) = delete;
  PipelineContext& operator=(PipelineContext const&) = delete;
  PipelineContext(PipelineContext&&) noexcept;
  PipelineContext& operator=(PipelineContext&&) noexcept;

  // === Initialization ===

  /// Set the RHI device
  void SetDevice(rhi::IDevice* device);

  /// Get the RHI device
  rhi::IDevice* GetDevice() const;

  /// Set the rendering configuration
  void SetRenderingConfig(RenderingConfig const* config);

  /// Get the rendering configuration
  RenderingConfig const* GetRenderingConfig() const;

  /// Set the frame graph
  void SetFrameGraph(pipelinecore::IFrameGraph* graph);

  /// Get the frame graph
  pipelinecore::IFrameGraph* GetFrameGraph() const;

  /// Set the swap chain
  void SetSwapChain(rhi::ISwapChain* swapChain);

  /// Get the swap chain
  rhi::ISwapChain* GetSwapChain() const;

  // === Frame Lifecycle ===

  /// Begin a new frame
  void BeginFrame(pipelinecore::FrameContext const& frameCtx);

  /// End the current frame
  void EndFrame();

  /// Get current frame context
  pipelinecore::FrameContext const& GetFrameContext() const;

  /// Get current frame slot index
  pipelinecore::FrameSlotId GetFrameSlot() const;

  /// Get current frame index
  uint64_t GetFrameIndex() const;

  // === Phase A: Game Update (Thread A) ===
  // Handled by game logic, not directly by PipelineContext

  // === Phase B: Build Logical Pipeline (Thread B) ===

  /// Build the logical pipeline from frame graph
  void BuildLogicalPipeline();

  /// Get the logical pipeline
  pipelinecore::ILogicalPipeline* GetLogicalPipeline() const;

  // === Phase C: Collect (Thread C) ===

  /// Collect visible renderables
  void CollectVisibleObjects();

  /// Get visible render item list for a pass
  pipelinecore::IRenderItemList* GetVisibleRenderItems(size_t passIndex) const;

  /// Get visible light item list
  pipelinecore::ILightItemList* GetVisibleLights() const;

  /// Get active camera
  pipelinecore::ICameraItemList* GetActiveCameras() const;

  /// Set collected render items (from external collector)
  void SetRenderItems(size_t passIndex, pipelinecore::IRenderItemList* items);

  /// Set collected lights
  void SetLights(pipelinecore::ILightItemList* lights);

  // === Phase D: Prepare Resources (Thread D - GPU Thread) ===

  /// Prepare GPU resources for all visible items
  void PrepareResources();

  /// Check if all resources are ready
  bool AreResourcesReady() const;

  /// Get transient resource pool
  pipelinecore::TransientResourcePool* GetTransientResourcePool();

  // === Phase D: Record Commands (Thread D - GPU Thread) ===

  /// Begin command list recording for a queue
  rhi::ICommandList* BeginCommandList();

  /// End command list recording
  void EndCommandList(rhi::ICommandList* cmd);

  /// Convert render items to logical command buffer
  void ConvertToLogicalCommandBuffer();

  /// Execute passes and record draw commands
  void ExecutePasses();

  // === Phase D: Submit (Thread D - GPU Thread) ===

  /// Submit all pending commands
  void Submit();

  /// Present to swap chain
  void Present();

  /// Get submit context
  pipelinecore::SubmitContext* GetSubmitContext();

  // === Render Target Management ===

  /// Set the current render target
  void SetRenderTarget(RenderTarget const& target);

  /// Get the current render target
  RenderTarget const& GetRenderTarget() const;

  /// Get back buffer texture
  rhi::ITexture* GetBackBuffer() const;

  // === Batching ===

  /// Build batches from visible items
  void BuildBatches();

  /// Get batch count
  size_t GetBatchCount() const;

  /// Get batch by index
  RenderBatch const* GetBatch(size_t index) const;

  // === Statistics ===

  /// Get frame statistics
  FrameStats const& GetFrameStats() const;

  /// Reset frame statistics
  void ResetFrameStats();

  // === Utility ===

  /// Check if context is valid for rendering
  bool IsValid() const;

  /// Get current viewport width
  uint32_t GetWidth() const;

  /// Get current viewport height
  uint32_t GetHeight() const;

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

// === Free Functions ===

/// Create a pipeline context
PipelineContext* CreatePipelineContext();

/// Destroy a pipeline context
void DestroyPipelineContext(PipelineContext* ctx);

}  // namespace te::pipeline
