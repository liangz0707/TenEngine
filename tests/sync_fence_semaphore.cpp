/**
 * @file sync_fence_semaphore.cpp
 * @brief Test CreateFence, CreateSemaphore, Wait, Signal.
 */
#include "te/rhi/device.hpp"
#include "te/rhi/sync.hpp"
#include "te/rhi/types.hpp"
#include <cassert>

int main() {
  auto* dev = te::rhi::CreateDevice(te::rhi::Backend::Vulkan);
  assert(dev);

  auto* fence = dev->CreateFence();
  assert(fence);
  te::rhi::Signal(fence);
  te::rhi::Wait(fence);
  dev->DestroyFence(fence);

  auto* sem = dev->CreateSemaphore();
  assert(sem);
  dev->DestroySemaphore(sem);

  te::rhi::DestroyDevice(dev);
  return 0;
}
