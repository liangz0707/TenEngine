/** @file TextureDeviceImpl.cpp
 *  030-DeviceResourceManager internal: Texture device resource creation implementation.
 */
#include "TextureDeviceImpl.h"
#include "ResourceUploadHelper.h"
#include "AsyncOperationContext.h"
#include "DeviceResources.h"
#include <te/deviceresource/ResourceOperationTypes.h>
#include <te/rhi/device.hpp>
#include <te/rhi/resources.hpp>
#include <te/rhi/command_list.hpp>
#include <te/rhi/sync.hpp>
#include <te/core/log.h>
#include <te/core/thread.h>

// Forward declaration for internal registration functions
namespace te {
namespace deviceresource {
namespace internal {
ResourceOperationHandle RegisterOperation(AsyncOperationContext* context);
void UnregisterOperation(ResourceOperationHandle handle);
}  // namespace internal
}  // namespace deviceresource
}  // namespace te

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

  // Check if cancelled
  if (context->IsCancelled()) {
    std::lock_guard<std::mutex> lock(context->statusMutex);
    context->status.store(ResourceOperationStatus::Cancelled);
    context->callback(nullptr, false, context->user_data);
    delete context;
    return;
  }

  // Update status: Uploading
  {
    std::lock_guard<std::mutex> lock(context->statusMutex);
    context->status.store(ResourceOperationStatus::Uploading);
  }
  context->progress.store(0.1f);  // 10% - starting upload

  // Allocate staging buffer
  context->stagingBuffer = AllocateAndCopyStagingBuffer(
      context->device,
      context->stagingBufferManager,
      context->pixelData,
      context->pixelDataSize);
  if (!context->stagingBuffer) {
    te::core::Log(te::core::LogLevel::Error, "TextureDeviceImpl::AsyncTextureCreateWorker: Failed to allocate staging buffer");
    std::lock_guard<std::mutex> lock(context->statusMutex);
    context->status.store(ResourceOperationStatus::Failed);
    context->callback(nullptr, false, context->user_data);
    delete context;
    return;
  }

  context->progress.store(0.3f);  // 30% - staging buffer allocated

  // Check if cancelled
  if (context->IsCancelled()) {
    std::lock_guard<std::mutex> lock(context->statusMutex);
    context->status.store(ResourceOperationStatus::Cancelled);
    context->stagingBufferManager->Release(context->stagingBuffer);
    context->callback(nullptr, false, context->user_data);
    delete context;
    return;
  }

  // Acquire command list from pool
  context->cmd = context->commandListPool->Acquire();
  if (!context->cmd) {
    te::core::Log(te::core::LogLevel::Error, "TextureDeviceImpl::AsyncTextureCreateWorker: Failed to acquire command list");
    std::lock_guard<std::mutex> lock(context->statusMutex);
    context->status.store(ResourceOperationStatus::Failed);
    context->stagingBufferManager->Release(context->stagingBuffer);
    context->callback(nullptr, false, context->user_data);
    delete context;
    return;
  }

  context->progress.store(0.5f);  // 50% - command list acquired

  // Check if cancelled
  if (context->IsCancelled()) {
    std::lock_guard<std::mutex> lock(context->statusMutex);
    context->status.store(ResourceOperationStatus::Cancelled);
    context->commandListPool->Release(context->cmd);
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

  context->progress.store(0.7f);  // 70% - commands recorded

  // Create fence for synchronization
  context->fence = context->device->CreateFence(false);
  if (!context->fence) {
    te::core::Log(te::core::LogLevel::Error, "TextureDeviceImpl::AsyncTextureCreateWorker: Failed to create fence");
    std::lock_guard<std::mutex> lock(context->statusMutex);
    context->status.store(ResourceOperationStatus::Failed);
    context->commandListPool->Release(context->cmd);
    context->stagingBufferManager->Release(context->stagingBuffer);
    context->callback(nullptr, false, context->user_data);
    delete context;
    return;
  }

  // Submit command list with fence
  if (!SubmitCommandListAsync(context->cmd, context->device, context->fence)) {
    std::lock_guard<std::mutex> lock(context->statusMutex);
    context->status.store(ResourceOperationStatus::Failed);
    context->device->DestroyFence(context->fence);
    context->commandListPool->Release(context->cmd);
    context->stagingBufferManager->Release(context->stagingBuffer);
    context->callback(nullptr, false, context->user_data);
    delete context;
    return;
  }

  // Update status: Submitted
  {
    std::lock_guard<std::mutex> lock(context->statusMutex);
    context->status.store(ResourceOperationStatus::Submitted);
  }
  context->progress.store(0.8f);  // 80% - submitted to GPU

  te::core::IThreadPool* pool = te::core::GetThreadPool();
  te::core::ITaskExecutor* workerEx = pool ? pool->GetWorkerExecutor() : nullptr;
  if (workerEx) {
    workerEx->SubmitTask([](void* ctx) {
      auto* ctxt = static_cast<AsyncTextureCreateContext*>(ctx);
      if (!ctxt || !ctxt->fence) {
        return;
      }

      // Wait for GPU to complete
      ctxt->fence->Wait();

      // Check if cancelled during wait
      bool wasCancelled = ctxt->IsCancelled();
      
      // Update status
      {
        std::lock_guard<std::mutex> lock(ctxt->statusMutex);
        if (!wasCancelled) {
          ctxt->status.store(ResourceOperationStatus::Completed);
        }
      }
      ctxt->progress.store(1.0f);  // 100% - completed

      // Cleanup resources
      ctxt->Cleanup();

      // Call user callback
      bool success = (ctxt->texture != nullptr && !wasCancelled);
      ctxt->callback(ctxt->texture, success, ctxt->user_data);

      // Unregister operation before deleting context
      ResourceOperationHandle handle = reinterpret_cast<ResourceOperationHandle>(ctxt);
      internal::UnregisterOperation(handle);

      // Cleanup context
      delete ctxt;
    }, context);
  } else {
    // Fallback: synchronous wait
    context->fence->Wait();
    
    bool wasCancelled = context->IsCancelled();
    {
      std::lock_guard<std::mutex> lock(context->statusMutex);
      if (!wasCancelled) {
        context->status.store(ResourceOperationStatus::Completed);
      }
    }
    context->progress.store(1.0f);
    
    context->Cleanup();
    bool success = (context->texture != nullptr && !wasCancelled);
    context->callback(context->texture, success, context->user_data);
    
    // Unregister operation before deleting context
    ResourceOperationHandle handle = reinterpret_cast<ResourceOperationHandle>(context);
    internal::UnregisterOperation(handle);
    
    delete context;
  }
}

