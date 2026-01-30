/**
 * @file command_list_submit.cpp
 * @brief Test command list Begin/End, Draw/Dispatch/Copy, ResourceBarrier, Submit.
 */
#include "te/rhi/device.hpp"
#include "te/rhi/command_list.hpp"
#include "te/rhi/types.hpp"
#include <cassert>

int main() {
  auto* dev = te::rhi::CreateDevice(te::rhi::Backend::Vulkan);
  assert(dev);
  auto* q = dev->GetQueue(te::rhi::QueueType::Graphics, 0);
  assert(q);
  auto* cmd = dev->CreateCommandList();
  assert(cmd);

  te::rhi::Begin(cmd);
  cmd->Draw(3, 1);
  cmd->Dispatch(1, 1, 1);
  cmd->ResourceBarrier();
  te::rhi::End(cmd);
  te::rhi::Submit(cmd, q);

  dev->DestroyCommandList(cmd);
  te::rhi::DestroyDevice(dev);
  return 0;
}
