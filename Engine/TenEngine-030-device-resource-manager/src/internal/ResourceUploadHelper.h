/** @file ResourceUploadHelper.h
 *  030-DeviceResourceManager internal: Helper functions for resource upload operations.
 */
#pragma once

#include <te/rhi/device.hpp>
#include <te/rhi/resources.hpp>
#include <te/rhi/command_list.hpp>
#include <te/rhi/types.hpp>
#include <te/deviceresource/StagingBufferManager.h>
#include <cstddef>

namespace te {
namespace deviceresource {
namespace internal {

/**
 * Setup texture barrier for state transition.
 */
void SetupTextureBarrier(
    rhi::ICommandList* cmd,
    rhi::ITexture* texture,
    rhi::ResourceState srcState,
    rhi::ResourceState dstState,
    uint32_t mipLevel = 0,
    uint32_t arrayLayer = 0);

/**
 * Submit command list synchronously and wait for completion.
 */
bool SubmitCommandListSync(
    rhi::ICommandList* cmd,
    rhi::IDevice* device,
    rhi::QueueType queueType = rhi::QueueType::Graphics);

/**
 * Submit command list asynchronously with fence.
 */
bool SubmitCommandListAsync(
    rhi::ICommandList* cmd,
    rhi::IDevice* device,
    rhi::IFence* fence,
    rhi::QueueType queueType = rhi::QueueType::Graphics);

/**
 * Allocate staging buffer and copy data to it.
 */
rhi::IBuffer* AllocateAndCopyStagingBuffer(
    rhi::IDevice* device,
    StagingBufferManager* stagingBufferManager,
    void const* data,
    size_t size);

/**
 * Copy buffer to texture region.
 */
void CopyBufferToTexture(
    rhi::ICommandList* cmd,
    rhi::IBuffer* stagingBuffer,
    size_t bufferOffset,
    rhi::ITexture* texture,
    rhi::TextureDesc const& textureDesc,
    uint32_t mipLevel = 0,
    uint32_t arrayLayer = 0);

}  // namespace internal
}  // namespace deviceresource
}  // namespace te
