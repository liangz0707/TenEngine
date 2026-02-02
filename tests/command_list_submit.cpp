/**
 * @file command_list_submit.cpp
 * @brief Test command list Begin/End, Draw/Dispatch/Copy, ResourceBarrier, Submit (US2).
 * Tests real backend API: Vulkan (TE_RHI_VULKAN), D3D12 on Windows (TE_RHI_D3D12), Metal on macOS (TE_RHI_METAL).
 * Verifies no crash; optional Fence Wait when backend supports it.
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

  te::rhi::IQueue* q = dev->GetQueue(te::rhi::QueueType::Graphics, 0);
  assert(q != nullptr);

  te::rhi::ICommandList* cmd = dev->CreateCommandList();
  assert(cmd != nullptr);

  te::rhi::Begin(cmd);
  cmd->Draw(3, 1);
  cmd->Dispatch(1, 1, 1);
  cmd->ResourceBarrier(0, nullptr, 0, nullptr);
  te::rhi::End(cmd);
  te::rhi::Submit(cmd, q);

  te::rhi::IFence* fence = dev->CreateFence();
  if (fence) {
    te::rhi::ICommandList* cmd2 = dev->CreateCommandList();
    if (cmd2) {
      te::rhi::Begin(cmd2);
      te::rhi::End(cmd2);
      q->Submit(cmd2, fence, nullptr, nullptr);
      fence->Wait();
      dev->DestroyCommandList(cmd2);
    }
    dev->DestroyFence(fence);
  }

  dev->DestroyCommandList(cmd);
  te::rhi::DestroyDevice(dev);
}

}  // namespace

int main() {
#if defined(TE_RHI_VULKAN) && TE_RHI_VULKAN
  test_backend(te::rhi::Backend::Vulkan);
#endif

#if defined(TE_RHI_D3D12) && TE_RHI_D3D12 && defined(_WIN32)
  test_backend(te::rhi::Backend::D3D12);
#endif

#if defined(TE_RHI_METAL) && TE_RHI_METAL && defined(__APPLE__)
  test_backend(te::rhi::Backend::Metal);
#endif

  return 0;
}
