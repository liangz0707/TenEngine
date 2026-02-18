/**
 * @file RenderPipeline.h
 * @brief 020-Pipeline: Core rendering pipeline interface.
 *
 * IRenderPipeline is the main entry point for rendering.
 * Coordinates multi-threaded rendering through phases A-D:
 * - Phase A: Game update (Thread A)
 * - Phase B: Build logical pipeline (Thread B)
 * - Phase C: Parallel collect (Thread C)
 * - Phase D: GPU operations (Thread D)
 */

#pragma once

#include <te/pipeline/RenderingConfig.h>
#include <te/pipelinecore/FrameContext.h>
#include <te/rendercore/types.hpp>
#include <cstdint>

namespace te::rhi {
struct IDevice;
struct ISwapChain;
struct ITexture;
struct ICommandList;
}

namespace te::pipelinecore {
struct IFrameGraph;
struct ILogicalPipeline;
struct ILogicalCommandBuffer;
class TransientResourcePool;
class SubmitContext;
}

namespace te::pipeline {

class PipelineContext;

/**
 * @brief IRenderPipeline - Core rendering pipeline interface.
 *
 * Usage:
 * 1. Create pipeline with CreateRenderPipeline()
 * 2. Set device, swap chain, frame graph
 * 3. Call RenderFrame() per frame (simple mode)
 *    OR TickPipeline() for multi-threaded control
 */
struct IRenderPipeline {
  virtual ~IRenderPipeline() = default;

  // === Configuration ===

  /// Set the RHI device
  virtual void SetDevice(rhi::IDevice* device) = 0;

  /// Get the RHI device
  virtual rhi::IDevice* GetDevice() const = 0;

  /// Set the rendering configuration
  virtual void SetRenderingConfig(RenderingConfig const* config) = 0;

  /// Get the rendering configuration
  virtual RenderingConfig const* GetRenderingConfig() const = 0;

  /// Set the swap chain for presentation
  virtual void SetSwapChain(rhi::ISwapChain* swapChain) = 0;

  /// Get the swap chain
  virtual rhi::ISwapChain* GetSwapChain() const = 0;

  // === Frame Graph ===

  /// Set the frame graph
  virtual void SetFrameGraph(pipelinecore::IFrameGraph* graph) = 0;

  /// Get the frame graph
  virtual pipelinecore::IFrameGraph* GetFrameGraph() = 0;

  /// Get the current logical pipeline
  virtual pipelinecore::ILogicalPipeline* GetLogicalPipeline() = 0;

  // === Rendering ===

  /// Render a single frame (simple, blocking mode)
  /// Combines all phases in sequence on current thread
  virtual void RenderFrame(pipelinecore::FrameContext const& ctx) = 0;

  /// Tick the pipeline (multi-threaded mode)
  /// Advances through phases A-D based on timing
  /// Returns true if a frame was completed
  virtual bool TickPipeline(pipelinecore::FrameContext const& ctx) = 0;

  // === Multi-threaded Control ===

  /// Trigger a render (starts phase B)
  /// Non-blocking: queues work for threads B/C/D
  virtual void TriggerRender(pipelinecore::FrameContext const& ctx) = 0;

  /// Check if current frame is complete
  virtual bool IsFrameComplete() const = 0;

  /// Wait for current frame to complete
  virtual void WaitForFrame() = 0;

  // === Frame Slot Management ===

  /// Get current frame slot index (for multi-buffering)
  virtual pipelinecore::FrameSlotId GetCurrentSlot() const = 0;

  /// Get number of frames in flight
  virtual uint32_t GetFramesInFlight() const = 0;

  /// Set frames in flight count (1-4)
  virtual void SetFramesInFlight(uint32_t count) = 0;

  // === Resource Management ===

  /// Get transient resource pool
  virtual pipelinecore::TransientResourcePool* GetTransientResourcePool() = 0;

  /// Get submit context
  virtual pipelinecore::SubmitContext* GetSubmitContext() = 0;

  /// Get pipeline context for current frame
  virtual PipelineContext* GetPipelineContext() = 0;

  // === Submission ===

  /// Submit a logical command buffer (Thread D only)
  virtual void SubmitLogicalCommandBuffer(
    pipelinecore::ILogicalCommandBuffer* cb) = 0;

  /// Execute a single pass with command list
  virtual void ExecutePass(
    size_t passIndex,
    pipelinecore::IFrameGraph* graph,
    rhi::ICommandList* cmd) = 0;

  // === Statistics ===

  /// Get current frame index
  virtual uint64_t GetFrameIndex() const = 0;

  /// Get average FPS
  virtual double GetAverageFPS() const = 0;

  /// Get frame time (last frame, milliseconds)
  virtual double GetFrameTime() const = 0;

  // === Lifecycle ===

  /// Initialize the pipeline
  virtual bool Initialize() = 0;

  /// Shutdown the pipeline
  virtual void Shutdown() = 0;

  /// Check if initialized
  virtual bool IsInitialized() const = 0;
};

// === Render Pipeline Phases ===

/// Phase identifiers for multi-threaded control
enum class RenderPhase : uint8_t {
  Idle = 0,
  GameUpdate = 1,       // Phase A
  BuildPipeline = 2,    // Phase B
  Collect = 3,          // Phase C
  Prepare = 4,          // Phase D-1
  Record = 5,           // Phase D-2
  Submit = 6,           // Phase D-3
  Present = 7,          // Phase D-4
};

/// Callback for phase completion
using PhaseCompleteCallback = void (*)(RenderPhase phase, void* userData);

// === Factory Functions ===

/// Create a render pipeline
IRenderPipeline* CreateRenderPipeline();

/// Destroy a render pipeline
void DestroyRenderPipeline(IRenderPipeline* pipeline);

// === Utility Functions ===

/// Get phase name as string
char const* GetPhaseName(RenderPhase phase);

/// Check if phase is GPU-bound (must run on Thread D)
bool IsGPUPhase(RenderPhase phase);

}  // namespace te::pipeline
