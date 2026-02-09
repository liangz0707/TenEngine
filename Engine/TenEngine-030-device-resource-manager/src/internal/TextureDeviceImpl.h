/** @file TextureDeviceImpl.h
 *  030-DeviceResourceManager internal: Texture device resource creation implementation.
 */
#pragma once

#include <te/rhi/device.hpp>
#include <te/rhi/resources.hpp>
#include <te/resource/Resource.h>
#include <te/deviceresource/CommandListPool.h>
#include <te/deviceresource/StagingBufferManager.h>
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
struct AsyncTextureCreateContext {
  rhi::IDevice* device;
  void (*callback)(rhi::ITexture* texture, bool success, void* user_data);
  void* user_data;
  
  // Operation data
  void const* pixelData;
  size_t pixelDataSize;
  rhi::TextureDesc rhiDesc;
  rhi::ITexture* texture;
  
  // Resources (managed by DeviceResources)
  CommandListPool* commandListPool;
  StagingBufferManager* stagingBufferManager;
  
  rhi::ICommandList* cmd;
  rhi::IBuffer* stagingBuffer;
  rhi::IFence* fence;

  AsyncTextureCreateContext(
      void const* pData,
      size_t pDataSize,
      rhi::TextureDesc const& desc,
      rhi::IDevice* dev,
      void (*cb)(rhi::ITexture*, bool, void*),
      void* ud,
      CommandListPool* pool,
      StagingBufferManager* stagingMgr)
      : device(dev),
        callback(cb),
        user_data(ud),
        pixelData(pData),
        pixelDataSize(pDataSize),
        rhiDesc(desc),
        texture(nullptr),
        commandListPool(pool),
        stagingBufferManager(stagingMgr),
        cmd(nullptr),
        stagingBuffer(nullptr),
        fence(nullptr) {}
};

/**
 * Worker function for async texture creation.
 */
void AsyncTextureCreateWorker(void* ctx);

/**
 * Create GPU texture asynchronously.
 */
bool CreateDeviceTextureAsync(
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
    size_t size);

}  // namespace internal
}  // namespace deviceresource
}  // namespace te
