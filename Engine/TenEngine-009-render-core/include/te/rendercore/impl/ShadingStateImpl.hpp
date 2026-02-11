/** @file ShadingStateImpl.hpp
 *  009-RenderCore: IShadingState implementation; holds IRenderPipelineState + IShaderEntry.
 */
#pragma once

#include <te/rendercore/IShadingState.hpp>

namespace te {
namespace rendercore {

struct IRenderPipelineState;
struct IShaderEntry;

/** Holds pipeline state and shader entry; owns both (deletes in destructor). */
class ShadingStateImpl : public IShadingState {
 public:
  /** Takes ownership of pipelineState and shaderEntry. */
  ShadingStateImpl(IRenderPipelineState* pipelineState, IShaderEntry* shaderEntry);
  ~ShadingStateImpl() override;
  rhi::GraphicsPipelineStateDesc const* GetRHIStateDesc() const override;
  IRenderPipelineState const* GetPipelineState() const override;
  IShaderEntry const* GetShaderEntry() const override;

 private:
  IRenderPipelineState* pipelineState_;
  IShaderEntry* shaderEntry_;
};

}  // namespace rendercore
}  // namespace te
