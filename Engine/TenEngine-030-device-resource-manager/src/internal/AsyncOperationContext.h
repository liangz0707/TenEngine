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
#include <te/deviceresource/ResourceOperationTypes.h>
#include <atomic>
#include <mutex>

namespace te {
namespace deviceresource {
namespace internal {

/**
 * Base class for async operation contexts.
 * Manages common resources like command lists, staging buffers, and fences.
 * Tracks operation status and progress.
 */
struct AsyncOperationContext {
  rhi::IDevice* device;
  CommandListPool* commandListPool;
  StagingBufferManager* stagingBufferManager;
  
  rhi::ICommandList* cmd;
  rhi::IBuffer* stagingBuffer;
  rhi::IFence* fence;

  // Operation tracking
  std::atomic<ResourceOperationStatus> status{ResourceOperationStatus::Pending};
  std::atomic<float> progress{0.0f};
  std::atomic<bool> cancelled{false};
  std::mutex statusMutex;  // Protect status changes

  AsyncOperationContext(
      rhi::IDevice* dev,
      CommandListPool* pool,
      StagingBufferManager* stagingMgr)
      : device(dev),
        commandListPool(pool),
        stagingBufferManager(stagingMgr),
        cmd(nullptr),
        stagingBuffer(nullptr),
        fence(nullptr),
        status(ResourceOperationStatus::Pending),
        progress(0.0f),
        cancelled(false) {}

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

  /**
   * Check if operation is cancelled.
   */
  bool IsCancelled() const {
    return cancelled.load();
  }

  /**
   * Cancel the operation.
   */
  void Cancel() {
    cancelled.store(true);
    std::lock_guard<std::mutex> lock(statusMutex);
    ResourceOperationStatus current = status.load();
    if (current == ResourceOperationStatus::Pending ||
        current == ResourceOperationStatus::Uploading ||
        current == ResourceOperationStatus::Submitted) {
      status.store(ResourceOperationStatus::Cancelled);
    }
  }
};

}  // namespace internal
}  // namespace deviceresource
}  // namespace te
