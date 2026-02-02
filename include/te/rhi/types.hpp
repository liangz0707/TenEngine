/** @file types.hpp
 *  008-RHI ABI: Backend, DeviceLimits, QueueType, DeviceFeatures, ResourceState,
 *  BufferBarrier, TextureBarrier, and forward declarations.
 */
#pragma once

#include <cstdint>
#include <cstddef>

namespace te {
namespace rhi {

enum class Backend : unsigned {
  Vulkan = 0,
  D3D12  = 1,
  Metal  = 2,
  D3D11  = 3,
};

struct DeviceLimits {
  size_t   maxBufferSize;
  uint32_t maxTextureDimension2D;
  uint32_t maxTextureDimension3D;
  size_t   minUniformBufferOffsetAlignment;
};

enum class QueueType : unsigned {
  Graphics = 0,
  Compute  = 1,
  Copy     = 2,
};

struct DeviceFeatures {
  uint32_t maxTextureDimension2D;
  uint32_t maxTextureDimension3D;
};

enum class ResourceState : uint32_t {
  Common        = 0,
  VertexBuffer  = 1,
  IndexBuffer   = 2,
  RenderTarget  = 3,
  DepthWrite    = 4,
  ShaderResource = 5,
  CopySrc       = 6,
  CopyDst       = 7,
  Present       = 8,
};

// Forward declarations
struct IBuffer;
struct ITexture;
struct ISampler;
struct IDevice;
struct IQueue;
struct ICommandList;
struct IPSO;
struct IFence;
struct ISemaphore;
struct ISwapChain;

struct BufferBarrier {
  IBuffer*       buffer;
  size_t         offset;
  size_t         size;
  ResourceState  srcState;
  ResourceState  dstState;
};

struct TextureBarrier {
  ITexture*      texture;
  uint32_t       mipLevel;
  uint32_t       arrayLayer;
  ResourceState  srcState;
  ResourceState  dstState;
};

}  // namespace rhi
}  // namespace te
