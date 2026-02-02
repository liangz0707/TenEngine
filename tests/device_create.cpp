/**
 * @file device_create.cpp
 * @brief Test device creation, GetQueue, GetFeatures, SelectBackend (US1).
 * Tests Vulkan when TE_RHI_VULKAN; D3D12 on Windows when TE_RHI_D3D12; Metal on macOS when TE_RHI_METAL.
 * Unsupported backend returns nullptr; no auto-fallback.
 */
#include "te/rhi/device.hpp"
#include "te/rhi/sync.hpp"
#include "te/rhi/types.hpp"
#include <cassert>
#include <cstdio>

namespace {

/** Test one backend: create device, get queue, check features, destroy. */
void test_backend(te::rhi::Backend backend) {
  te::rhi::SelectBackend(backend);
  assert(te::rhi::GetSelectedBackend() == backend);

  te::rhi::IDevice* dev = te::rhi::CreateDevice(backend);
  if (!dev) {
    std::fprintf(stderr, "CreateDevice(%u) returned nullptr (backend may be unavailable)\n", static_cast<unsigned>(backend));
    return;
  }
  assert(dev->GetBackend() == backend);

  te::rhi::IQueue* q = dev->GetQueue(te::rhi::QueueType::Graphics, 0);
  assert(q != nullptr);
  assert(dev->GetQueue(te::rhi::QueueType::Graphics, 1) == nullptr);
  assert(dev->GetQueue(te::rhi::QueueType::Compute, 0) == nullptr);

  te::rhi::DeviceFeatures const& feats = dev->GetFeatures();
  assert(feats.maxTextureDimension2D > 0);
  assert(feats.maxTextureDimension3D > 0);

  te::rhi::DeviceLimits const& limits = dev->GetLimits();
  (void)limits;

  te::rhi::IFence* fenceFalse = dev->CreateFence(false);
  if (fenceFalse) {
    fenceFalse->Reset();
    dev->DestroyFence(fenceFalse);
  }
  te::rhi::IFence* fenceTrue = dev->CreateFence(true);
  if (fenceTrue) {
    fenceTrue->Wait();
    fenceTrue->Reset();
    dev->DestroyFence(fenceTrue);
  }

  te::rhi::DestroyDevice(dev);
}

/** CreateDevice() with no args uses GetSelectedBackend(). */
void test_create_device_no_args() {
  te::rhi::SelectBackend(te::rhi::Backend::Vulkan);
  te::rhi::IDevice* dev = te::rhi::CreateDevice();
  if (dev) {
    assert(dev->GetBackend() == te::rhi::GetSelectedBackend());
    te::rhi::DestroyDevice(dev);
  }
}

/** Unsupported backend returns nullptr (no auto-fallback). */
void test_unsupported_returns_null() {
#if !(defined(TE_RHI_METAL) && TE_RHI_METAL)
  te::rhi::IDevice* dev = te::rhi::CreateDevice(te::rhi::Backend::Metal);
  assert(dev == nullptr);
#endif
}

}  // namespace

int main() {
  test_create_device_no_args();
  test_unsupported_returns_null();

#if defined(TE_RHI_VULKAN) && TE_RHI_VULKAN
  test_backend(te::rhi::Backend::Vulkan);
#endif

#if defined(TE_RHI_D3D12) && TE_RHI_D3D12 && defined(_WIN32)
  test_backend(te::rhi::Backend::D3D12);
#endif

#if defined(TE_RHI_METAL) && TE_RHI_METAL && defined(__APPLE__)
  test_backend(te::rhi::Backend::Metal);
#endif

#if defined(TE_RHI_D3D11) && TE_RHI_D3D11 && defined(_WIN32)
  test_backend(te::rhi::Backend::D3D11);
#endif

  te::rhi::DestroyDevice(nullptr);
  return 0;
}
