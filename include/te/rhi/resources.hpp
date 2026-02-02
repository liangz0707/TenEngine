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

/** Buffer usage bits; BufferDesc::usage is a bitmask of BufferUsage. */
enum class BufferUsage : uint32_t {
  Vertex  = 1u << 0,
  Index   = 1u << 1,
  Uniform = 1u << 2,
  Storage = 1u << 3,
  CopySrc = 1u << 4,
  CopyDst = 1u << 5,
};

/** Buffer creation descriptor. usage is a BufferUsage bitmask; include Uniform for constant buffers. */
struct BufferDesc {
  size_t   size;
  uint32_t usage;  // BufferUsage bitmask; Uniform bit required for CreateUniformBuffer etc.
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
