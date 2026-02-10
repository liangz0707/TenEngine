/**
 * @file TextureDevice.h
 * @brief GPU device resource management for texture (contract: specs/_contracts/028-texture-ABI.md).
 */
#ifndef TE_TEXTURE_TEXTURE_DEVICE_H
#define TE_TEXTURE_TEXTURE_DEVICE_H

#include <te/texture/Texture.h>
#include <te/rhi/device.hpp>
#include <cstddef>

namespace te {
namespace texture {

/**
 * Ensure device resources are created for texture (synchronous).
 * Creates GPU texture from pixel data via 030-DeviceResourceManager.
 *
 * @param h Texture handle
 * @param device RHI device
 * @return true on success, false on failure
 */
bool EnsureDeviceResources(TextureHandle h, rhi::IDevice* device);

/**
 * Ensure device resources are created for texture (asynchronous).
 *
 * @param h Texture handle
 * @param device RHI device
 * @param on_done Completion callback: void(*)(void* user_data)
 * @param user_data User data passed to callback
 */
void EnsureDeviceResourcesAsync(TextureHandle h, rhi::IDevice* device,
                                void (*on_done)(void*), void* user_data);

/**
 * Get RHI texture handle. Valid after EnsureDeviceResources completes.
 *
 * @param h Texture handle
 * @return GPU texture handle, or nullptr if not created
 */
rhi::ITexture* GetTextureHandle(TextureHandle h);

/**
 * Destroy GPU texture for this handle. Call before ReleaseTexture when device is known.
 *
 * @param h Texture handle
 * @param device RHI device
 */
void DestroyDeviceTexture(TextureHandle h, rhi::IDevice* device);

}  // namespace texture
}  // namespace te

#endif  // TE_TEXTURE_TEXTURE_DEVICE_H
