/** @file swapchain.hpp
 *  008-RHI ABI: SwapChainDesc, ISwapChain.
 */
#pragma once

#include <te/rhi/types.hpp>
#include <cstdint>

namespace te {
namespace rhi {

/// VSync mode
enum class VSyncMode : uint8_t {
  Off = 0,        // No VSync, unlimited FPS
  On = 1,         // VSync enabled
  Adaptive = 2,   // Adaptive VSync (sync when FPS >= refresh rate)
  Mailbox = 3,    // Mailbox mode (fast VSync, lowest latency)
};

/// Color space for swap chain
enum class ColorSpace : uint8_t {
  SRGB = 0,           // Standard sRGB
  HDR10_ST2084 = 1,   // HDR10 with PQ curve
  HDR10_HLG = 2,      // HDR10 with HLG curve
  scRGB = 3,          // scRGB linear (Windows)
  DisplayNative = 4,  // Display native color space
};

/// Swap chain present mode
enum class PresentMode : uint8_t {
  Immediate = 0,  // No waiting, may tear
  FIFO = 1,       // VSync, wait for vertical blank
  FIFO_Relaxed = 2, // VSync with tearing allowed when late
  Mailbox = 3,    // Triple buffering, no tearing
};

/// HDR metadata for HDR10
struct HDRMetadata {
  float primaryRedX{0.68f};
  float primaryRedY{0.32f};
  float primaryGreenX{0.265f};
  float primaryGreenY{0.69f};
  float primaryBlueX{0.15f};
  float primaryBlueY{0.06f};
  float whitePointX{0.3127f};
  float whitePointY{0.329f};
  float minLuminance{0.0f};
  float maxLuminance{1000.0f};
  float maxContentLightLevel{1000.0f};
  float maxFrameAverageLightLevel{400.0f};
};

struct SwapChainDesc {
  void*   windowHandle;
  uint32_t width;
  uint32_t height;
  uint32_t format;
  uint32_t bufferCount;
  bool     vsync;

  // Extended settings
  VSyncMode vsyncMode{VSyncMode::On};
  ColorSpace colorSpace{ColorSpace::SRGB};
  PresentMode presentMode{PresentMode::FIFO};
  bool enableHDR{false};
  bool allowTearing{false};
  HDRMetadata hdrMetadata{};
};

struct ISwapChain {
  virtual bool Present() = 0;
  virtual ITexture* GetCurrentBackBuffer() = 0;
  virtual uint32_t GetCurrentBackBufferIndex() const = 0;
  virtual void Resize(uint32_t width, uint32_t height) = 0;
  virtual uint32_t GetWidth() const = 0;
  virtual uint32_t GetHeight() const = 0;

  // Extended interface
  virtual bool SetVSyncMode(VSyncMode mode) { (void)mode; return false; }
  virtual VSyncMode GetVSyncMode() const { return VSyncMode::On; }
  virtual bool SetHDRMode(bool enable, ColorSpace colorSpace) {
    (void)enable; (void)colorSpace; return false;
  }
  virtual bool IsHDREnabled() const { return false; }
  virtual ColorSpace GetColorSpace() const { return ColorSpace::SRGB; }
  virtual bool SetHDRMetadata(HDRMetadata const& metadata) {
    (void)metadata; return false;
  }
  virtual bool SupportsHDR() const { return false; }
  virtual bool SupportsTearing() const { return false; }
  virtual uint32_t GetRefreshRate() const { return 60; }

  virtual ~ISwapChain() = default;

  // Backward compatibility
  ITexture* GetBackBuffer() { return GetCurrentBackBuffer(); }
};

}  // namespace rhi
}  // namespace te
