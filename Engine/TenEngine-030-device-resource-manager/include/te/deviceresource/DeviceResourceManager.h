/** @file DeviceResourceManager.h
 *  030-DeviceResourceManager ABI: DeviceResourceManager static methods for GPU resource creation.
 */
#pragma once

#include <te/rhi/device.hpp>
#include <te/rhi/resources.hpp>
#include <te/resource/Resource.h>
#include <cstddef>

namespace te {
namespace deviceresource {

// Forward declarations
struct IResource;

/**
 * DeviceResourceManager provides static methods for creating GPU resources (DResource)
 * from runtime resources (RResource).
 * 
 * Manages command list pools and staging buffers per IDevice for efficient
 * synchronous and asynchronous GPU resource creation.
 */
class DeviceResourceManager {
 public:
  // Texture creation
  /**
   * Synchronously create GPU texture from texture data.
   * 
   * @param pixelData Pointer to pixel data (RGBA8 format)
   * @param pixelDataSize Size of pixel data in bytes
   * @param textureDesc RHI texture description (width, height, format, etc.)
   * @param device RHI device
   * @return GPU texture handle, or nullptr on failure
   */
  static rhi::ITexture* CreateDeviceTexture(
      void const* pixelData,
      size_t pixelDataSize,
      rhi::TextureDesc const& textureDesc,
      rhi::IDevice* device);

  /**
   * Asynchronously create GPU texture from texture data.
   * 
   * @param pixelData Pointer to pixel data (RGBA8 format)
   * @param pixelDataSize Size of pixel data in bytes
   * @param textureDesc RHI texture description (width, height, format, etc.)
   * @param device RHI device
   * @param callback Completion callback: void(*)(rhi::ITexture* texture, bool success, void* user_data)
   * @param user_data User data passed to callback
   * @return true if async operation started successfully
   */
  static bool CreateDeviceTextureAsync(
      void const* pixelData,
      size_t pixelDataSize,
      rhi::TextureDesc const& textureDesc,
      rhi::IDevice* device,
      void (*callback)(rhi::ITexture* texture, bool success, void* user_data),
      void* user_data);

  /**
   * Update GPU texture data.
   * 
   * @param texture GPU texture handle
   * @param device RHI device
   * @param data Source data
   * @param size Data size in bytes
   * @return true on success
   */
  static bool UpdateDeviceTexture(
      rhi::ITexture* texture,
      rhi::IDevice* device,
      void const* data,
      size_t size);

  /**
   * Destroy GPU texture.
   * 
   * @param texture GPU texture handle
   * @param device RHI device
   */
  static void DestroyDeviceTexture(
      rhi::ITexture* texture,
      rhi::IDevice* device);

  // Buffer creation (for Mesh resources)
  /**
   * Synchronously create GPU buffer from mesh resource.
   * 
   * @param meshResource Mesh resource (must be ResourceType::Mesh)
   * @param bufferType Buffer type (Vertex or Index)
   * @param device RHI device
   * @return GPU buffer handle, or nullptr on failure
   */
  static rhi::IBuffer* CreateDeviceBuffer(
      resource::IResource* meshResource,
      rhi::BufferUsage bufferType,
      rhi::IDevice* device);

  /**
   * Asynchronously create GPU buffer from mesh resource.
   * 
   * @param meshResource Mesh resource (must be ResourceType::Mesh)
   * @param bufferType Buffer type (Vertex or Index)
   * @param device RHI device
   * @param callback Completion callback: void(*)(rhi::IBuffer* buffer, bool success, void* user_data)
   * @param user_data User data passed to callback
   * @return true if async operation started successfully
   */
  static bool CreateDeviceBufferAsync(
      resource::IResource* meshResource,
      rhi::BufferUsage bufferType,
      rhi::IDevice* device,
      void (*callback)(rhi::IBuffer* buffer, bool success, void* user_data),
      void* user_data);

  /**
   * Destroy GPU buffer.
   * 
   * @param buffer GPU buffer handle
   * @param device RHI device
   */
  static void DestroyDeviceBuffer(
      rhi::IBuffer* buffer,
      rhi::IDevice* device);

  // Batch operations
  /**
   * Synchronously create multiple GPU resources in batch.
   * 
   * Resources are grouped by type and processed together for efficiency.
   * 
   * @param resources Array of resource pointers
   * @param count Number of resources
   * @param device RHI device
   * @return true if all resources created successfully
   */
  static bool CreateDeviceResourcesBatch(
      resource::IResource** resources,
      size_t count,
      rhi::IDevice* device);

  /**
   * Asynchronously create multiple GPU resources in batch.
   * 
   * @param resources Array of resource pointers
   * @param count Number of resources
   * @param device RHI device
   * @param callback Completion callback: void(*)(resource::IResource** resources, size_t count, bool* success_flags, void* user_data)
   * @param user_data User data passed to callback
   * @return true if async operation started successfully
   */
  static bool CreateDeviceResourcesBatchAsync(
      resource::IResource** resources,
      size_t count,
      rhi::IDevice* device,
      void (*callback)(resource::IResource** resources, size_t count, bool* success_flags, void* user_data),
      void* user_data);

  // Cleanup
  /**
   * Cleanup command list pool and staging buffers for a device.
   * 
   * Call this before destroying the IDevice to free associated resources.
   * 
   * @param device RHI device to cleanup
   */
  static void CleanupDevice(rhi::IDevice* device);

 private:
  // Internal implementation details
  // Command list pools and staging buffer managers are managed per device
};

}  // namespace deviceresource
}  // namespace te
