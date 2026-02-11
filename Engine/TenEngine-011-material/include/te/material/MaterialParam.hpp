/**
 * @file MaterialParam.hpp
 * @brief Material parameter: scalar, vector, matrix, 1D array; aligns with rendercore::UniformMemberType for UB upload.
 */
#ifndef TE_MATERIAL_MATERIAL_PARAM_HPP
#define TE_MATERIAL_MATERIAL_PARAM_HPP

#include <te/rendercore/uniform_layout.hpp>
#include <cstdint>
#include <vector>
#include <cstring>

namespace te {
namespace material {

/** Single uniform parameter: type, element count (0 or 1 = scalar, N = 1D array), and raw bytes (std140 layout). */
struct MaterialParam {
  te::rendercore::UniformMemberType type = te::rendercore::UniformMemberType::Unknown;
  std::uint32_t count = 0;  /* 0 or 1 = scalar; N = array of N elements */
  std::vector<std::uint8_t> data;

  bool IsValid() const {
    if (type == te::rendercore::UniformMemberType::Unknown) return false;
    std::size_t elemSize = GetElementSize(type);
    if (elemSize == 0) return false;
    std::size_t n = (count == 0) ? 1u : count;
    return data.size() >= elemSize * n;
  }

  /** Size in bytes of one element for the given type (std140). */
  static std::size_t GetElementSize(te::rendercore::UniformMemberType t) {
    switch (t) {
      case te::rendercore::UniformMemberType::Float:  return 4;
      case te::rendercore::UniformMemberType::Float2: return 8;
      case te::rendercore::UniformMemberType::Float3: return 12;
      case te::rendercore::UniformMemberType::Float4: return 16;
      case te::rendercore::UniformMemberType::Mat3:   return 48;   /* 3 * float4 */
      case te::rendercore::UniformMemberType::Mat4:   return 64;
      case te::rendercore::UniformMemberType::Int:    return 4;
      case te::rendercore::UniformMemberType::Int2:   return 8;
      case te::rendercore::UniformMemberType::Int3:   return 12;
      case te::rendercore::UniformMemberType::Int4:   return 16;
      default: return 0;
    }
  }

  std::size_t GetTotalSize() const {
    std::size_t elem = GetElementSize(type);
    std::size_t n = (count == 0) ? 1u : count;
    return elem * n;
  }
};

/** Blend factor for color/alpha (pipeline state description). */
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
  bool        blendEnable = false;
  BlendFactor srcColorBlend = BlendFactor::SrcAlpha;
  BlendFactor dstColorBlend = BlendFactor::OneMinusSrcAlpha;
  BlendOp     colorBlendOp = BlendOp::Add;
  BlendFactor srcAlphaBlend = BlendFactor::One;
  BlendFactor dstAlphaBlend = BlendFactor::Zero;
  BlendOp     alphaBlendOp = BlendOp::Add;
  std::uint8_t colorWriteMask = 0xFu;
};

/** Depth state (stencil reserved). */
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

/** Pipeline state description (blend, depth, cull, etc.); CPU-side only. */
struct PipelineStateDesc {
  bool blendEnable = false;  /* Legacy: when blendAttachmentCount==0, enables first attachment. */
  static constexpr std::uint32_t kMaxBlendAttachments = 8u;
  BlendAttachmentDesc    blendAttachments[kMaxBlendAttachments];
  std::uint32_t           blendAttachmentCount = 0;
  DepthStencilStateDesc   depthStencil;
  RasterizationStateDesc  rasterization;
};

}  // namespace material
}  // namespace te

#endif
