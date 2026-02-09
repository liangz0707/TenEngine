/** @file DeviceResourceManager.cpp
 *  030-DeviceResourceManager implementation.
 */
#include <te/deviceresource/DeviceResourceManager.h>
#include "internal/DeviceResources.h"
#include "internal/TextureDeviceImpl.h"
#include "internal/ResourceUploadHelper.h"
#include <te/rhi/device.hpp>
#include <te/rhi/resources.hpp>
#include <te/rhi/command_list.hpp>
#include <te/rhi/types.hpp>
#include <te/resource/Resource.h>
#include <te/resource/ResourceTypes.h>
#include <te/core/log.h>
#include <te/core/thread.h>
// Note: Batch operations need TextureResource for type casting
// This is a delayed include - only in implementation file, not in header
#ifdef TENENGINE_028_TEXTURE_AVAILABLE
#include <te/texture/TextureResource.h>
#endif
#include <unordered_map>
#include <mutex>
#include <memory>
#include <cstring>
#include <vector>

namespace te {
namespace deviceresource {

namespace {
using internal::DeviceResources;

// Thread-safe map of device -> resources
std::mutex g_deviceResourcesMutex;
std::unordered_map<rhi::IDevice*, DeviceResources> g_deviceResources;

// Get or create device resources
DeviceResources& GetDeviceResources(rhi::IDevice* device) {
  std::lock_guard<std::mutex> lock(g_deviceResourcesMutex);
  auto it = g_deviceResources.find(device);
  if (it == g_deviceResources.end()) {
    // Create new device resources
    g_deviceResources.emplace(device, DeviceResources(device));
    return g_deviceResources[device];
  }
  return it->second;
}

}  // anonymous namespace

rhi::ITexture* DeviceResourceManager::CreateDeviceTexture(
    void const* pixelData,
    size_t pixelDataSize,
    rhi::TextureDesc const& textureDesc,
    rhi::IDevice* device) {
  if (!pixelData || pixelDataSize == 0 || !device) {
    te::core::Log(te::core::LogLevel::Error, "DeviceResourceManager::CreateDeviceTexture: Invalid parameters");
    return nullptr;
  }

  if (textureDesc.width == 0 || textureDesc.height == 0) {
    te::core::Log(te::core::LogLevel::Error, "DeviceResourceManager::CreateDeviceTexture: Invalid texture dimensions");
    return nullptr;
  }

  // Get device resources and delegate to implementation
  auto& deviceResources = GetDeviceResources(device);
  return internal::CreateDeviceTextureSync(pixelData, pixelDataSize, textureDesc, device, deviceResources);
}

bool DeviceResourceManager::CreateDeviceTextureAsync(
    void const* pixelData,
    size_t pixelDataSize,
    rhi::TextureDesc const& textureDesc,
    rhi::IDevice* device,
    void (*callback)(rhi::ITexture* texture, bool success, void* user_data),
    void* user_data) {
  if (!pixelData || pixelDataSize == 0 || !device || !callback) {
    te::core::Log(te::core::LogLevel::Error, "DeviceResourceManager::CreateDeviceTextureAsync: Invalid parameters");
    return false;
  }

  if (textureDesc.width == 0 || textureDesc.height == 0) {
    te::core::Log(te::core::LogLevel::Error, "DeviceResourceManager::CreateDeviceTextureAsync: Invalid texture dimensions");
    return false;
  }

  // Get device resources and delegate to implementation
  auto& deviceResources = GetDeviceResources(device);
  return internal::CreateDeviceTextureAsync(pixelData, pixelDataSize, textureDesc, device, deviceResources, callback, user_data);
}

bool DeviceResourceManager::UpdateDeviceTexture(
    rhi::ITexture* texture,
    rhi::IDevice* device,
    void const* data,
    size_t size) {
  if (!texture || !device || !data || size == 0) {
    te::core::Log(te::core::LogLevel::Error, "DeviceResourceManager::UpdateDeviceTexture: Invalid parameters");
    return false;
  }

  // Get device resources and delegate to implementation
  auto& deviceResources = GetDeviceResources(device);
  return internal::UpdateDeviceTexture(texture, device, deviceResources, data, size);
}

void DeviceResourceManager::DestroyDeviceTexture(
    rhi::ITexture* texture,
    rhi::IDevice* device) {
  if (!texture || !device) {
    return;
  }

  device->DestroyTexture(texture);
}

rhi::IBuffer* DeviceResourceManager::CreateDeviceBuffer(
    resource::IResource* meshResource,
    rhi::BufferUsage bufferType,
    rhi::IDevice* device) {
  if (!meshResource || !device) {
    te::core::Log(te::core::LogLevel::Error, "DeviceResourceManager::CreateDeviceBuffer: Invalid parameters");
    return nullptr;
  }

  // Check resource type
  if (meshResource->GetResourceType() != resource::ResourceType::Mesh) {
    te::core::Log(te::core::LogLevel::Error, "DeviceResourceManager::CreateDeviceBuffer: Resource is not a Mesh");
    return nullptr;
  }

  // TODO: Implement buffer creation
  te::core::Log(te::core::LogLevel::Error, "DeviceResourceManager::CreateDeviceBuffer: Not implemented yet");
  return nullptr;
}

bool DeviceResourceManager::CreateDeviceBufferAsync(
    resource::IResource* meshResource,
    rhi::BufferUsage bufferType,
    rhi::IDevice* device,
    void (*callback)(rhi::IBuffer* buffer, bool success, void* user_data),
    void* user_data) {
  if (!meshResource || !device || !callback) {
    te::core::Log(te::core::LogLevel::Error, "DeviceResourceManager::CreateDeviceBufferAsync: Invalid parameters");
    return false;
  }

  // TODO: Implement async buffer creation
  te::core::Log(te::core::LogLevel::Error, "DeviceResourceManager::CreateDeviceBufferAsync: Not implemented yet");
  return false;
}

void DeviceResourceManager::DestroyDeviceBuffer(
    rhi::IBuffer* buffer,
    rhi::IDevice* device) {
  if (!buffer || !device) {
    return;
  }

  device->DestroyBuffer(buffer);
}

bool DeviceResourceManager::CreateDeviceResourcesBatch(
    resource::IResource** resources,
    size_t count,
    rhi::IDevice* device) {
  if (!resources || count == 0 || !device) {
    te::core::Log(te::core::LogLevel::Error, "DeviceResourceManager::CreateDeviceResourcesBatch: Invalid parameters");
    return false;
  }

  // Group resources by type
  std::vector<resource::IResource*> textureResources;
  std::vector<resource::IResource*> meshResources;
  std::vector<resource::IResource*> materialResources;

  for (size_t i = 0; i < count; ++i) {
    if (!resources[i]) {
      continue;
    }

    resource::ResourceType type = resources[i]->GetResourceType();
    switch (type) {
      case resource::ResourceType::Texture:
        textureResources.push_back(resources[i]);
        break;
      case resource::ResourceType::Mesh:
        meshResources.push_back(resources[i]);
        break;
      case resource::ResourceType::Material:
        materialResources.push_back(resources[i]);
        break;
      default:
        te::core::Log(te::core::LogLevel::Error, "DeviceResourceManager::CreateDeviceResourcesBatch: Unsupported resource type");
        break;
    }
  }

  bool allSuccess = true;
  auto& deviceResources = GetDeviceResources(device);

  // Process textures in batch (one command list per type)
  if (!textureResources.empty()) {
    rhi::ICommandList* cmd = device->CreateCommandList();
    if (!cmd) {
      te::core::Log(te::core::LogLevel::Error, "DeviceResourceManager::CreateDeviceResourcesBatch: Failed to create command list");
      return false;
    }

    cmd->Begin();

    // Track textures and staging buffers for cleanup
    std::vector<rhi::ITexture*> createdTextures;
    std::vector<rhi::IBuffer*> stagingBuffers;

    // Process each texture
    // Note: Batch operations need TextureResource for type casting and data access
    // This is a delayed include - only in implementation file, not in header
    // TODO: Consider refactoring batch operations to use a callback-based approach
    for (resource::IResource* res : textureResources) {
      // Try to cast to TextureResource (requires 028-texture header)
      // For now, we'll need to include TextureResource.h for batch operations
      // This is acceptable as it's only in the implementation file
      // In the future, we could use a callback-based approach where 028-texture
      // provides the data extraction callback
      
      // For now, batch operations are disabled until we have a better design
      // that doesn't require direct TextureResource access
      te::core::Log(te::core::LogLevel::Error, "DeviceResourceManager::CreateDeviceResourcesBatch: Texture batch creation temporarily disabled - use individual CreateDeviceTexture calls");
      allSuccess = false;
      break;
    }

    // Final barrier: CopyDst -> ShaderResource for all textures
    if (!createdTextures.empty()) {
      std::vector<rhi::TextureBarrier> finalBarriers;
      for (rhi::ITexture* texture : createdTextures) {
        rhi::TextureBarrier barrier{};
        barrier.texture = texture;
        barrier.mipLevel = 0;
        barrier.arrayLayer = 0;
        barrier.srcState = rhi::ResourceState::CopyDst;
        barrier.dstState = rhi::ResourceState::ShaderResource;
        finalBarriers.push_back(barrier);
      }
      cmd->ResourceBarrier(0, nullptr, static_cast<uint32_t>(finalBarriers.size()), finalBarriers.data());
    }

    cmd->End();

    // Submit command list synchronously
    if (!internal::SubmitCommandListSync(cmd, device)) {
      device->DestroyCommandList(cmd);
      // Cleanup created textures and staging buffers
      for (rhi::ITexture* texture : createdTextures) {
        device->DestroyTexture(texture);
      }
      for (rhi::IBuffer* stagingBuffer : stagingBuffers) {
        deviceResources.stagingBufferManager->Release(stagingBuffer);
      }
      return false;
    }

    // Textures are already stored in TextureResource during creation above

    // Release staging buffers
    for (rhi::IBuffer* stagingBuffer : stagingBuffers) {
      deviceResources.stagingBufferManager->Release(stagingBuffer);
    }

    // Cleanup command list
    device->DestroyCommandList(cmd);
  }

  // TODO: Process mesh resources in batch
  if (!meshResources.empty()) {
    te::core::Log(te::core::LogLevel::Error, "DeviceResourceManager::CreateDeviceResourcesBatch: Mesh batch creation not implemented yet");
    allSuccess = false;
  }

  // TODO: Process material resources in batch
  if (!materialResources.empty()) {
    te::core::Log(te::core::LogLevel::Error, "DeviceResourceManager::CreateDeviceResourcesBatch: Material batch creation not implemented yet");
    allSuccess = false;
  }

  return allSuccess;
}

namespace {
// Async batch operation context
struct AsyncBatchCreateContext {
  resource::IResource** resources;
  size_t count;
  rhi::IDevice* device;
  void (*callback)(resource::IResource** resources, size_t count, bool* success_flags, void* user_data);
  void* user_data;
  bool* successFlags;

