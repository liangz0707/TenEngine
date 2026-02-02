/** @file resources.hpp
 *  008-RHI ABI: BufferDesc, TextureDesc, SamplerDesc, ViewDesc, ViewHandle,
 *  IBuffer, ITexture, ISampler.
 */
#pragma once

#include <te/rhi/types.hpp>
#include <cstddef>
#include <cstdint>

namespace te {
namespace rhi {

struct BufferDesc {
  size_t  size;
  uint32_t usage;
};

struct TextureDesc {
  uint32_t width;
  uint32_t height;
  uint32_t depth;
  uint32_t format;
};

struct SamplerDesc {
  uint32_t filter;
};

struct ViewDesc {
  void*    resource;
  uint32_t type;
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
