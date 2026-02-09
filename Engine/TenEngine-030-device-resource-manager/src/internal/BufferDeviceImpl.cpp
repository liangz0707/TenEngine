/** @file BufferDeviceImpl.cpp
 *  030-DeviceResourceManager internal: Buffer device resource creation implementation.
 */
#include "BufferDeviceImpl.h"
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

rhi::IBuffer* CreateDeviceBufferSync(
    void const* data,
    size_t dataSize,
    rhi::BufferDesc const& bufferDesc,
    rhi::IDevice* device,
    DeviceResources& deviceResources) {
  if (!data || dataSize == 0 || !device) {
    te::core::Log(te::core::LogLevel::Error, "BufferDeviceImpl::CreateDeviceBufferSync: Invalid parameters");
    return nullptr;
  }

  if (bufferDesc.size == 0) {
    te::core::Log(te::core::LogLevel::Error, "BufferDeviceImpl::CreateDeviceBufferSync: Invalid buffer description");
    return nullptr;
  }

  // Create GPU buffer
  rhi::IBuffer* buffer = device->CreateBuffer(bufferDesc);
  if (!buffer) {
    te::core::Log(te::core::LogLevel::Error, "BufferDeviceImpl::CreateDeviceBufferSync: Failed to create GPU buffer");
    return nullptr;
  }

  // Allocate staging buffer and copy data
  rhi::IBuffer* stagingBuffer = AllocateAndCopyStagingBuffer(
      device,
      deviceResources.stagingBufferManager.get(),
      data,
      dataSize);
  if (!stagingBuffer) {
    te::core::Log(te::core::LogLevel::Error, "BufferDeviceImpl::CreateDeviceBufferSync: Failed to allocate staging buffer");
    device->DestroyBuffer(buffer);
    return nullptr;
  }

  // Create command list for upload
  rhi::ICommandList* cmd = device->CreateCommandList();
  if (!cmd) {
    te::core::Log(te::core::LogLevel::Error, "BufferDeviceImpl::CreateDeviceBufferSync: Failed to create command list");
    deviceResources.stagingBufferManager->Release(stagingBuffer);
    device->DestroyBuffer(buffer);
    return nullptr;
  }

  // Begin command list
  cmd->Begin();

  // Resource barrier: Common -> CopyDst
  rhi::BufferBarrier barrier{};
  barrier.buffer = buffer;
  barrier.offset = 0;
  barrier.size = bufferDesc.size;
  barrier.srcState = rhi::ResourceState::Common;
  barrier.dstState = rhi::ResourceState::CopyDst;
  cmd->ResourceBarrier(1, &barrier, 0, nullptr);

  // Copy staging buffer to GPU buffer
  cmd->CopyBuffer(stagingBuffer, 0, buffer, 0, dataSize);

  // Resource barrier: CopyDst -> VertexBuffer/IndexBuffer (based on usage)
  rhi::ResourceState finalState = rhi::ResourceState::Common;
  if (bufferDesc.usage & static_cast<uint32_t>(rhi::BufferUsage::Vertex)) {
    finalState = rhi::ResourceState::VertexBuffer;
  } else if (bufferDesc.usage & static_cast<uint32_t>(rhi::BufferUsage::Index)) {
    finalState = rhi::ResourceState::IndexBuffer;
  }
  
  barrier.srcState = rhi::ResourceState::CopyDst;
  barrier.dstState = finalState;
  cmd->ResourceBarrier(1, &barrier, 0, nullptr);

  // End command list
  cmd->End();

  // Submit command list synchronously
  if (!SubmitCommandListSync(cmd, device)) {
    device->DestroyCommandList(cmd);
    deviceResources.stagingBufferManager->Release(stagingBuffer);
    device->DestroyBuffer(buffer);
    return nullptr;
  }

  // Cleanup
  device->DestroyCommandList(cmd);
  deviceResources.stagingBufferManager->Release(stagingBuffer);

  return buffer;
}

