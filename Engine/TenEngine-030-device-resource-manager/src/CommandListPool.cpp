/** @file CommandListPool.cpp
 *  030-DeviceResourceManager implementation: Command list pool.
 */
#include <te/deviceresource/CommandListPool.h>
#include <te/rhi/device.hpp>
#include <te/core/log.h>

namespace te {
namespace deviceresource {

CommandListPool::CommandListPool(rhi::IDevice* device)
    : device_(device), maxSize_(16) {
  if (!device_) {
    te::core::Log(te::core::LogLevel::Error, "CommandListPool: Invalid device");
  }
}

CommandListPool::~CommandListPool() {
  Clear();
}

rhi::ICommandList* CommandListPool::Acquire() {
  if (!device_) {
    return nullptr;
  }

  std::lock_guard<std::mutex> lock(mutex_);
  
  if (!available_.empty()) {
    rhi::ICommandList* cmd = available_.front();
    available_.pop();
    return cmd;
  }

  // Create new command list if pool is empty
  rhi::ICommandList* cmd = device_->CreateCommandList();
  if (cmd) {
    all_.push_back(cmd);
  }
  return cmd;
}

void CommandListPool::Release(rhi::ICommandList* cmd) {
  if (!cmd) {
    return;
  }

  std::lock_guard<std::mutex> lock(mutex_);
  
  // Check if we should keep it in the pool
  if (available_.size() < maxSize_) {
    available_.push(cmd);
  } else {
    // Pool is full, destroy the command list
    device_->DestroyCommandList(cmd);
    // Remove from all_ list
    all_.erase(std::remove(all_.begin(), all_.end(), cmd), all_.end());
  }
}

void CommandListPool::Clear() {
  std::lock_guard<std::mutex> lock(mutex_);
  
  // Destroy all command lists
  while (!available_.empty()) {
    rhi::ICommandList* cmd = available_.front();
    available_.pop();
    if (device_) {
      device_->DestroyCommandList(cmd);
    }
  }
  
  for (rhi::ICommandList* cmd : all_) {
    if (device_) {
      device_->DestroyCommandList(cmd);
    }
  }
  
  all_.clear();
}

}  // namespace deviceresource
}  // namespace te
