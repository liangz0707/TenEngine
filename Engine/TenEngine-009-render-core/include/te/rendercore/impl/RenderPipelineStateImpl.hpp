/** @file RenderPipelineStateImpl.hpp
 *  009-RenderCore: Implementation of IRenderPipelineState.
 */
#pragma once

#include <te/rendercore/IRenderPipelineState.hpp>
#include <te/rhi/pso.hpp>
#include <memory>

namespace te {
namespace rendercore {

class RenderPipelineStateImpl : public IRenderPipelineState {
 public:
  explicit RenderPipelineStateImpl(rhi::GraphicsPipelineStateDesc const& desc);
  rhi::GraphicsPipelineStateDesc const* GetRHIStateDesc() const override;

 private:
  std::unique_ptr<rhi::GraphicsPipelineStateDesc> desc_;
};

}  // namespace rendercore
}  // namespace te
