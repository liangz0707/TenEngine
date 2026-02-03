/** @file swapchain.hpp
 *  008-RHI ABI: SwapChainDesc, ISwapChain.
 */
#pragma once

#include <te/rhi/types.hpp>
#include <cstdint>

namespace te {
namespace rhi {

struct SwapChainDesc {
  void*   windowHandle;
  uint32_t width;
  uint32_t height;
  uint32_t format;
  uint32_t bufferCount;
  bool     vsync;
};

struct ISwapChain {
  virtual bool Present() = 0;
  virtual ITexture* GetCurrentBackBuffer() = 0;
  virtual uint32_t GetCurrentBackBufferIndex() const = 0;
  virtual void Resize(uint32_t width, uint32_t height) = 0;
  virtual uint32_t GetWidth() const = 0;
  virtual uint32_t GetHeight() const = 0;
  virtual ~ISwapChain() = default;
};

}  // namespace rhi
}  // namespace te
