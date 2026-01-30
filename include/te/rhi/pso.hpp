/**
 * @file pso.hpp
 * @brief RHI pipeline state object (contract: specs/_contracts/008-rhi-public-api.md section 6).
 */
#ifndef TE_RHI_PSO_HPP
#define TE_RHI_PSO_HPP

#include "te/rhi/types.hpp"
#include <cstddef>

namespace te {
namespace rhi {

struct GraphicsPSODesc {
  void const* vertex_shader{nullptr};
  size_t vertex_shader_size{0};
  void const* fragment_shader{nullptr};
  size_t fragment_shader_size{0};
};

struct ComputePSODesc {
  void const* compute_shader{nullptr};
  size_t compute_shader_size{0};
};

struct IPSO {
  virtual ~IPSO() = default;
};

}  // namespace rhi
}  // namespace te

#endif  // TE_RHI_PSO_HPP
