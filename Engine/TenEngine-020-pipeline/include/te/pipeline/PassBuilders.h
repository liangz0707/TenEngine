/**
 * @file PassBuilders.h
 * @brief 020-Pipeline: Standard render pass implementations.
 *
 * Provides built-in pass builders for common rendering operations:
 * - GBufferPass: Scene geometry pass with albedo/normal/depth outputs
 * - LightingPass: Deferred lighting computation
 * - PostProcessPass: Screen-space effects
 */

#ifndef TE_PIPELINE_PASS_BUILDERS_H
#define TE_PIPELINE_PASS_BUILDERS_H

#include <te/pipelinecore/FrameGraph.h>
#include <te/pipelinecore/RenderItem.h>
#include <te/rhi/command_list.hpp>
#include <te/rhi/device.hpp>

namespace te {
namespace pipeline {

/// GBuffer pass configuration
struct GBufferPassConfig {
  uint32_t width{1280};
  uint32_t height{720};
  bool useNormalBuffer{true};
  bool useMaterialBuffer{true};
  bool useDepthBuffer{true};
  float clearColor[4]{0.f, 0.f, 0.f, 1.f};
  float clearDepth{1.f};
};

/// Lighting pass configuration
struct LightingPassConfig {
  uint32_t width{1280};
  uint32_t height{720};
  bool useShadows{false};
  bool useIBL{false};
  uint32_t maxLights{256};
};

/// Post-process pass configuration
struct PostProcessPassConfig {
  uint32_t width{1280};
  uint32_t height{720};
  bool enableToneMapping{true};
  bool enableGammaCorrection{true};
  float exposure{1.0f};
  float gamma{2.2f};
};

/// Create a GBuffer scene pass
/// @param fg Frame graph to add pass to
/// @param config GBuffer configuration
/// @return Pass builder for further configuration
pipelinecore::IPassBuilder* CreateGBufferPass(
    pipelinecore::IFrameGraph* fg,
    GBufferPassConfig const& config = {});

/// Create a deferred lighting pass
/// @param fg Frame graph to add pass to
/// @param config Lighting configuration
/// @return Pass builder for further configuration
pipelinecore::IPassBuilder* CreateLightingPass(
    pipelinecore::IFrameGraph* fg,
    LightingPassConfig const& config = {});

/// Create a post-process pass
/// @param fg Frame graph to add pass to
/// @param config Post-process configuration
/// @return Pass builder for further configuration
pipelinecore::IPassBuilder* CreatePostProcessPass(
    pipelinecore::IFrameGraph* fg,
    PostProcessPassConfig const& config = {});

/// Create a present pass (copies to backbuffer)
/// @param fg Frame graph to add pass to
/// @return Pass builder for further configuration
pipelinecore::IPassBuilder* CreatePresentPass(pipelinecore::IFrameGraph* fg);

/// Execute scene pass (GBuffer-style) with render items
/// @param ctx Pass context containing render items
/// @param cmd RHI command list
/// @param device RHI device
/// @param frameSlot Current frame slot
/// @param logicalCb Pre-built logical command buffer (or nullptr to build on-the-fly)
void ExecuteScenePass(
    pipelinecore::PassContext& ctx,
    rhi::ICommandList* cmd,
    rhi::IDevice* device,
    uint32_t frameSlot,
    pipelinecore::ILogicalCommandBuffer* logicalCb);

/// Execute lighting pass with light items
/// @param ctx Pass context containing light items
/// @param cmd RHI command list
/// @param device RHI device
void ExecuteLightingPass(
    pipelinecore::PassContext& ctx,
    rhi::ICommandList* cmd,
    rhi::IDevice* device);

/// Execute post-process pass (fullscreen quad)
/// @param ctx Pass context
/// @param cmd RHI command list
/// @param device RHI device
/// @param materialName Post-process material name
void ExecutePostProcessPass(
    pipelinecore::PassContext& ctx,
    rhi::ICommandList* cmd,
    rhi::IDevice* device,
    char const* materialName);

/// Builtin pass names
namespace PassNames {
  constexpr char const* GBuffer = "GBuffer";
  constexpr char const* Lighting = "Lighting";
  constexpr char const* PostProcess = "PostProcess";
  constexpr char const* Present = "Present";
}

/// Builtin material names
namespace MaterialNames {
  constexpr char const* GBufferDefault = "builtin/gbuffer_default";
  constexpr char const* LightingDeferred = "builtin/lighting_deferred";
  constexpr char const* PostProcessToneMap = "builtin/postprocess_tonemap";
  constexpr char const* PresentCopy = "builtin/present_copy";
}

/// Builtin mesh names
namespace MeshNames {
  constexpr char const* FullscreenQuad = "builtin/fullscreen_quad";
  constexpr char const* Sphere = "builtin/sphere";
  constexpr char const* Cone = "builtin/cone";
}

}  // namespace pipeline
}  // namespace te

#endif  // TE_PIPELINE_PASS_BUILDERS_H
