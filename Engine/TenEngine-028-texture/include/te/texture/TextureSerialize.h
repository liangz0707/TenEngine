/**
 * @file TextureSerialize.h
 * @brief Texture pixel data serialization for Save (.texdata file).
 */
#ifndef TE_TEXTURE_TEXTURE_SERIALIZE_H
#define TE_TEXTURE_TEXTURE_SERIALIZE_H

#include <te/texture/Texture.h>
#include <cstddef>

namespace te {
namespace texture {

/**
 * Write texture pixel data to buffer (for .texdata file).
 *
 * @param h Texture handle
 * @param buffer Output buffer (caller-allocated, must be at least GetPixelDataSize(h) bytes)
 * @param size In: buffer capacity; Out: bytes written
 * @return true on success
 */
bool SerializeTextureToBuffer(TextureHandle h, void* buffer, size_t* size);

/**
 * Get size in bytes required to serialize texture pixel data.
 */
size_t GetTexturePixelDataSize(TextureHandle h);

}  // namespace texture
}  // namespace te

#endif  // TE_TEXTURE_TEXTURE_SERIALIZE_H
