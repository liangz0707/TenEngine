/** @file AsyncOperationContext.h
 *  030-DeviceResourceManager internal: Base class for async operation contexts.
 */
#pragma once

#include <te/rhi/device.hpp>
#include <te/rhi/command_list.hpp>
#include <te/rhi/resources.hpp>
#include <te/rhi/sync.hpp>
#include <te/deviceresource/CommandListPool.h>
#include <te/deviceresource/StagingBufferManager.h>

namespace te {
namespace deviceresource {
namespace internal {

/**
 * Base class for async operation contexts.
 * Manages common resources like command lists, staging buffers, and fences.
 */
struct AsyncOperationContext {
  rhi::IDevice* device;
  CommandListPool* commandListPool;
  StagingBufferManager* stagingBufferManager;
  
  rhi::ICommandList* cmd;
  rhi::IBuffer* stagingBuffer;
  rhi::IFence* fence;

  AsyncOperationContext(
      rhi::IDevice* dev,
      CommandListPool* pool,
      StagingBufferManager* stagingMgr)
      : device(dev),
        commandListPool(pool),
        stagingBufferManager(stagingMgr),
        cmd(nullptr),
        stagingBuffer(nullptr),
        fence(nullptr) {}

  virtual ~AsyncOperationContext() = default;

  /**
   * Cleanup resources. Should be called when operation completes or fails.
   */
  void Cleanup() {
    if (commandListPool && cmd) {
      commandListPool->Release(cmd);
      cmd = nullptr;
    }
    if (stagingBufferManager && stagingBuffer) {
      stagingBufferManager->Release(stagingBuffer);
      stagingBuffer = nullptr;
    }
    if (device && fence) {
      device->DestroyFence(fence);
      fence = nullptr;
    }
  }
};

}  // namespace internal
}  // namespace deviceresource
}  // namespace te
