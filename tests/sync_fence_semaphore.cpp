/** @file sync_fence_semaphore.cpp
 *  US5 test: CreateFence, Signal, Wait, Submit with fence (if CreateFence returns non-null).
 */
#include <te/rhi/device.hpp>
#include <te/rhi/command_list.hpp>
#include <te/rhi/sync.hpp>
#include <te/rhi/types.hpp>
#include <cstdio>

int main() {
  using namespace te::rhi;
  IDevice* dev = CreateDevice();
  if (!dev) {
    std::printf("CreateDevice() nullptr; skip sync_fence_semaphore\n");
    return 0;
  }
  IFence* fence = dev->CreateFence(false);
  if (fence) {
    Signal(fence);
    Wait(fence);
    dev->DestroyFence(fence);
  }
  ICommandList* cmd = dev->CreateCommandList();
  IQueue* queue = dev->GetQueue(QueueType::Graphics, 0);
  if (cmd && queue) {
    Begin(cmd);
    End(cmd);
    IFence* f2 = dev->CreateFence(false);
    Submit(cmd, queue, f2, nullptr, nullptr);
    if (f2) {
      f2->Wait();
      dev->DestroyFence(f2);
    }
    dev->DestroyCommandList(cmd);
  }
  DestroyDevice(dev);
  std::printf("sync_fence_semaphore passed\n");
  return 0;
}
