/** @file PipelineStateDesc.hpp
 *  009-RenderCore: Pipeline state description (blend, depth, raster); maps to RHI.
 */
#pragma once

#include <cstdint>

namespace te {
namespace rhi {
struct GraphicsPipelineStateDesc;
}
namespace rendercore {

/** Blend factor for color/alpha. */
enum class BlendFactor : std::uint8_t {
  Zero = 0, One, SrcColor, OneMinusSrcColor, DstColor, OneMinusDstColor,
  SrcAlpha, OneMinusSrcAlpha, DstAlpha, OneMinusDstAlpha,
};
/** Blend operation. */
enum class BlendOp : std::uint8_t {
  Add = 0, Subtract, ReverseSubtract, Min, Max,
};
/** Depth compare. */
enum class CompareOp : std::uint8_t {
  Never = 0, Less, Equal, LessOrEqual, Greater, NotEqual, GreaterOrEqual, Always,
};
/** Cull mode. */
enum class CullMode : std::uint8_t {
  None = 0, Front, Back, FrontAndBack,
};
/** Front face winding. */
enum class FrontFace : std::uint8_t {
  CounterClockwise = 0, Clockwise,
};

/** Per-attachment color blend. */
struct BlendAttachmentDesc {
  bool blendEnable = false;
  BlendFactor srcColorBlend = BlendFactor::SrcAlpha;
  BlendFactor dstColorBlend = BlendFactor::OneMinusSrcAlpha;
  BlendOp colorBlendOp = BlendOp::Add;
  BlendFactor srcAlphaBlend = BlendFactor::One;
  BlendFactor dstAlphaBlend = BlendFactor::Zero;
  BlendOp alphaBlendOp = BlendOp::Add;
  std::uint8_t colorWriteMask = 0xFu;
};

/** Depth state (stencil reserved). */
struct DepthStencilStateDesc {
  bool depthTestEnable = true;
  bool depthWriteEnable = true;
  CompareOp depthCompareOp = CompareOp::Less;
};

/** Rasterization state. */
struct RasterizationStateDesc {
  CullMode cullMode = CullMode::Back;
  FrontFace frontFace = FrontFace::CounterClockwise;
};

/** Pipeline state description; converted to RHI when creating PSO. */
struct PipelineStateDesc {
  bool blendEnable = false;
  static constexpr std::uint32_t kMaxBlendAttachments = 8u;
  BlendAttachmentDesc blendAttachments[kMaxBlendAttachments];
  std::uint32_t blendAttachmentCount = 0;
  DepthStencilStateDesc depthStencil;
  RasterizationStateDesc rasterization;
};

/** Convert to RHI description. */
void ConvertToRHI(PipelineStateDesc const& desc, rhi::GraphicsPipelineStateDesc& out);

}  // namespace rendercore
}  // namespace te
