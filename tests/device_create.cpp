/** @file device_create.cpp
 *  US1 test: CreateDevice(Backend), GetQueue, GetFeatures, GetLimits, DestroyDevice.
 */
#include <te/rhi/device.hpp>
#include <te/rhi/types.hpp>
#include <cassert>
#include <cstdio>

int main() {
  using namespace te::rhi;
  Backend backends[] = { Backend::Vulkan, Backend::D3D12, Backend::D3D11, Backend::Metal };
  for (Backend b : backends) {
    IDevice* dev = CreateDevice(b);
    if (!dev) {
      std::printf("CreateDevice(Backend %u) returned nullptr (backend may be disabled)\n", static_cast<unsigned>(b));
      continue;
    }
    assert(dev->GetBackend() == b);
    IQueue* q = dev->GetQueue(QueueType::Graphics, 0);
    if (q) {
      dev->GetLimits();
      dev->GetFeatures();
      q->WaitIdle();
    }
    DestroyDevice(dev);
    std::printf("Backend %u: CreateDevice/GetQueue/GetLimits/GetFeatures/DestroyDevice OK\n", static_cast<unsigned>(b));
  }
  SelectBackend(Backend::Vulkan);
  assert(GetSelectedBackend() == Backend::Vulkan);
  IDevice* def = CreateDevice();
  if (def) {
    assert(def->GetBackend() == Backend::Vulkan);
    DestroyDevice(def);
  }
  std::printf("device_create passed\n");
  return 0;
}
