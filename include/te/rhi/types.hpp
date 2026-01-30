/**
 * @file types.hpp
 * @brief RHI type definitions (contract: specs/_contracts/008-rhi-public-api.md).
 * Only types declared in the contract API sketch are exposed.
 */
#ifndef TE_RHI_TYPES_HPP
#define TE_RHI_TYPES_HPP

#include <cstdint>

namespace te {
namespace rhi {

/** Backend enum per contract API sketch section 1. */
enum class Backend : unsigned {
  Vulkan = 0,
  D3D12  = 1,
  Metal  = 2,
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

}  // namespace rhi
}  // namespace te

#endif  // TE_RHI_TYPES_HPP
