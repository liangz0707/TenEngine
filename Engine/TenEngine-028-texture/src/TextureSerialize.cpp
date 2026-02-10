/**
 * @file TextureSerialize.cpp
 * @brief Texture pixel data serialization implementation.
 */
#include <te/texture/TextureSerialize.h>
#include <te/texture/Texture.h>
#include <te/texture/detail/texture_data.hpp>
#include <cstring>

namespace te {
namespace texture {

bool SerializeTextureToBuffer(TextureHandle h, void* buffer, size_t* size) {
  if (!h || !size) return false;
  size_t dataSize = GetPixelDataSize(h);
  if (dataSize == 0) {
    *size = 0;
    return true;
  }
  if (!buffer || *size < dataSize) {
    *size = dataSize;
    return false;
  }
  void const* pixels = GetPixelData(h);
  if (!pixels) return false;
  std::memcpy(buffer, pixels, dataSize);
  *size = dataSize;
  return true;
}

size_t GetTexturePixelDataSize(TextureHandle h) {
  return GetPixelDataSize(h);
}

}  // namespace texture
}  // namespace te
