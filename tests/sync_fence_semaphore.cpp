/**
 * @file sync_fence_semaphore.cpp
 * @brief Test CreateFence, CreateSemaphore, Wait, Signal (US5).
 * Tests real backend sync: Vulkan (TE_RHI_VULKAN), D3D12/D3D11 on Windows (TE_RHI_D3D12/TE_RHI_D3D11), Metal on macOS (TE_RHI_METAL).
 * Submit command then Signal Fence, main thread Wait; verifies sync path (backends may use no-op Wait to avoid hang).
 */
#include "te/rhi/device.hpp"
#include "te/rhi/command_list.hpp"
#include "te/rhi/queue.hpp"
#include "te/rhi/sync.hpp"
#include "te/rhi/types.hpp"
#include <cassert>

namespace {

void test_backend(te::rhi::Backend backend) {
  te::rhi::IDevice* dev = te::rhi::CreateDevice(backend);
  if (!dev) return;
  assert(dev->GetBackend() == backend);

  te::rhi::IFence* fence = dev->CreateFence(false);
  assert(fence != nullptr);
  te::rhi::Signal(fence);
  te::rhi::Wait(fence);
  dev->DestroyFence(fence);

  te::rhi::IFence* fenceSignaled = dev->CreateFence(true);
  if (fenceSignaled) {
    fenceSignaled->Wait();
    fenceSignaled->Reset();
    dev->DestroyFence(fenceSignaled);
  }

  te::rhi::ISemaphore* sem = dev->CreateSemaphore();
  assert(sem != nullptr);
  dev->DestroySemaphore(sem);

  te::rhi::DestroyDevice(dev);
}

void test_submit_then_signal_fence_then_wait(te::rhi::Backend backend) {
  te::rhi::IDevice* dev = te::rhi::CreateDevice(backend);
  if (!dev) return;

  te::rhi::IQueue* q = dev->GetQueue(te::rhi::QueueType::Graphics, 0);
  assert(q != nullptr);
  te::rhi::ICommandList* cmd = dev->CreateCommandList();
  assert(cmd != nullptr);
  te::rhi::IFence* fence = dev->CreateFence(false);
  assert(fence != nullptr);

  te::rhi::Begin(cmd);
  te::rhi::End(cmd);
  q->Submit(cmd, fence, nullptr, nullptr);
  fence->Wait();

  dev->DestroyFence(fence);
  dev->DestroyCommandList(cmd);
  te::rhi::DestroyDevice(dev);
}

}  // namespace

int main() {
#if defined(TE_RHI_VULKAN) && TE_RHI_VULKAN
  test_backend(te::rhi::Backend::Vulkan);
  test_submit_then_signal_fence_then_wait(te::rhi::Backend::Vulkan);
#endif

#if defined(TE_RHI_D3D12) && TE_RHI_D3D12 && defined(_WIN32)
  test_backend(te::rhi::Backend::D3D12);
  test_submit_then_signal_fence_then_wait(te::rhi::Backend::D3D12);
#endif

#if defined(TE_RHI_METAL) && TE_RHI_METAL && defined(__APPLE__)
  test_backend(te::rhi::Backend::Metal);
  test_submit_then_signal_fence_then_wait(te::rhi::Backend::Metal);
#endif

#if defined(TE_RHI_D3D11) && TE_RHI_D3D11 && defined(_WIN32)
  test_backend(te::rhi::Backend::D3D11);
  test_submit_then_signal_fence_then_wait(te::rhi::Backend::D3D11);
#endif

  return 0;
}
