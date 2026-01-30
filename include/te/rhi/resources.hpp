/**
 * @file resources.hpp
 * @brief RHI resources (contract: specs/_contracts/008-rhi-public-api.md section 5).
 */
#ifndef TE_RHI_RESOURCES_HPP
#define TE_RHI_RESOURCES_HPP

#include "te/rhi/types.hpp"
#include <cstddef>

namespace te {
namespace rhi {

struct BufferDesc {
  size_t size{0};
  uint32_t usage{0};
};

struct TextureDesc {
  uint32_t width{0};
  uint32_t height{0};
  uint32_t depth{1};
  uint32_t format{0};
};

struct SamplerDesc {
  uint32_t filter{0};
};

struct ViewDesc {
  void* resource{nullptr};
  uint32_t type{0};
};

using ViewHandle = uintptr_t;

struct IBuffer {
  virtual ~IBuffer() = default;
};

struct ITexture {
  virtual ~ITexture() = default;
};

struct ISampler {
  virtual ~ISampler() = default;
};

}  // namespace rhi
}  // namespace te

#endif  // TE_RHI_RESOURCES_HPP