ResourceOperationHandle CreateDeviceTextureAsync(
    void const* pixelData,
    size_t pixelDataSize,
    rhi::TextureDesc const& textureDesc,
    rhi::IDevice* device,
    DeviceResources& deviceResources,
    void (*callback)(rhi::ITexture* texture, bool success, void* user_data),
    void* user_data) {
  if (!pixelData || pixelDataSize == 0 || !device || !callback) {
    te::core::Log(te::core::LogLevel::Error, "TextureDeviceImpl::CreateDeviceTextureAsync: Invalid parameters");
    return nullptr;
  }

  if (textureDesc.width == 0 || textureDesc.height == 0) {
    te::core::Log(te::core::LogLevel::Error, "TextureDeviceImpl::CreateDeviceTextureAsync: Invalid texture dimensions");
    callback(nullptr, false, user_data);
    return nullptr;
  }

  // Create GPU texture (synchronous, but fast)
  rhi::ITexture* texture = device->CreateTexture(textureDesc);
  if (!texture) {
    te::core::Log(te::core::LogLevel::Error, "TextureDeviceImpl::CreateDeviceTextureAsync: Failed to create GPU texture");
    callback(nullptr, false, user_data);
    return nullptr;
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

  // Register operation and get handle
  ResourceOperationHandle handle = internal::RegisterOperation(context);

  te::core::IThreadPool* pool = te::core::GetThreadPool();
  te::core::ITaskExecutor* workerEx = pool ? pool->GetWorkerExecutor() : nullptr;
  if (workerEx) {
    workerEx->SubmitTask(AsyncTextureCreateWorker, context);
    return handle;
  }
  te::core::Log(te::core::LogLevel::Error, "TextureDeviceImpl::CreateDeviceTextureAsync: Thread pool not available, falling back to sync");
  AsyncTextureCreateWorker(context);
  return handle;
}

bool UpdateDeviceTexture(
    rhi::ITexture* texture,
    rhi::IDevice* device,
    DeviceResources& deviceResources,
    void const* data,
    size_t size,
    rhi::TextureDesc const& textureDesc) {
  if (!texture || !device || !data || size == 0) {
    te::core::Log(te::core::LogLevel::Error, "TextureDeviceImpl::UpdateDeviceTexture: Invalid parameters");
    return false;
  }

  if (textureDesc.width == 0 || textureDesc.height == 0) {
    te::core::Log(te::core::LogLevel::Error, "TextureDeviceImpl::UpdateDeviceTexture: Invalid texture dimensions");
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

  // Copy buffer to texture using provided texture description
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