void AsyncBufferCreateWorker(void* ctx) {
  auto* context = static_cast<AsyncBufferCreateContext*>(ctx);
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
      context->data,
      context->dataSize);
  if (!context->stagingBuffer) {
    te::core::Log(te::core::LogLevel::Error, "BufferDeviceImpl::AsyncBufferCreateWorker: Failed to allocate staging buffer");
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
    te::core::Log(te::core::LogLevel::Error, "BufferDeviceImpl::AsyncBufferCreateWorker: Failed to acquire command list");
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
  rhi::BufferBarrier barrier{};
  barrier.buffer = context->buffer;
  barrier.offset = 0;
  barrier.size = context->bufferDesc.size;
  barrier.srcState = rhi::ResourceState::Common;
  barrier.dstState = rhi::ResourceState::CopyDst;
  context->cmd->ResourceBarrier(1, &barrier, 0, nullptr);

  // Copy staging buffer to GPU buffer
  context->cmd->CopyBuffer(context->stagingBuffer, 0, context->buffer, 0, context->dataSize);

  // Resource barrier: CopyDst -> VertexBuffer/IndexBuffer
  rhi::ResourceState finalState = rhi::ResourceState::Common;
  if (context->bufferDesc.usage & static_cast<uint32_t>(rhi::BufferUsage::Vertex)) {
    finalState = rhi::ResourceState::VertexBuffer;
  } else if (context->bufferDesc.usage & static_cast<uint32_t>(rhi::BufferUsage::Index)) {
    finalState = rhi::ResourceState::IndexBuffer;
  }
  
  barrier.srcState = rhi::ResourceState::CopyDst;
  barrier.dstState = finalState;
  context->cmd->ResourceBarrier(1, &barrier, 0, nullptr);

  // End command list
  context->cmd->End();

  context->progress.store(0.7f);  // 70% - commands recorded

  // Create fence for synchronization
  context->fence = context->device->CreateFence(false);
  if (!context->fence) {
    te::core::Log(te::core::LogLevel::Error, "BufferDeviceImpl::AsyncBufferCreateWorker: Failed to create fence");
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

  // Wait for fence in background thread (non-blocking for main thread)
  te::core::IThreadPool* threadPool = te::core::GetThreadPool();
  if (threadPool) {
    threadPool->SubmitTask([](void* ctx) {
      auto* ctxt = static_cast<AsyncBufferCreateContext*>(ctx);
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
      bool success = (ctxt->buffer != nullptr && !wasCancelled);
      ctxt->callback(ctxt->buffer, success, ctxt->user_data);

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
    bool success = (context->buffer != nullptr && !wasCancelled);
    context->callback(context->buffer, success, context->user_data);
    
    // Unregister operation before deleting context
    ResourceOperationHandle handle = reinterpret_cast<ResourceOperationHandle>(context);
    internal::UnregisterOperation(handle);
    
    delete context;
  }
}

ResourceOperationHandle CreateDeviceBufferAsync(
    void const* data,
    size_t dataSize,
    rhi::BufferDesc const& bufferDesc,
    rhi::IDevice* device,
    DeviceResources& deviceResources,
    void (*callback)(rhi::IBuffer* buffer, bool success, void* user_data),
    void* user_data) {
  if (!data || dataSize == 0 || !device || !callback) {
    te::core::Log(te::core::LogLevel::Error, "BufferDeviceImpl::CreateDeviceBufferAsync: Invalid parameters");
    return nullptr;
  }

  if (bufferDesc.size == 0) {
    te::core::Log(te::core::LogLevel::Error, "BufferDeviceImpl::CreateDeviceBufferAsync: Invalid buffer description");
    callback(nullptr, false, user_data);
    return nullptr;
  }

  // Create GPU buffer (synchronous, but fast)
  rhi::IBuffer* buffer = device->CreateBuffer(bufferDesc);
  if (!buffer) {
    te::core::Log(te::core::LogLevel::Error, "BufferDeviceImpl::CreateDeviceBufferAsync: Failed to create GPU buffer");
    callback(nullptr, false, user_data);
    return nullptr;
  }

  // Create async context
  auto* context = new AsyncBufferCreateContext(
      data,
      dataSize,
      bufferDesc,
      device,
      callback,
      user_data,
      deviceResources.commandListPool.get(),
      deviceResources.stagingBufferManager.get());
  context->buffer = buffer;

  // Register operation and get handle
  ResourceOperationHandle handle = internal::RegisterOperation(context);

  // Submit async work to thread pool
  te::core::IThreadPool* threadPool = te::core::GetThreadPool();
  if (threadPool) {
    threadPool->SubmitTask(AsyncBufferCreateWorker, context);
    return handle;
  } else {
    // Fallback: synchronous creation
    te::core::Log(te::core::LogLevel::Error, "BufferDeviceImpl::CreateDeviceBufferAsync: Thread pool not available, falling back to sync");
    AsyncBufferCreateWorker(context);
    return handle;
  }
}

}  // namespace internal
}  // namespace deviceresource
}  // namespace te
