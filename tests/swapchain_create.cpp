/**
 * @file swapchain_create.cpp
 * @brief Test CreateSwapChain, Present, GetCurrentBackBuffer, Resize (no window).
 * Tests real backend swapchain path: Vulkan (TE_RHI_VULKAN), D3D12 on Windows (TE_RHI_D3D12), Metal on macOS (TE_RHI_METAL).
 * Without window: CreateSwapChain returns non-null with width/height; GetCurrentBackBuffer may be nullptr; Present returns true.
 * With window (optional): pass SwapChainDesc.windowHandle and test real present; conditionally compile if needed.
 */
#include "te/rhi/device.hpp"
#include "te/rhi/swapchain.hpp"
#include "te/rhi/types.hpp"
#include <cassert>

namespace {

void test_backend(te::rhi::Backend backend) {
  te::rhi::IDevice* dev = te::rhi::CreateDevice(backend);
  if (!dev) return;
  assert(dev->GetBackend() == backend);

  te::rhi::SwapChainDesc desc{};
  desc.windowHandle = nullptr;
  desc.width = 800;
  desc.height = 600;
  desc.bufferCount = 2;
  desc.vsync = true;

  te::rhi::ISwapChain* sc = dev->CreateSwapChain(desc);
  assert(sc != nullptr);
  assert(sc->GetWidth() == 800);
  assert(sc->GetHeight() == 600);
  assert(sc->Present());
  (void)sc->GetCurrentBackBuffer();
  (void)sc->GetCurrentBackBufferIndex();
  sc->Resize(640, 480);
  assert(sc->GetWidth() == 640);
  assert(sc->GetHeight() == 480);

  te::rhi::DestroyDevice(dev);
}

void test_failure_returns_null(te::rhi::Backend backend) {
  te::rhi::IDevice* dev = te::rhi::CreateDevice(backend);
  if (!dev) return;

  te::rhi::SwapChainDesc desc{};
  desc.width = 0;
  desc.height = 600;
  assert(dev->CreateSwapChain(desc) == nullptr);
  desc.width = 800;
  desc.height = 0;
  assert(dev->CreateSwapChain(desc) == nullptr);

  te::rhi::DestroyDevice(dev);
}

}  // namespace

int main() {
#if defined(TE_RHI_VULKAN) && TE_RHI_VULKAN
  test_backend(te::rhi::Backend::Vulkan);
  test_failure_returns_null(te::rhi::Backend::Vulkan);
#endif

#if defined(TE_RHI_D3D12) && TE_RHI_D3D12 && defined(_WIN32)
  test_backend(te::rhi::Backend::D3D12);
  test_failure_returns_null(te::rhi::Backend::D3D12);
#endif

#if defined(TE_RHI_METAL) && TE_RHI_METAL && defined(__APPLE__)
  test_backend(te::rhi::Backend::Metal);
  test_failure_returns_null(te::rhi::Backend::Metal);
#endif

  return 0;
}
