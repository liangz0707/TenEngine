/** @file IRenderPipelineState.hpp
 *  009-RenderCore: Render state interface; maps directly to RHI.
 */
#pragma once

namespace te {
namespace rhi {
struct GraphicsPipelineStateDesc;
}
namespace rendercore {

/** Render pipeline state (blend, depth, raster); maps directly to RHI for PSO creation. */
struct IRenderPipelineState {
  virtual ~IRenderPipelineState() = default;
  /** Returns the RHI pipeline state description. Must not be null. */
  virtual rhi::GraphicsPipelineStateDesc const* GetRHIStateDesc() const = 0;
};

}  // namespace rendercore
}  // namespace te
