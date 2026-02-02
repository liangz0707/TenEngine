/** @file buffer_update_uniform_bind.cpp
 *  US7 test: CreateBuffer(BufferUsage::Uniform), UpdateBuffer, SetUniformBuffer, Submit.
 *  Verifies no crash and that upstream/backend APIs are invoked.
 */
#include <te/rhi/device.hpp>
#include <te/rhi/command_list.hpp>
#include <te/rhi/queue.hpp>
#include <te/rhi/resources.hpp>
#include <te/rhi/types.hpp>
#include <cassert>
#include <cstdio>
#include <cstring>

int main() {
  using namespace te::rhi;
  IDevice* dev = CreateDevice();
  if (!dev) {
    std::printf("CreateDevice() nullptr; skip buffer_update_uniform_bind\n");
    return 0;
  }
  BufferDesc bd = {};
  bd.size = 256;
  bd.usage = static_cast<uint32_t>(BufferUsage::Uniform);
  IBuffer* buf = dev->CreateBuffer(bd);
  if (!buf) {
    std::printf("CreateBuffer(Uniform) nullptr; skip buffer_update_uniform_bind\n");
    DestroyDevice(dev);
    return 0;
  }
  unsigned char data[256];
  std::memset(data, 0xAB, sizeof(data));
  dev->UpdateBuffer(buf, 0, data, sizeof(data));
  ICommandList* cmd = dev->CreateCommandList();
  if (!cmd) {
    dev->DestroyBuffer(buf);
    DestroyDevice(dev);
    std::printf("CreateCommandList nullptr; skip buffer_update_uniform_bind\n");
    return 0;
  }
  IQueue* queue = dev->GetQueue(QueueType::Graphics, 0);
  if (!queue) {
    dev->DestroyCommandList(cmd);
    dev->DestroyBuffer(buf);
    DestroyDevice(dev);
    std::printf("GetQueue nullptr; skip buffer_update_uniform_bind\n");
    return 0;
  }
  cmd->Begin();
  cmd->SetUniformBuffer(0, buf, 0);
  cmd->End();
  queue->Submit(cmd, nullptr, nullptr, nullptr);
  queue->WaitIdle();
  dev->DestroyCommandList(cmd);
  dev->DestroyBuffer(buf);
  DestroyDevice(dev);
  std::printf("buffer_update_uniform_bind passed\n");
  return 0;
}
