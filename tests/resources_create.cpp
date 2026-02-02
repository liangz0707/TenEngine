/**
 * @file resources_create.cpp
 * @brief Test CreateBuffer, CreateTexture, CreateSampler, CreateView, Destroy (US3).
 * Covers real backend allocation: Vulkan (TE_RHI_VULKAN), D3D12/D3D11 on Windows (TE_RHI_D3D12/TE_RHI_D3D11), Metal on macOS (TE_RHI_METAL).
 * Verifies handles valid; Destroy then no use; failure paths return nullptr.
 */
#include "te/rhi/device.hpp"
#include "te/rhi/resources.hpp"
#include "te/rhi/types.hpp"
#include <cassert>

namespace {

void test_backend(te::rhi::Backend backend) {
  te::rhi::IDevice* dev = te::rhi::CreateDevice(backend);
  if (!dev) return;
  assert(dev->GetBackend() == backend);

  te::rhi::BufferDesc bd;
  bd.size = 256;
  bd.usage = 0;
  te::rhi::IBuffer* buf = dev->CreateBuffer(bd);
  assert(buf != nullptr);
  dev->DestroyBuffer(buf);
  buf = nullptr;

  te::rhi::TextureDesc td;
  td.width = 64;
  td.height = 64;
  td.depth = 1;
  td.format = 0;
  te::rhi::ITexture* tex = dev->CreateTexture(td);
  assert(tex != nullptr);
  dev->DestroyTexture(tex);
  tex = nullptr;

  te::rhi::SamplerDesc sd;
  sd.filter = 0;
  te::rhi::ISampler* samp = dev->CreateSampler(sd);
  assert(samp != nullptr);
  dev->DestroySampler(samp);
  samp = nullptr;

  te::rhi::DestroyDevice(dev);
}

void test_failure_returns_null(te::rhi::Backend backend) {
  te::rhi::IDevice* dev = te::rhi::CreateDevice(backend);
  if (!dev) return;

  te::rhi::BufferDesc bd_zero;
  bd_zero.size = 0;
  assert(dev->CreateBuffer(bd_zero) == nullptr);

  te::rhi::TextureDesc td_zero;
  td_zero.width = 0;
  td_zero.height = 64;
  assert(dev->CreateTexture(td_zero) == nullptr);
  td_zero.width = 64;
  td_zero.height = 0;
  assert(dev->CreateTexture(td_zero) == nullptr);

  te::rhi::DestroyDevice(dev);
}

void test_create_view(te::rhi::Backend backend) {
  te::rhi::IDevice* dev = te::rhi::CreateDevice(backend);
  if (!dev) return;

  te::rhi::BufferDesc bd;
  bd.size = 128;
  te::rhi::IBuffer* buf = dev->CreateBuffer(bd);
  assert(buf != nullptr);
  te::rhi::ViewDesc vd;
  vd.resource = buf;
  vd.type = 0;
  te::rhi::ViewHandle h = dev->CreateView(vd);
  (void)h;
  dev->DestroyBuffer(buf);
  te::rhi::DestroyDevice(dev);
}

}  // namespace

int main() {
#if defined(TE_RHI_VULKAN) && TE_RHI_VULKAN
  test_backend(te::rhi::Backend::Vulkan);
  test_failure_returns_null(te::rhi::Backend::Vulkan);
  test_create_view(te::rhi::Backend::Vulkan);
#endif

#if defined(TE_RHI_D3D12) && TE_RHI_D3D12 && defined(_WIN32)
  test_backend(te::rhi::Backend::D3D12);
  test_failure_returns_null(te::rhi::Backend::D3D12);
  test_create_view(te::rhi::Backend::D3D12);
#endif

#if defined(TE_RHI_METAL) && TE_RHI_METAL && defined(__APPLE__)
  test_backend(te::rhi::Backend::Metal);
  test_failure_returns_null(te::rhi::Backend::Metal);
  test_create_view(te::rhi::Backend::Metal);
#endif

#if defined(TE_RHI_D3D11) && TE_RHI_D3D11 && defined(_WIN32)
  test_backend(te::rhi::Backend::D3D11);
  test_failure_returns_null(te::rhi::Backend::D3D11);
  test_create_view(te::rhi::Backend::D3D11);
#endif

  return 0;
}
