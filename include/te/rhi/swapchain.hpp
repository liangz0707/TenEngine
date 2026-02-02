/**
 * @file swapchain.hpp
 * @brief RHI swapchain (contract: specs/_contracts/008-rhi-ABI.md, 008-rhi-public-api.md).
 */
#ifndef TE_RHI_SWAPCHAIN_HPP
#define TE_RHI_SWAPCHAIN_HPP

#include "te/rhi/types.hpp"
#include <cstdint>

namespace te {
namespace rhi {

struct ITexture;

/** Swap chain creation descriptor. */
struct SwapChainDesc {
  void* windowHandle{nullptr};
  uint32_t width{0};
  uint32_t height{0};
  uint32_t format{0};
  uint32_t bufferCount{2};
  bool vsync{true};
};

/** Swap chain; Present, GetCurrentBackBuffer, Resize (ABI: ISwapChain). */
struct ISwapChain {
  virtual ~ISwapChain() = default;
  virtual bool Present() = 0;
  virtual ITexture* GetCurrentBackBuffer() = 0;
  virtual uint32_t GetCurrentBackBufferIndex() const = 0;
  virtual void Resize(uint32_t width, uint32_t height) = 0;
  virtual uint32_t GetWidth() const = 0;
  virtual uint32_t GetHeight() const = 0;
};

}  // namespace rhi
}  // namespace te

#endif  // TE_RHI_SWAPCHAIN_HPP
