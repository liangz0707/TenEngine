/** @file IShadingState.hpp
 *  009-RenderCore: Shading state = IRenderPipelineState + IShaderEntry.
 */
#pragma once

#include <te/rendercore/IRenderPipelineState.hpp>

namespace te {
namespace rendercore {

struct IShaderEntry;

/** Pipeline state plus shader entry; used to create PSO. */
struct IShadingState : IRenderPipelineState {
  ~IShadingState() override = default;
  virtual IRenderPipelineState const* GetPipelineState() const = 0;
  virtual IShaderEntry const* GetShaderEntry() const = 0;
};

}  // namespace rendercore
}  // namespace te
