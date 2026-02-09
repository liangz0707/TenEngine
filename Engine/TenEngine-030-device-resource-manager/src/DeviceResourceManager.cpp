/** @file DeviceResourceManager.cpp
 *  030-DeviceResourceManager implementation.
 */
#include <te/deviceresource/DeviceResourceManager.h>
#include <te/deviceresource/ResourceOperationTypes.h>
#include "internal/DeviceResources.h"
#include "internal/TextureDeviceImpl.h"
#include "internal/BufferDeviceImpl.h"
#include "internal/ResourceUploadHelper.h"
#include "internal/AsyncOperationContext.h"
#include <te/rhi/device.hpp>
#include <te/rhi/resources.hpp>
#include <te/rhi/command_list.hpp>
#include <te/rhi/types.hpp>
#include <te/core/log.h>
#include <te/core/thread.h>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <cstring>
#include <vector>

namespace te {
namespace deviceresource {

namespace {
using internal::DeviceResources;
using internal::AsyncOperationContext;

// Thread-safe map of device -> resources
std::mutex g_deviceResourcesMutex;
std::unordered_map<rhi::IDevice*, DeviceResources> g_deviceResources;

// Thread-safe map of operation handle -> context
std::mutex g_operationsMutex;
std::unordered_map<ResourceOperationHandle, AsyncOperationContext*> g_operations;

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

// Get operation context
AsyncOperationContext* GetOperationContext(ResourceOperationHandle handle) {
  std::lock_guard<std::mutex> lock(g_operationsMutex);
  auto it = g_operations.find(handle);
  if (it != g_operations.end()) {
    return it->second;
  }
  return nullptr;
}

}  // anonymous namespace

// Internal functions for operation registration (used by internal implementations)
namespace internal {
ResourceOperationHandle RegisterOperation(AsyncOperationContext* context) {
  std::lock_guard<std::mutex> lock(g_operationsMutex);
  ResourceOperationHandle handle = reinterpret_cast<ResourceOperationHandle>(context);
  g_operations[handle] = context;
  return handle;
}

void UnregisterOperation(ResourceOperationHandle handle) {
  std::lock_guard<std::mutex> lock(g_operationsMutex);
  g_operations.erase(handle);
}
}  // namespace internal

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

ResourceOperationHandle DeviceResourceManager::CreateDeviceTextureAsync(
    void const* pixelData,
    size_t pixelDataSize,
    rhi::TextureDesc const& textureDesc,
    rhi::IDevice* device,
    void (*callback)(rhi::ITexture* texture, bool success, void* user_data),
    void* user_data) {
  if (!pixelData || pixelDataSize == 0 || !device || !callback) {
    te::core::Log(te::core::LogLevel::Error, "DeviceResourceManager::CreateDeviceTextureAsync: Invalid parameters");
    return nullptr;
  }

  if (textureDesc.width == 0 || textureDesc.height == 0) {
    te::core::Log(te::core::LogLevel::Error, "DeviceResourceManager::CreateDeviceTextureAsync: Invalid texture dimensions");
    return nullptr;
  }

  // Get device resources and delegate to implementation
  auto& deviceResources = GetDeviceResources(device);
  return internal::CreateDeviceTextureAsync(pixelData, pixelDataSize, textureDesc, device, deviceResources, callback, user_data);
}

bool DeviceResourceManager::UpdateDeviceTexture(
    rhi::ITexture* texture,
    rhi::IDevice* device,
    void const* data,
    size_t size,
    rhi::TextureDesc const& textureDesc) {
  if (!texture || !device || !data || size == 0) {
    te::core::Log(te::core::LogLevel::Error, "DeviceResourceManager::UpdateDeviceTexture: Invalid parameters");
    return false;
  }

  if (textureDesc.width == 0 || textureDesc.height == 0) {
    te::core::Log(te::core::LogLevel::Error, "DeviceResourceManager::UpdateDeviceTexture: Invalid texture dimensions");
    return false;
  }

  // Get device resources and delegate to implementation
  auto& deviceResources = GetDeviceResources(device);
  return internal::UpdateDeviceTexture(texture, device, deviceResources, data, size, textureDesc);
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
    void const* data,
    size_t dataSize,
    rhi::BufferDesc const& bufferDesc,
    rhi::IDevice* device) {
  if (!data || dataSize == 0 || !device) {
    te::core::Log(te::core::LogLevel::Error, "DeviceResourceManager::CreateDeviceBuffer: Invalid parameters");
    return nullptr;
  }

  if (bufferDesc.size == 0) {
    te::core::Log(te::core::LogLevel::Error, "DeviceResourceManager::CreateDeviceBuffer: Invalid buffer description");
    return nullptr;
  }

  // Get device resources and delegate to implementation
  auto& deviceResources = GetDeviceResources(device);
  return internal::CreateDeviceBufferSync(data, dataSize, bufferDesc, device, deviceResources);
}

ResourceOperationHandle DeviceResourceManager::CreateDeviceBufferAsync(
    void const* data,
    size_t dataSize,
    rhi::BufferDesc const& bufferDesc,
    rhi::IDevice* device,
    void (*callback)(rhi::IBuffer* buffer, bool success, void* user_data),
    void* user_data) {
  if (!data || dataSize == 0 || !device || !callback) {
    te::core::Log(te::core::LogLevel::Error, "DeviceResourceManager::CreateDeviceBufferAsync: Invalid parameters");
    return nullptr;
  }

  if (bufferDesc.size == 0) {
    te::core::Log(te::core::LogLevel::Error, "DeviceResourceManager::CreateDeviceBufferAsync: Invalid buffer description");
    return nullptr;
  }

  // Get device resources and delegate to implementation
  auto& deviceResources = GetDeviceResources(device);
  return internal::CreateDeviceBufferAsync(data, dataSize, bufferDesc, device, deviceResources, callback, user_data);
}

ResourceOperationStatus DeviceResourceManager::GetOperationStatus(ResourceOperationHandle handle) {
  if (!handle) {
    return ResourceOperationStatus::Failed;
  }

  AsyncOperationContext* context = GetOperationContext(handle);
  if (!context) {
    return ResourceOperationStatus::Failed;
  }

  return context->status.load();
}

float DeviceResourceManager::GetOperationProgress(ResourceOperationHandle handle) {
  if (!handle) {
    return 0.0f;
  }

  AsyncOperationContext* context = GetOperationContext(handle);
  if (!context) {
    return 0.0f;
  }

  return context->progress.load();
}

void DeviceResourceManager::CancelOperation(ResourceOperationHandle handle) {
  if (!handle) {
    return;
  }

  AsyncOperationContext* context = GetOperationContext(handle);
  if (!context) {
    return;
  }

  context->Cancel();
}

void DeviceResourceManager::DestroyDeviceBuffer(
    rhi::IBuffer* buffer,
    rhi::IDevice* device) {
  if (!buffer || !device) {
    return;
  }

  device->DestroyBuffer(buffer);
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
