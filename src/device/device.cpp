/**
 * @file device.cpp
 * @brief RHI device implementation (stub backend for testing when Vulkan/D3D12/Metal unavailable).
 */
#include "te/rhi/device.hpp"
#include "te/rhi/types.hpp"
#include "te/core/alloc.h"
#include "te/core/log.h"
#include <cstddef>
#include <new>

namespace te {
namespace rhi {

namespace {

Backend g_selected_backend{Backend::Vulkan};
constexpr uint32_t kMaxQueuesPerType = 1u;

struct QueueStub : IQueue {};

struct DeviceStub : IDevice {
  DeviceFeatures features{4096, 256};
  QueueStub graphics_queue;

  IQueue* GetQueue(QueueType type, uint32_t index) override {
    if (index >= kMaxQueuesPerType) return nullptr;
    if (type == QueueType::Graphics) return &graphics_queue;
    if (type == QueueType::Compute || type == QueueType::Copy) return nullptr;
    return nullptr;
  }
  DeviceFeatures const& GetFeatures() const override { return features; }
  ICommandList* CreateCommandList() override { return nullptr; }
  void DestroyCommandList(ICommandList* cmd) override { (void)cmd; }
};

}  // namespace

void SelectBackend(Backend b) {
  g_selected_backend = b;
}

Backend GetSelectedBackend() {
  return g_selected_backend;
}

IDevice* CreateDevice(Backend backend) {
  (void)backend;
  auto* dev = static_cast<DeviceStub*>(core::Alloc(sizeof(DeviceStub), alignof(DeviceStub)));
  if (!dev) return nullptr;
  new (dev) DeviceStub();
  return dev;
}

IDevice* CreateDevice() {
  return CreateDevice(GetSelectedBackend());
}

void DestroyDevice(IDevice* device) {
  if (device) {
    static_cast<DeviceStub*>(device)->~DeviceStub();
    core::Free(device);
  }
}

}  // namespace rhi
}  // namespace te
