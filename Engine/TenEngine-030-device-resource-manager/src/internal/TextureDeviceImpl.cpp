/** @file TextureDeviceImpl.cpp
 *  030-DeviceResourceManager internal: Texture device resource creation implementation.
 */
#include "TextureDeviceImpl.h"
#include "ResourceUploadHelper.h"
#include "AsyncOperationContext.h"
#include "DeviceResources.h"
#include <te/rhi/device.hpp>
#include <te/rhi/resources.hpp>
#include <te/rhi/command_list.hpp>
#include <te/rhi/sync.hpp>
#include <te/core/log.h>
#include <te/core/thread.h>

namespace te {
namespace deviceresource {
namespace internal {

rhi::ITexture* CreateDeviceTextureSync(
    void const* pixelData,
    size_t pixelDataSize,
    rhi::TextureDesc const& textureDesc,
    rhi::IDevice* device,
    DeviceResources& deviceResources) {
  if (!pixelData || pixelDataSize == 0 || !device) {
    te::core::Log(te::core::LogLevel::Error, "TextureDeviceImpl::CreateDeviceTextureSync: Invalid parameters");
    return nullptr;
  }

  if (textureDesc.width == 0 || textureDesc.height == 0) {
    te::core::Log(te::core::LogLevel::Error, "TextureDeviceImpl::CreateDeviceTextureSync: Invalid texture dimensions");
    return nullptr;
  }

  // Create GPU texture
  rhi::ITexture* texture = device->CreateTexture(textureDesc);
  if (!texture) {
    te::core::Log(te::core::LogLevel::Error, "TextureDeviceImpl::CreateDeviceTextureSync: Failed to create GPU texture");
    return nullptr;
  }

  // Allocate staging buffer and copy data
  rhi::IBuffer* stagingBuffer = AllocateAndCopyStagingBuffer(
      device,
      deviceResources.stagingBufferManager.get(),
      pixelData,
      pixelDataSize);
  if (!stagingBuffer) {
    te::core::Log(te::core::LogLevel::Error, "TextureDeviceImpl::CreateDeviceTextureSync: Failed to allocate staging buffer");
    device->DestroyTexture(texture);
    return nullptr;
  }

  // Create command list for upload
  rhi::ICommandList* cmd = device->CreateCommandList();
  if (!cmd) {
    te::core::Log(te::core::LogLevel::Error, "TextureDeviceImpl::CreateDeviceTextureSync: Failed to create command list");
    deviceResources.stagingBufferManager->Release(stagingBuffer);
    device->DestroyTexture(texture);
    return nullptr;
  }

  // Begin command list
  cmd->Begin();

  // Resource barrier: Common -> CopyDst
  SetupTextureBarrier(cmd, texture, rhi::ResourceState::Common, rhi::ResourceState::CopyDst);

  // Copy buffer to texture
  CopyBufferToTexture(cmd, stagingBuffer, 0, texture, textureDesc);

  // Resource barrier: CopyDst -> ShaderResource
  SetupTextureBarrier(cmd, texture, rhi::ResourceState::CopyDst, rhi::ResourceState::ShaderResource);

  // End command list
  cmd->End();

  // Submit command list synchronously
  if (!SubmitCommandListSync(cmd, device)) {
    device->DestroyCommandList(cmd);
    deviceResources.stagingBufferManager->Release(stagingBuffer);
    device->DestroyTexture(texture);
    return nullptr;
  }

  // Cleanup
  device->DestroyCommandList(cmd);
  deviceResources.stagingBufferManager->Release(stagingBuffer);

  return texture;
}

void AsyncTextureCreateWorker(void* ctx) {
  auto* context = static_cast<AsyncTextureCreateContext*>(ctx);
  if (!context) {
    return;
  }

  // Allocate staging buffer
  context->stagingBuffer = AllocateAndCopyStagingBuffer(
      context->device,
      context->stagingBufferManager,
      context->pixelData,
      context->pixelDataSize);
  if (!context->stagingBuffer) {
    te::core::Log(te::core::LogLevel::Error, "TextureDeviceImpl::AsyncTextureCreateWorker: Failed to allocate staging buffer");
    context->callback(nullptr, false, context->user_data);
    delete context;
    return;
  }

  // Acquire command list from pool
  context->cmd = context->commandListPool->Acquire();
  if (!context->cmd) {
    te::core::Log(te::core::LogLevel::Error, "TextureDeviceImpl::AsyncTextureCreateWorker: Failed to acquire command list");
    context->stagingBufferManager->Release(context->stagingBuffer);
    context->callback(nullptr, false, context->user_data);
    delete context;
    return;
  }

  // Begin command list
  context->cmd->Begin();

  // Resource barrier: Common -> CopyDst
  SetupTextureBarrier(context->cmd, context->texture, rhi::ResourceState::Common, rhi::ResourceState::CopyDst);

  // Copy buffer to texture
  CopyBufferToTexture(context->cmd, context->stagingBuffer, 0, context->texture, context->rhiDesc);

  // Resource barrier: CopyDst -> ShaderResource
  SetupTextureBarrier(context->cmd, context->texture, rhi::ResourceState::CopyDst, rhi::ResourceState::ShaderResource);

  // End command list
  context->cmd->End();

  // Create fence for synchronization
  context->fence = context->device->CreateFence(false);
  if (!context->fence) {
    te::core::Log(te::core::LogLevel::Error, "TextureDeviceImpl::AsyncTextureCreateWorker: Failed to create fence");
    context->commandListPool->Release(context->cmd);
    context->stagingBufferManager->Release(context->stagingBuffer);
    context->callback(nullptr, false, context->user_data);
    delete context;
    return;
  }

  // Submit command list with fence
  if (!SubmitCommandListAsync(context->cmd, context->device, context->fence)) {
    context->device->DestroyFence(context->fence);
    context->commandListPool->Release(context->cmd);
    context->stagingBufferManager->Release(context->stagingBuffer);
    context->callback(nullptr, false, context->user_data);
    delete context;
    return;
  }

  // Wait for fence in background thread (non-blocking for main thread)
  te::core::IThreadPool* threadPool = te::core::GetThreadPool();
  if (threadPool) {
    threadPool->SubmitTask([](void* ctx) {
      auto* ctxt = static_cast<AsyncTextureCreateContext*>(ctx);
      if (!ctxt || !ctxt->fence) {
        return;
      }

      // Wait for GPU to complete
      ctxt->fence->Wait();

      // Cleanup resources
      ctxt->commandListPool->Release(ctxt->cmd);
      ctxt->stagingBufferManager->Release(ctxt->stagingBuffer);
      ctxt->device->DestroyFence(ctxt->fence);

      // Call user callback
      bool success = (ctxt->texture != nullptr);
      ctxt->callback(ctxt->texture, success, ctxt->user_data);

      // Cleanup context
      delete ctxt;
    }, context);
  } else {
    // Fallback: synchronous wait
    context->fence->Wait();
    context->commandListPool->Release(context->cmd);
    context->stagingBufferManager->Release(context->stagingBuffer);
    context->device->DestroyFence(context->fence);
    context->callback(context->texture, true, context->user_data);
    delete context;
  }
}

bool CreateDeviceTextureAsync(
    void const* pixelData,
    size_t pixelDataSize,
    rhi::TextureDesc const& textureDesc,
    rhi::IDevice* device,
    DeviceResources& deviceResources,
    void (*callback)(rhi::ITexture* texture, bool success, void* user_data),
    void* user_data) {
  if (!pixelData || pixelDataSize == 0 || !device || !callback) {
    te::core::Log(te::core::LogLevel::Error, "TextureDeviceImpl::CreateDeviceTextureAsync: Invalid parameters");
    return false;
  }

  if (textureDesc.width == 0 || textureDesc.height == 0) {
    te::core::Log(te::core::LogLevel::Error, "TextureDeviceImpl::CreateDeviceTextureAsync: Invalid texture dimensions");
    callback(nullptr, false, user_data);
    return false;
  }

  // Create GPU texture (synchronous, but fast)
  rhi::ITexture* texture = device->CreateTexture(textureDesc);
  if (!texture) {
    te::core::Log(te::core::LogLevel::Error, "TextureDeviceImpl::CreateDeviceTextureAsync: Failed to create GPU texture");
    callback(nullptr, false, user_data);
    return false;
  }

  // Create async context
  auto* context = new AsyncTextureCreateContext(
      pixelData,
      pixelDataSize,
      textureDesc,
      device,
      callback,
      user_data,
      deviceResources.commandListPool.get(),
      deviceResources.stagingBufferManager.get());
  context->texture = texture;

  // Submit async work to thread pool
  te::core::IThreadPool* threadPool = te::core::GetThreadPool();
  if (threadPool) {
    threadPool->SubmitTask(AsyncTextureCreateWorker, context);
    return true;
  } else {
    // Fallback: synchronous creation
    te::core::Log(te::core::LogLevel::Error, "TextureDeviceImpl::CreateDeviceTextureAsync: Thread pool not available, falling back to sync");
    AsyncTextureCreateWorker(context);
    return true;
  }
}

bool UpdateDeviceTexture(
    rhi::ITexture* texture,
    rhi::IDevice* device,
    DeviceResources& deviceResources,
    void const* data,
    size_t size) {
  if (!texture || !device || !data || size == 0) {
    te::core::Log(te::core::LogLevel::Error, "TextureDeviceImpl::UpdateDeviceTexture: Invalid parameters");
    return false;
  }

  // Allocate staging buffer
  rhi::IBuffer* stagingBuffer = AllocateAndCopyStagingBuffer(
      device,
      deviceResources.stagingBufferManager.get(),
      data,
      size);
  if (!stagingBuffer) {
    te::core::Log(te::core::LogLevel::Error, "TextureDeviceImpl::UpdateDeviceTexture: Failed to allocate staging buffer");
    return false;
  }

  // Create command list
  rhi::ICommandList* cmd = device->CreateCommandList();
  if (!cmd) {
    te::core::Log(te::core::LogLevel::Error, "TextureDeviceImpl::UpdateDeviceTexture: Failed to create command list");
    deviceResources.stagingBufferManager->Release(stagingBuffer);
    return false;
  }

  // Begin command list
  cmd->Begin();

  // Resource barrier: ShaderResource -> CopyDst
  SetupTextureBarrier(cmd, texture, rhi::ResourceState::ShaderResource, rhi::ResourceState::CopyDst);

  // Note: For full texture update, we need texture dimensions
  // This is a simplified version - caller should provide dimensions or we should query them
  // For now, assume the size matches the texture dimensions
  rhi::TextureDesc textureDesc{};  // Would need to query actual texture desc
  CopyBufferToTexture(cmd, stagingBuffer, 0, texture, textureDesc);

  // Resource barrier: CopyDst -> ShaderResource
  SetupTextureBarrier(cmd, texture, rhi::ResourceState::CopyDst, rhi::ResourceState::ShaderResource);

  // End command list
  cmd->End();

  // Submit command list
  if (!SubmitCommandListSync(cmd, device)) {
    device->DestroyCommandList(cmd);
    deviceResources.stagingBufferManager->Release(stagingBuffer);
    return false;
  }

  // Cleanup
  device->DestroyCommandList(cmd);
  deviceResources.stagingBufferManager->Release(stagingBuffer);

  return true;
}

}  // namespace internal
}  // namespace deviceresource
}  // namespace te
