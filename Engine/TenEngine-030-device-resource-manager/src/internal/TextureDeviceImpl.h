/** @file TextureDeviceImpl.h
 *  030-DeviceResourceManager internal: Texture device resource creation implementation.
 */
#pragma once

#include <te/rhi/device.hpp>
#include <te/rhi/resources.hpp>
#include <te/resource/Resource.h>
#include <te/deviceresource/CommandListPool.h>
#include <te/deviceresource/StagingBufferManager.h>
#include "AsyncOperationContext.h"
#include <cstddef>

namespace te {
namespace deviceresource {
namespace internal {

// Forward declaration
struct DeviceResources;

/**
 * Create GPU texture synchronously.
 */
rhi::ITexture* CreateDeviceTextureSync(
    void const* pixelData,
    size_t pixelDataSize,
    rhi::TextureDesc const& textureDesc,
    rhi::IDevice* device,
    DeviceResources& deviceResources);

/**
 * Async context for texture creation.
 */
struct AsyncTextureCreateContext : public AsyncOperationContext {
  void (*callback)(rhi::ITexture* texture, bool success, void* user_data);
  void* user_data;
  
  // Operation data
  void const* pixelData;
  size_t pixelDataSize;
  rhi::TextureDesc rhiDesc;
  rhi::ITexture* texture;

  AsyncTextureCreateContext(
      void const* pData,
      size_t pDataSize,
      rhi::TextureDesc const& desc,
      rhi::IDevice* dev,
      void (*cb)(rhi::ITexture*, bool, void*),
      void* ud,
      CommandListPool* pool,
      StagingBufferManager* stagingMgr)
      : AsyncOperationContext(dev, pool, stagingMgr),
        callback(cb),
        user_data(ud),
        pixelData(pData),
        pixelDataSize(pDataSize),
        rhiDesc(desc),
        texture(nullptr) {}
};

/**
 * Worker function for async texture creation.
 */
void AsyncTextureCreateWorker(void* ctx);

/**
 * Create GPU texture asynchronously.
 * Returns operation handle for status tracking.
 */
ResourceOperationHandle CreateDeviceTextureAsync(
    void const* pixelData,
    size_t pixelDataSize,
    rhi::TextureDesc const& textureDesc,
    rhi::IDevice* device,
    DeviceResources& deviceResources,
    void (*callback)(rhi::ITexture* texture, bool success, void* user_data),
    void* user_data);

/**
 * Update GPU texture data.
 */
bool UpdateDeviceTexture(
    rhi::ITexture* texture,
    rhi::IDevice* device,
    DeviceResources& deviceResources,
    void const* data,
    size_t size,
    rhi::TextureDesc const& textureDesc);

}  // namespace internal
}  // namespace deviceresource
}  // namespace te
