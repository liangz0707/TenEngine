/**
 * @file Texture.h
 * @brief Texture handle and query API (contract: specs/_contracts/028-texture-ABI.md).
 */
#ifndef TE_TEXTURE_TEXTURE_H
#define TE_TEXTURE_TEXTURE_H

#include <te/rendercore/resource_desc.hpp>
#include <cstdint>
#include <cstddef>

namespace te {
namespace texture {

namespace detail {
struct TextureData;
}

/** Opaque texture handle; returned by CreateTexture, released by ReleaseTexture. */
using TextureHandle = detail::TextureData*;

/** Texture description (009-RenderCore); used for format/size queries. */
using TextureDesc = rendercore::TextureDesc;

/** Get texture format. */
rendercore::TextureFormat GetFormat(TextureHandle h);

/** Get texture width. */
uint32_t GetWidth(TextureHandle h);

/** Get texture height. */
uint32_t GetHeight(TextureHandle h);

/** Get mip level count. */
uint32_t GetMipCount(TextureHandle h);

/** Get pointer to pixel data (read-only). Returns nullptr if handle is invalid. */
void const* GetPixelData(TextureHandle h);

/** Get pixel data size in bytes. */
size_t GetPixelDataSize(TextureHandle h);

}  // namespace texture
}  // namespace te

#endif  // TE_TEXTURE_TEXTURE_H
