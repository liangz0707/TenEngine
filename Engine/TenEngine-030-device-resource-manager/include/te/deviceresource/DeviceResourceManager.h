/** @file DeviceResourceManager.h
 *  030-DeviceResourceManager ABI: DeviceResourceManager static methods for GPU resource creation.
 */
#pragma once

#include <te/rhi/device.hpp>
#include <te/rhi/resources.hpp>
#include <te/deviceresource/ResourceOperationTypes.h>
#include <cstddef>

namespace te {
namespace deviceresource {

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
   * @return Operation handle for status tracking, or nullptr on failure
   */
  static ResourceOperationHandle CreateDeviceTextureAsync(
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
   * @param textureDesc Texture description (width, height, format, etc.) to determine update region
   * @return true on success
   */
  static bool UpdateDeviceTexture(
      rhi::ITexture* texture,
      rhi::IDevice* device,
      void const* data,
      size_t size,
      rhi::TextureDesc const& textureDesc);

  /**
   * Destroy GPU texture.
   * 
   * @param texture GPU texture handle
   * @param device RHI device
   */
  static void DestroyDeviceTexture(
      rhi::ITexture* texture,
      rhi::IDevice* device);

  // Buffer creation (data-oriented interface)
  /**
   * Synchronously create GPU buffer from raw data.
   * 
   * Callers (e.g., 012-Mesh) should call this from EnsureDeviceResources after
   * obtaining vertex/index data from their resource objects.
   * 
   * @param data Buffer data pointer (vertex data or index data)
   * @param dataSize Data size in bytes
   * @param bufferDesc RHI buffer description (size, usage, etc.)
   * @param device RHI device
   * @return GPU buffer handle, or nullptr on failure
   */
  static rhi::IBuffer* CreateDeviceBuffer(
      void const* data,
      size_t dataSize,
      rhi::BufferDesc const& bufferDesc,
      rhi::IDevice* device);

  /**
   * Asynchronously create GPU buffer from raw data.
   * 
   * @param data Buffer data pointer
   * @param dataSize Data size in bytes
   * @param bufferDesc RHI buffer description
   * @param device RHI device
   * @param callback Completion callback: void(*)(rhi::IBuffer* buffer, bool success, void* user_data)
   * @param user_data User data passed to callback
   * @return Operation handle for status tracking, or nullptr on failure
   */
  static ResourceOperationHandle CreateDeviceBufferAsync(
      void const* data,
      size_t dataSize,
      rhi::BufferDesc const& bufferDesc,
      rhi::IDevice* device,
      void (*callback)(rhi::IBuffer* buffer, bool success, void* user_data),
      void* user_data);

  // Operation status query
  /**
   * Get operation status.
   * Thread-safe.
   * 
   * @param handle Operation handle returned by CreateDeviceTextureAsync or CreateDeviceBufferAsync
   * @return Current operation status
   */
  static ResourceOperationStatus GetOperationStatus(ResourceOperationHandle handle);

  /**
   * Get operation progress (0.0 to 1.0).
   * Thread-safe.
   * 
   * @param handle Operation handle
   * @return Progress value (0.0 = started, 1.0 = completed)
   */
  static float GetOperationProgress(ResourceOperationHandle handle);

  /**
   * Cancel operation.
   * Callback will still be called with success=false.
   * Thread-safe.
   * 
   * @param handle Operation handle
   */
  static void CancelOperation(ResourceOperationHandle handle);

  /**
   * Destroy GPU buffer.
   * 
   * @param buffer GPU buffer handle
   * @param device RHI device
   */
  static void DestroyDeviceBuffer(
      rhi::IBuffer* buffer,
      rhi::IDevice* device);

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
