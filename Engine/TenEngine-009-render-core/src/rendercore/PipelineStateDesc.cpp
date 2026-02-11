/** @file PipelineStateDesc.cpp
 *  009-RenderCore: Convert PipelineStateDesc to RHI.
 */
#include <te/rendercore/PipelineStateDesc.hpp>
#include <te/rhi/pso.hpp>
#include <cstdint>

namespace te {
namespace rendercore {

void ConvertToRHI(PipelineStateDesc const& desc, rhi::GraphicsPipelineStateDesc& out) {
  out.blendAttachmentCount = desc.blendAttachmentCount;
  if (out.blendAttachmentCount == 0 && desc.blendEnable)
    out.blendAttachmentCount = 1u;
  if (out.blendAttachmentCount > rhi::GraphicsPipelineStateDesc::kMaxBlendAttachments)
    out.blendAttachmentCount = rhi::GraphicsPipelineStateDesc::kMaxBlendAttachments;
  for (uint32_t i = 0; i < out.blendAttachmentCount; ++i) {
    rhi::BlendAttachmentDesc& d = out.blendAttachments[i];
    BlendAttachmentDesc const& s = desc.blendAttachments[i];
    d.blendEnable = s.blendEnable;
    d.srcColorBlend = static_cast<rhi::BlendFactor>(static_cast<std::uint8_t>(s.srcColorBlend));
    d.dstColorBlend = static_cast<rhi::BlendFactor>(static_cast<std::uint8_t>(s.dstColorBlend));
    d.colorBlendOp = static_cast<rhi::BlendOp>(static_cast<std::uint8_t>(s.colorBlendOp));
    d.srcAlphaBlend = static_cast<rhi::BlendFactor>(static_cast<std::uint8_t>(s.srcAlphaBlend));
    d.dstAlphaBlend = static_cast<rhi::BlendFactor>(static_cast<std::uint8_t>(s.dstAlphaBlend));
    d.alphaBlendOp = static_cast<rhi::BlendOp>(static_cast<std::uint8_t>(s.alphaBlendOp));
    d.colorWriteMask = s.colorWriteMask;
  }
  if (out.blendAttachmentCount == 1u && desc.blendEnable && desc.blendAttachmentCount == 0u)
    out.blendAttachments[0].blendEnable = true;
  out.depthStencil.depthTestEnable = desc.depthStencil.depthTestEnable;
  out.depthStencil.depthWriteEnable = desc.depthStencil.depthWriteEnable;
  out.depthStencil.depthCompareOp = static_cast<rhi::CompareOp>(static_cast<std::uint8_t>(desc.depthStencil.depthCompareOp));
  out.rasterization.cullMode = static_cast<rhi::CullMode>(static_cast<std::uint8_t>(desc.rasterization.cullMode));
  out.rasterization.frontFace = static_cast<rhi::FrontFace>(static_cast<std::uint8_t>(desc.rasterization.frontFace));
}

}  // namespace rendercore
}  // namespace te
