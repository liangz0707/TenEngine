/**
 * @file TextureFactory.h
 * @brief Texture creation and release (contract: specs/_contracts/028-texture-ABI.md).
 */
#ifndef TE_TEXTURE_TEXTURE_FACTORY_H
#define TE_TEXTURE_TEXTURE_FACTORY_H

#include <te/texture/Texture.h>
#include <te/texture/TextureAssetDesc.h>

namespace te {
namespace texture {

/**
 * Create texture from asset description (memory only).
 * Copies pixel data into internal storage and returns an opaque handle.
 *
 * @param desc Texture asset description (must be valid; pixelData and pixelDataSize used)
 * @return Texture handle, or nullptr on failure
 */
TextureHandle CreateTexture(TextureAssetDesc const* desc);

/**
 * Release texture handle and free associated memory.
 * Does not destroy GPU texture; callers must call EnsureDeviceResources cleanup
 * or TextureResource::Release before releasing the handle.
 *
 * @param h Texture handle to release
 */
void ReleaseTexture(TextureHandle h);

}  // namespace texture
}  // namespace te

#endif  // TE_TEXTURE_TEXTURE_FACTORY_H
