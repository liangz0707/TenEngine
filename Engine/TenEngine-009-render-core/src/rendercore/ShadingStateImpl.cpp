/** @file ShadingStateImpl.cpp */
#include <te/rendercore/impl/ShadingStateImpl.hpp>
#include <te/rendercore/IRenderPipelineState.hpp>
#include <te/rendercore/IShaderEntry.hpp>

namespace te {
namespace rendercore {

ShadingStateImpl::ShadingStateImpl(IRenderPipelineState* pipelineState, IShaderEntry* shaderEntry)
    : pipelineState_(pipelineState), shaderEntry_(shaderEntry) {}

ShadingStateImpl::~ShadingStateImpl() {
  delete shaderEntry_;
  shaderEntry_ = nullptr;
  delete pipelineState_;
  pipelineState_ = nullptr;
}

rhi::GraphicsPipelineStateDesc const* ShadingStateImpl::GetRHIStateDesc() const {
  return pipelineState_ ? pipelineState_->GetRHIStateDesc() : nullptr;
}

IRenderPipelineState const* ShadingStateImpl::GetPipelineState() const {
  return pipelineState_;
}

IShaderEntry const* ShadingStateImpl::GetShaderEntry() const {
  return shaderEntry_;
}

}  // namespace rendercore
}  // namespace te
