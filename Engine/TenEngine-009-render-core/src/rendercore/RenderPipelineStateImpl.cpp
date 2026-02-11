/** @file RenderPipelineStateImpl.cpp */
#include <te/rendercore/impl/RenderPipelineStateImpl.hpp>
#include <cstring>

namespace te {
namespace rendercore {

RenderPipelineStateImpl::RenderPipelineStateImpl(rhi::GraphicsPipelineStateDesc const& desc)
    : desc_(new rhi::GraphicsPipelineStateDesc(desc)) {}

rhi::GraphicsPipelineStateDesc const* RenderPipelineStateImpl::GetRHIStateDesc() const {
  return desc_.get();
}

}  // namespace rendercore
}  // namespace te
