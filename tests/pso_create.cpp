/**
 * @file pso_create.cpp
 * @brief Test CreateGraphicsPSO, CreateComputePSO, SetShader, Cache (US4).
 * Tests real backend PSO path: Vulkan (TE_RHI_VULKAN), D3D12 on Windows (TE_RHI_D3D12), Metal on macOS (TE_RHI_METAL).
 * Uses minimal shader bytecode; verifies non-null; failure (e.g. invalid bytecode) may return nullptr.
 */
#include "te/rhi/device.hpp"
#include "te/rhi/pso.hpp"
#include "te/rhi/types.hpp"
#include <cassert>

namespace {

/** Minimal SPIR-V header (magic, version 1.0, generator 0, bound 0, reserved 0). Vulkan accepts this for VkShaderModule. */
static unsigned char const kMinimalSpirv[] = {
  0x03, 0x02, 0x23, 0x07,  // magic
  0x00, 0x00, 0x01, 0x00,  // version 1.0
  0x00, 0x00, 0x00, 0x00,  // generator
  0x00, 0x00, 0x00, 0x00,  // bound
  0x00, 0x00, 0x00, 0x00,  // instruction stream (empty)
};

void test_backend(te::rhi::Backend backend) {
  te::rhi::IDevice* dev = te::rhi::CreateDevice(backend);
  if (!dev) return;
  assert(dev->GetBackend() == backend);

  te::rhi::GraphicsPSODesc gd;
  gd.vertex_shader = kMinimalSpirv;
  gd.vertex_shader_size = sizeof(kMinimalSpirv);
  gd.fragment_shader = nullptr;
  gd.fragment_shader_size = 0;
  te::rhi::IPSO* gpso = dev->CreateGraphicsPSO(gd);
  assert(gpso != nullptr);
  dev->SetShader(gpso, kMinimalSpirv, sizeof(kMinimalSpirv));
  dev->Cache(gpso);
  dev->DestroyPSO(gpso);

  te::rhi::ComputePSODesc cd;
  cd.compute_shader = kMinimalSpirv;
  cd.compute_shader_size = sizeof(kMinimalSpirv);
  te::rhi::IPSO* cpso = dev->CreateComputePSO(cd);
  assert(cpso != nullptr);
  dev->DestroyPSO(cpso);

  te::rhi::DestroyDevice(dev);
}

void test_failure_returns_null(te::rhi::Backend backend) {
  te::rhi::IDevice* dev = te::rhi::CreateDevice(backend);
  if (!dev) return;

  te::rhi::GraphicsPSODesc gd;
  gd.vertex_shader = nullptr;
  gd.vertex_shader_size = 0;
  assert(dev->CreateGraphicsPSO(gd) == nullptr);

  te::rhi::ComputePSODesc cd;
  cd.compute_shader = nullptr;
  cd.compute_shader_size = 0;
  assert(dev->CreateComputePSO(cd) == nullptr);

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
