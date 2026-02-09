/** @file ResourceUploadHelper.cpp
 *  030-DeviceResourceManager internal: Helper functions implementation.
 */
#include "ResourceUploadHelper.h"
#include <te/rhi/device.hpp>
#include <te/rhi/command_list.hpp>
#include <te/rhi/sync.hpp>
#include <te/core/log.h>

namespace te {
namespace deviceresource {
namespace internal {

void SetupTextureBarrier(
    rhi::ICommandList* cmd,
    rhi::ITexture* texture,
    rhi::ResourceState srcState,
    rhi::ResourceState dstState,
    uint32_t mipLevel,
    uint32_t arrayLayer) {
  if (!cmd || !texture) {
    return;
  }

  rhi::TextureBarrier barrier{};
  barrier.texture = texture;
  barrier.mipLevel = mipLevel;
  barrier.arrayLayer = arrayLayer;
  barrier.srcState = srcState;
  barrier.dstState = dstState;
  cmd->ResourceBarrier(0, nullptr, 1, &barrier);
}

bool SubmitCommandListSync(
    rhi::ICommandList* cmd,
    rhi::IDevice* device,
    rhi::QueueType queueType) {
  if (!cmd || !device) {
    te::core::Log(te::core::LogLevel::Error, "ResourceUploadHelper::SubmitCommandListSync: Invalid parameters");
    return false;
  }

  rhi::IQueue* queue = device->GetQueue(queueType, 0);
  if (!queue) {
    te::core::Log(te::core::LogLevel::Error, "ResourceUploadHelper::SubmitCommandListSync: Failed to get queue");
    return false;
  }

  queue->Submit(cmd);
  queue->WaitIdle();
  return true;
}

bool SubmitCommandListAsync(
    rhi::ICommandList* cmd,
    rhi::IDevice* device,
    rhi::IFence* fence,
    rhi::QueueType queueType) {
  if (!cmd || !device || !fence) {
    te::core::Log(te::core::LogLevel::Error, "ResourceUploadHelper::SubmitCommandListAsync: Invalid parameters");
    return false;
  }

  rhi::IQueue* queue = device->GetQueue(queueType, 0);
  if (!queue) {
    te::core::Log(te::core::LogLevel::Error, "ResourceUploadHelper::SubmitCommandListAsync: Failed to get queue");
    return false;
  }

  rhi::Submit(cmd, queue, fence, nullptr, nullptr);
  return true;
}

rhi::IBuffer* AllocateAndCopyStagingBuffer(
    rhi::IDevice* device,
    StagingBufferManager* stagingBufferManager,
    void const* data,
    size_t size) {
  if (!device || !stagingBufferManager || !data || size == 0) {
    return nullptr;
  }

  rhi::IBuffer* stagingBuffer = stagingBufferManager->Allocate(size);
  if (!stagingBuffer) {
    return nullptr;
  }

  device->UpdateBuffer(stagingBuffer, 0, data, size);
  return stagingBuffer;
}

void CopyBufferToTexture(
    rhi::ICommandList* cmd,
    rhi::IBuffer* stagingBuffer,
    size_t bufferOffset,
    rhi::ITexture* texture,
    rhi::TextureDesc const& textureDesc,
    uint32_t mipLevel,
    uint32_t arrayLayer) {
  if (!cmd || !stagingBuffer || !texture) {
    return;
  }

  rhi::TextureRegion dstRegion{};
  dstRegion.texture = texture;
  dstRegion.mipLevel = mipLevel;
  dstRegion.arrayLayer = arrayLayer;
  dstRegion.x = 0;
  dstRegion.y = 0;
  dstRegion.z = 0;
  dstRegion.width = textureDesc.width;
  dstRegion.height = textureDesc.height;
  dstRegion.depth = textureDesc.depth;
  cmd->CopyBufferToTexture(stagingBuffer, bufferOffset, texture, dstRegion);
}

}  // namespace internal
}  // namespace deviceresource
}  // namespace te
