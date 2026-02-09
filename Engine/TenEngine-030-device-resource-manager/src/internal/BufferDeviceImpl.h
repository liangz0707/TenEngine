/** @file BufferDeviceImpl.h
 *  030-DeviceResourceManager internal: Buffer device resource creation implementation.
 */
#pragma once

#include <te/rhi/device.hpp>
#include <te/rhi/resources.hpp>
#include <te/deviceresource/CommandListPool.h>
#include <te/deviceresource/StagingBufferManager.h>
#include <te/deviceresource/ResourceOperationTypes.h>
#include "AsyncOperationContext.h"
#include <cstddef>

namespace te {
namespace deviceresource {
namespace internal {

// Forward declaration
struct DeviceResources;

/**
 * Create GPU buffer synchronously.
 */
rhi::IBuffer* CreateDeviceBufferSync(
    void const* data,
    size_t dataSize,
    rhi::BufferDesc const& bufferDesc,
    rhi::IDevice* device,
    DeviceResources& deviceResources);

/**
 * Async context for buffer creation.
 */
struct AsyncBufferCreateContext : public AsyncOperationContext {
  void (*callback)(rhi::IBuffer* buffer, bool success, void* user_data);
  void* user_data;
  
  // Operation data
  void const* data;
  size_t dataSize;
  rhi::BufferDesc bufferDesc;
  rhi::IBuffer* buffer;

  AsyncBufferCreateContext(
      void const* d,
      size_t dSize,
      rhi::BufferDesc const& desc,
      rhi::IDevice* dev,
      void (*cb)(rhi::IBuffer*, bool, void*),
      void* ud,
      CommandListPool* pool,
      StagingBufferManager* stagingMgr)
      : AsyncOperationContext(dev, pool, stagingMgr),
        callback(cb),
        user_data(ud),
        data(d),
        dataSize(dSize),
        bufferDesc(desc),
        buffer(nullptr) {}
};

/**
 * Worker function for async buffer creation.
 */
void AsyncBufferCreateWorker(void* ctx);

/**
 * Create GPU buffer asynchronously.
 * Returns operation handle for status tracking.
 */
ResourceOperationHandle CreateDeviceBufferAsync(
    void const* data,
    size_t dataSize,
    rhi::BufferDesc const& bufferDesc,
    rhi::IDevice* device,
    DeviceResources& deviceResources,
    void (*callback)(rhi::IBuffer* buffer, bool success, void* user_data),
    void* user_data);

}  // namespace internal
}  // namespace deviceresource
}  // namespace te
