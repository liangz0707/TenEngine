/**
 * @file device_create.cpp
 * @brief Test device creation, GetQueue, GetFeatures, SelectBackend.
 */
#include "te/rhi/device.hpp"
#include "te/rhi/types.hpp"
#include <cassert>

int main() {
  te::rhi::SelectBackend(te::rhi::Backend::Vulkan);
  assert(te::rhi::GetSelectedBackend() == te::rhi::Backend::Vulkan);

  auto* dev = te::rhi::CreateDevice(te::rhi::Backend::Vulkan);
  assert(dev != nullptr);

  auto* q = dev->GetQueue(te::rhi::QueueType::Graphics, 0);
  assert(q != nullptr);
  assert(dev->GetQueue(te::rhi::QueueType::Graphics, 1) == nullptr);
  assert(dev->GetQueue(te::rhi::QueueType::Compute, 0) == nullptr);

  auto const& feats = dev->GetFeatures();
  assert(feats.maxTextureDimension2D > 0);
  assert(feats.maxTextureDimension3D > 0);

  te::rhi::DestroyDevice(dev);
  te::rhi::DestroyDevice(nullptr);
  return 0;
}
