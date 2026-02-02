/**
 * @file types.hpp
 * @brief RHI type definitions (contract: specs/_contracts/008-rhi-public-api.md).
 * Only types declared in the contract API sketch are exposed.
 */
#ifndef TE_RHI_TYPES_HPP
#define TE_RHI_TYPES_HPP

#include <cstddef>
#include <cstdint>

namespace te {
namespace rhi {

/** Backend enum per contract API sketch section 1. */
enum class Backend : unsigned {
  Vulkan = 0,
  D3D12  = 1,
  Metal  = 2,
  D3D11  = 3,
};

/** Queue type per contract API sketch section 2. */
enum class QueueType : unsigned {
  Graphics = 0,
  Compute  = 1,
  Copy     = 2,
};

/** Device features per contract API sketch section 3; minimal set. */
struct DeviceFeatures {
  uint32_t maxTextureDimension2D{0};
  uint32_t maxTextureDimension3D{0};
};

/** Device limits (ABI TODO); query from backend where applicable. */
struct DeviceLimits {
  size_t maxBufferSize{0};
  uint32_t maxTextureDimension2D{0};
  uint32_t maxTextureDimension3D{0};
  uint32_t minUniformBufferOffsetAlignment{0};
};

/** Resource state for barrier (fine-grained per spec clarification). */
enum class ResourceState : uint32_t {
  Common = 0,
  VertexBuffer,
  IndexBuffer,
  RenderTarget,
  DepthWrite,
  ShaderResource,
  CopySrc,
  CopyDst,
  Present,
};

// Forward declarations for barrier structs
struct IBuffer;
struct ITexture;

/** Buffer barrier (fine-grained: per-resource + state transition). */
struct BufferBarrier {
  IBuffer* buffer{nullptr};
  size_t offset{0};
  size_t size{0};
  ResourceState srcState{ResourceState::Common};
  ResourceState dstState{ResourceState::Common};
};

/** Texture barrier (fine-grained: per-resource + state transition). */
struct TextureBarrier {
  ITexture* texture{nullptr};
  uint32_t mipLevel{0};
  uint32_t arrayLayer{0};
  ResourceState srcState{ResourceState::Common};
  ResourceState dstState{ResourceState::Common};
};

// Forward declarations per contract API sketch sections 1-7
struct IDevice;
struct IQueue;
struct ICommandList;
struct IBuffer;
struct ITexture;
struct ISampler;
struct IPSO;
struct IFence;
struct ISemaphore;
struct ISwapChain;

}  // namespace rhi
}  // namespace te

#endif  // TE_RHI_TYPES_HPP
