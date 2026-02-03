/** @file pso_create.cpp
 *  US4 test: CreateGraphicsPSO/CreateComputePSO (minimal desc); invalid can return nullptr.
 */
#include <te/rhi/device.hpp>
#include <te/rhi/pso.hpp>
#include <te/rhi/types.hpp>
#include <cstdio>

int main() {
  using namespace te::rhi;
  IDevice* dev = CreateDevice();
  if (!dev) {
    std::printf("CreateDevice() nullptr; skip pso_create\n");
    return 0;
  }
  GraphicsPSODesc gd = {};
  gd.vertex_shader = nullptr;
  gd.vertex_shader_size = 0;
  gd.fragment_shader = nullptr;
  gd.fragment_shader_size = 0;
  IPSO* gpso = dev->CreateGraphicsPSO(gd);
  if (gpso) dev->DestroyPSO(gpso);
  ComputePSODesc cd = {};
  cd.compute_shader = nullptr;
  cd.compute_shader_size = 0;
  IPSO* cpso = dev->CreateComputePSO(cd);
  if (cpso) dev->DestroyPSO(cpso);
  DestroyDevice(dev);
  std::printf("pso_create passed\n");
  return 0;
}
