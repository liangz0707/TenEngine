/**
 * @file RenderPipeline.h
 * @brief 020-Pipeline: Main render pipeline implementation.
 *
 * Coordinates frame rendering: BuildLogicalPipeline -> CollectRenderables ->
 * Prepare -> Execute -> Submit -> Present.
 */

#ifndef TE_PIPELINE_RENDER_PIPELINE_H
#define TE_PIPELINE_RENDER_PIPELINE_H

#include <te/pipelinecore/FrameGraph.h>
#include <te/pipelinecore/LogicalCommandBuffer.h>
#include <te/pipelinecore/RenderItem.h>
#include <te/rhi/device.hpp>
#include <te/rhi/swapchain.hpp>
#include <functional>
#include <memory>
#include <vector>

namespace te {
namespace pipeline {

/// Rendering configuration for the pipeline
struct RenderingConfig {
  enum class ValidationLevel : uint32_t {
    None = 0,      // Release mode, minimal checks
    Hybrid = 1,    // Partial validation
    Debug = 2      // Full validation
  };

  ValidationLevel validationLevel{ValidationLevel::Hybrid};
  bool enableProfiling{false};
  uint32_t frameSlotCount{2u};  // Double buffering by default
};

/// Render frame statistics
struct FrameStats {
  uint64_t frameNumber{0};
  uint32_t drawCallCount{0};
  uint32_t visibleObjectCount{0};
  float frameTimeMs{0.f};
};

/// Callback type for collecting renderables from the scene
using CollectRenderablesCallback = std::function<void(
    pipelinecore::IRenderItemList* outList,
    pipelinecore::ILightItemList* outLights)>;

/// Main render pipeline class
class RenderPipeline {
 public:
  RenderPipeline();
  ~RenderPipeline();

  RenderPipeline(RenderPipeline const&) = delete;
  RenderPipeline& operator=(RenderPipeline const&) = delete;

  /// Initialize the pipeline with RHI device and swapchain
  bool Initialize(rhi::IDevice* device, rhi::ISwapChain* swapChain);

  /// Shutdown and release resources
  void Shutdown();

  /// Set the rendering configuration
  void SetConfig(RenderingConfig const& config);

  /// Get current configuration
  RenderingConfig const& GetConfig() const { return config_; }

  /// Set callback for collecting renderables from scene
  void SetCollectRenderablesCallback(CollectRenderablesCallback callback);

  /// Add a render pass to the frame graph (before Compile)
  /// Returns IPassBuilder for configuration
  pipelinecore::IPassBuilder* AddPass(char const* name);
  pipelinecore::IPassBuilder* AddPass(char const* name, pipelinecore::PassKind kind);

  /// Compile the frame graph (call after adding all passes)
  bool CompileFrameGraph();

  /// Execute one complete frame: collect -> prepare -> execute -> submit -> present
  void RenderFrame();

  /// Get frame statistics
  FrameStats const& GetFrameStats() const { return frameStats_; }

  /// Get the underlying RHI device
  rhi::IDevice* GetDevice() const { return device_; }

  /// Get the swapchain
  rhi::ISwapChain* GetSwapChain() const { return swapChain_; }

 private:
  /// Phase A: Build logical pipeline and collect renderables
  void BuildAndCollect();

  /// Phase B: Prepare device resources
  void PrepareResources();

  /// Phase C: Execute passes and record commands
  void ExecutePasses();

  /// Phase D: Submit and present
  void SubmitAndPresent();

  /// Create default frame graph (GBuffer + Lighting + Present)
  void CreateDefaultFrameGraph();

  /// Check warning based on validation level
  void CheckWarning(char const* message);

  /// Check error based on validation level
  void CheckError(char const* message);

  rhi::IDevice* device_{nullptr};
  rhi::ISwapChain* swapChain_{nullptr};
  rhi::ICommandList* commandList_{nullptr};
  rhi::IQueue* graphicsQueue_{nullptr};

  RenderingConfig config_;
  FrameStats frameStats_;

  pipelinecore::IFrameGraph* frameGraph_{nullptr};
  pipelinecore::ILogicalPipeline* logicalPipeline_{nullptr};
  pipelinecore::IRenderItemList* renderItemList_{nullptr};
  pipelinecore::ILightItemList* lightItemList_{nullptr};
  pipelinecore::ILogicalCommandBuffer* logicalCommandBuffer_{nullptr};

  CollectRenderablesCallback collectCallback_;

  uint32_t currentFrameSlot_{0};
  bool initialized_{false};
};

/// Global functions for CheckWarning/CheckError macros
void CheckWarningGlobal(RenderingConfig const& config, char const* message);
void CheckErrorGlobal(RenderingConfig const& config, char const* message);

}  // namespace pipeline
}  // namespace te

#endif  // TE_PIPELINE_RENDER_PIPELINE_H