  AsyncBatchCreateContext(resource::IResource** res, size_t cnt, rhi::IDevice* dev,
                          void (*cb)(resource::IResource**, size_t, bool*, void*), void* ud)
      : resources(res), count(cnt), device(dev), callback(cb), user_data(ud) {
    successFlags = new bool[count];
    std::memset(successFlags, 0, count * sizeof(bool));
  }

  ~AsyncBatchCreateContext() {
    delete[] successFlags;
  }
};

// Worker function for async batch creation
void AsyncBatchCreateWorker(void* ctx) {
  auto* context = static_cast<AsyncBatchCreateContext*>(ctx);
  if (!context) {
    return;
  }

  // Perform batch creation synchronously in worker thread
  bool allSuccess = DeviceResourceManager::CreateDeviceResourcesBatch(
      context->resources, context->count, context->device);

  // Set success flags (simplified - all or nothing)
  for (size_t i = 0; i < context->count; ++i) {
    context->successFlags[i] = allSuccess;
  }

  // Call callback in callback thread
  te::core::IThreadPool* threadPool = te::core::GetThreadPool();
  if (threadPool) {
    threadPool->SubmitTask([](void* ctx) {
      auto* ctxt = static_cast<AsyncBatchCreateContext*>(ctx);
      if (ctxt && ctxt->callback) {
        ctxt->callback(ctxt->resources, ctxt->count, ctxt->successFlags, ctxt->user_data);
      }
      delete ctxt;
    }, context);
  } else {
    // Fallback: call callback directly
    if (context->callback) {
      context->callback(context->resources, context->count, context->successFlags, context->user_data);
    }
    delete context;
  }
}
}  // anonymous namespace

bool DeviceResourceManager::CreateDeviceResourcesBatchAsync(
    resource::IResource** resources,
    size_t count,
    rhi::IDevice* device,
    void (*callback)(resource::IResource** resources, size_t count, bool* success_flags, void* user_data),
    void* user_data) {
  if (!resources || count == 0 || !device || !callback) {
    te::core::Log(te::core::LogLevel::Error, "DeviceResourceManager::CreateDeviceResourcesBatchAsync: Invalid parameters");
    return false;
  }

  // Create async context
  auto* context = new AsyncBatchCreateContext(resources, count, device, callback, user_data);

  // Submit async work to thread pool
  te::core::IThreadPool* threadPool = te::core::GetThreadPool();
  if (threadPool) {
    threadPool->SubmitTask(AsyncBatchCreateWorker, context);
    return true;
  } else {
    // Fallback: synchronous creation
    te::core::Log(te::core::LogLevel::Error, "DeviceResourceManager::CreateDeviceResourcesBatchAsync: Thread pool not available, falling back to sync");
    AsyncBatchCreateWorker(context);
    return true;
  }
}

void DeviceResourceManager::CleanupDevice(rhi::IDevice* device) {
  if (!device) {
    return;
  }

  std::lock_guard<std::mutex> lock(g_deviceResourcesMutex);
  auto it = g_deviceResources.find(device);
  if (it != g_deviceResources.end()) {
    // Cleanup is handled by destructors
    g_deviceResources.erase(it);
  }
}

}  // namespace deviceresource
}  // namespace te
