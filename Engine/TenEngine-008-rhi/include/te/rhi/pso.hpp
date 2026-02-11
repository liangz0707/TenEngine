/** @file pso.hpp
 *  008-RHI ABI: GraphicsPSODesc, ComputePSODesc, IPSO, pipeline state (blend/depth/raster).
 */
#pragma once

#include <te/rhi/types.hpp>
#include <cstddef>
#include <cstdint>

namespace te {
namespace rhi {

/** Blend factor for color/alpha. Maps to VkBlendFactor / D3D11 blend op. */
enum class BlendFactor : uint8_t {
  Zero = 0,
  One,
  SrcColor,
  OneMinusSrcColor,
  DstColor,
  OneMinusDstColor,
  SrcAlpha,
  OneMinusSrcAlpha,
  DstAlpha,
  OneMinusDstAlpha,
};

/** Blend operation. Maps to VkBlendOp / D3D11. */
enum class BlendOp : uint8_t {
  Add = 0,
  Subtract,
  ReverseSubtract,
  Min,
  Max,
};

/** Depth/stencil compare. Maps to VkCompareOp / D3D11. */
enum class CompareOp : uint8_t {
  Never = 0,
  Less,
  Equal,
  LessOrEqual,
  Greater,
  NotEqual,
  GreaterOrEqual,
  Always,
};

/** Cull mode. Maps to VkCullModeFlags / D3D11. */
enum class CullMode : uint8_t {
  None = 0,
  Front,
  Back,
  FrontAndBack,
};

/** Front face winding. Maps to VkFrontFace / D3D11. */
enum class FrontFace : uint8_t {
  CounterClockwise = 0,
  Clockwise,
};

/** Per-attachment color blend. */
struct BlendAttachmentDesc {
  bool       blendEnable = false;
  BlendFactor srcColorBlend = BlendFactor::SrcAlpha;
  BlendFactor dstColorBlend = BlendFactor::OneMinusSrcAlpha;
  BlendOp    colorBlendOp = BlendOp::Add;
  BlendFactor srcAlphaBlend = BlendFactor::One;
  BlendFactor dstAlphaBlend = BlendFactor::Zero;
  BlendOp    alphaBlendOp = BlendOp::Add;
  uint8_t    colorWriteMask = 0xFu;  /* R=1, G=2, B=4, A=8 */
};

/** Depth/stencil state (depth only for now). */
struct DepthStencilStateDesc {
  bool      depthTestEnable = true;
  bool      depthWriteEnable = true;
  CompareOp depthCompareOp = CompareOp::Less;
};

/** Rasterization state. */
struct RasterizationStateDesc {
  CullMode  cullMode = CullMode::Back;
  FrontFace frontFace = FrontFace::CounterClockwise;
};

/** Optional graphics pipeline state; null in GraphicsPSODesc = backend defaults. */
struct GraphicsPipelineStateDesc {
  static constexpr uint32_t kMaxBlendAttachments = 8u;
  BlendAttachmentDesc    blendAttachments[kMaxBlendAttachments];
  uint32_t               blendAttachmentCount = 0;
  DepthStencilStateDesc  depthStencil;
  RasterizationStateDesc rasterization;
};

struct GraphicsPSODesc {
  void const* vertex_shader;
  size_t      vertex_shader_size;
  void const* fragment_shader;
  size_t      fragment_shader_size;
  /** Optional. When null, backend uses default blend/depth/raster. */
  GraphicsPipelineStateDesc const* pipelineState = nullptr;
};

struct ComputePSODesc {
  void const* compute_shader;
  size_t      compute_shader_size;
};

struct IPSO {
  virtual ~IPSO() = default;
};

}  // namespace rhi
}  // namespace te
