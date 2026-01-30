/**
 * @file pso_create.cpp
 * @brief Test CreateGraphicsPSO, CreateComputePSO, SetShader, Cache.
 */
#include "te/rhi/device.hpp"
#include "te/rhi/pso.hpp"
#include "te/rhi/types.hpp"
#include <cassert>

int main() {
  auto* dev = te::rhi::CreateDevice(te::rhi::Backend::Vulkan);
  assert(dev);

  unsigned char vs[] = {0x01, 0x02};
  te::rhi::GraphicsPSODesc gd;
  gd.vertex_shader = vs;
  gd.vertex_shader_size = sizeof(vs);
  auto* gpso = dev->CreateGraphicsPSO(gd);
  assert(gpso);
  dev->SetShader(gpso, vs, sizeof(vs));
  dev->Cache(gpso);
  dev->DestroyPSO(gpso);

  te::rhi::ComputePSODesc cd;
  cd.compute_shader = vs;
  cd.compute_shader_size = sizeof(vs);
  auto* cpso = dev->CreateComputePSO(cd);
  assert(cpso);
  dev->DestroyPSO(cpso);

  te::rhi::DestroyDevice(dev);
  return 0;
}
