/**
 * @file TextureFactory.cpp
 * @brief CreateTexture and ReleaseTexture implementation.
 */
#include <te/texture/TextureFactory.h>
#include <te/texture/Texture.h>
#include <te/texture/detail/texture_data.hpp>
#include <te/core/alloc.h>
#include <cstring>

namespace te {
namespace texture {

TextureHandle CreateTexture(TextureAssetDesc const* desc) {
  if (!desc || !desc->IsValid() || !desc->pixelData || desc->pixelDataSize == 0) {
    return nullptr;
  }

  detail::TextureData* data = static_cast<detail::TextureData*>(
      te::core::Alloc(sizeof(detail::TextureData), alignof(detail::TextureData)));
  if (!data) {
    return nullptr;
  }

  new (data) detail::TextureData();

  data->width = desc->width;
  data->height = desc->height;
  data->depth = desc->depth;
  data->mipLevels = desc->mipLevels;
  data->format = desc->format;
  data->isHDR = desc->isHDR;
  data->pixelDataSize = desc->pixelDataSize;

  uint8_t* pixels = static_cast<uint8_t*>(te::core::Alloc(desc->pixelDataSize, 16));
  if (!pixels) {
    data->~TextureData();
    te::core::Free(data);
    return nullptr;
  }
  std::memcpy(pixels, desc->pixelData, desc->pixelDataSize);
  data->pixelData = std::unique_ptr<uint8_t[], detail::CoreAllocDeleter>(pixels);

  return data;
}

void ReleaseTexture(TextureHandle h) {
  if (!h) return;
  detail::TextureData* data = static_cast<detail::TextureData*>(h);
  data->~TextureData();
  te::core::Free(data);
}

}  // namespace texture
}  // namespace te
