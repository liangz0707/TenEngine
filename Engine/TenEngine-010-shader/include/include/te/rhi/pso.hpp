/** @file pso.hpp
 *  008-RHI ABI: GraphicsPSODesc, ComputePSODesc, IPSO.
 */
#pragma once

#include <te/rhi/types.hpp>
#include <cstddef>

namespace te {
namespace rhi {

struct GraphicsPSODesc {
  void const* vertex_shader;
  size_t      vertex_shader_size;
  void const* fragment_shader;
  size_t      fragment_shader_size;
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
