/** @file swapchain_create.cpp
 *  US6 test: CreateSwapChain(desc); Present/Resize (no window: desc.windowHandle nullptr may return nullptr).
 */
#include <te/rhi/device.hpp>
#include <te/rhi/swapchain.hpp>
#include <te/rhi/types.hpp>
#include <cstdio>

int main() {
  using namespace te::rhi;
  IDevice* dev = CreateDevice();
  if (!dev) {
    std::printf("CreateDevice() nullptr; skip swapchain_create\n");
    return 0;
  }
  SwapChainDesc desc = {};
  desc.windowHandle = nullptr;
  desc.width = 64;
  desc.height = 64;
  desc.format = 0;
  desc.bufferCount = 2;
  desc.vsync = false;
  ISwapChain* sc = dev->CreateSwapChain(desc);
  if (sc) {
    (void)sc->GetWidth();
    (void)sc->GetHeight();
    (void)sc->GetCurrentBackBufferIndex();
    sc->Resize(128, 128);
    // ABI has no DestroySwapChain; swapchain lifetime is caller-managed or tied to device.
  }
  DestroyDevice(dev);
  std::printf("swapchain_create passed\n");
  return 0;
}
