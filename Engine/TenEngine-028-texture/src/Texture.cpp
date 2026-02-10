/**
 * @file Texture.cpp
 * @brief Texture handle query API implementation.
 */
#include <te/texture/Texture.h>
#include <te/texture/detail/texture_data.hpp>

namespace te {
namespace texture {

rendercore::TextureFormat GetFormat(TextureHandle h) {
  if (!h) return rendercore::TextureFormat::Unknown;
  return h->format;
}

uint32_t GetWidth(TextureHandle h) {
  if (!h) return 0;
  return h->width;
}

uint32_t GetHeight(TextureHandle h) {
  if (!h) return 0;
  return h->height;
}

uint32_t GetMipCount(TextureHandle h) {
  if (!h) return 0;
  return h->mipLevels;
}

void const* GetPixelData(TextureHandle h) {
  if (!h) return nullptr;
  return h->pixelData.get();
}

size_t GetPixelDataSize(TextureHandle h) {
  if (!h) return 0;
  return h->pixelDataSize;
}

}  // namespace texture
}  // namespace te
