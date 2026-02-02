/** @file command_list_submit.cpp
 *  US2 test: CreateCommandList, Begin, End, Submit (minimal; no draw if CreateCommandList returns nullptr).
 */
#include <te/rhi/device.hpp>
#include <te/rhi/command_list.hpp>
#include <te/rhi/types.hpp>
#include <cassert>
#include <cstdio>

int main() {
  using namespace te::rhi;
  IDevice* dev = CreateDevice();
  if (!dev) {
    std::printf("CreateDevice() returned nullptr; skip command_list_submit\n");
    return 0;
  }
  ICommandList* cmd = dev->CreateCommandList();
  if (!cmd) {
    std::printf("CreateCommandList returned nullptr; skip submit\n");
    DestroyDevice(dev);
    return 0;
  }
  IQueue* queue = dev->GetQueue(QueueType::Graphics, 0);
  if (!queue) {
    dev->DestroyCommandList(cmd);
    DestroyDevice(dev);
    std::printf("GetQueue returned nullptr; skip submit\n");
    return 0;
  }
  Begin(cmd);
  cmd->End();
  End(cmd);
  Submit(cmd, queue);
  queue->WaitIdle();
  dev->DestroyCommandList(cmd);
  DestroyDevice(dev);
  std::printf("command_list_submit passed\n");
  return 0;
}
