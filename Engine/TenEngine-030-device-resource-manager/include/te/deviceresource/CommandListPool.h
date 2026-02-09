/** @file CommandListPool.h
 *  030-DeviceResourceManager internal: Command list pool for async GPU resource creation.
 */
#pragma once

#include <te/rhi/device.hpp>
#include <te/rhi/command_list.hpp>
#include <mutex>
#include <queue>
#include <vector>

namespace te {
namespace deviceresource {

/**
 * Command list pool for efficient async GPU resource creation.
 * Thread-safe, manages command lists per device.
 */
class CommandListPool {
 public:
  explicit CommandListPool(rhi::IDevice* device);
  ~CommandListPool();

  /**
   * Acquire a command list from the pool.
   * Creates a new one if pool is empty.
   */
  rhi::ICommandList* Acquire();

  /**
   * Return a command list to the pool for reuse.
   */
  void Release(rhi::ICommandList* cmd);

  /**
   * Clear all command lists in the pool.
   */
  void Clear();

 private:
  rhi::IDevice* device_;
  std::mutex mutex_;
  std::queue<rhi::ICommandList*> available_;
  std::vector<rhi::ICommandList*> all_;
  size_t maxSize_;
};

}  // namespace deviceresource
}  // namespace te
