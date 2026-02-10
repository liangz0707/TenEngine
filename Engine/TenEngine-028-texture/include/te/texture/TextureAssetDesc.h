/**
 * @file TextureAssetDesc.h
 * @brief Texture asset description (contract: specs/_contracts/028-texture-ABI.md).
 *
 * TextureAssetDesc is owned by 028-Texture. Serialized format: .texture (AssetDesc) + .texdata (binary pixel data).
 */
#ifndef TE_TEXTURE_TEXTURE_ASSET_DESC_H
#define TE_TEXTURE_TEXTURE_ASSET_DESC_H

#include <te/rendercore/resource_desc.hpp>
#include <string>
#include <cstdint>
#include <cstddef>

namespace te {
namespace texture {

/**
 * Texture asset description.
 * Contains all data needed to create a texture resource.
 *
 * pixelData is not serialized; it points to data stored in the .texdata file.
 * During serialization, pixelData is set to nullptr and pixelDataSize is stored.
 */
struct TextureAssetDesc {
  uint32_t formatVersion = 1;
  std::string debugDescription;

  rendercore::TextureFormat format = rendercore::TextureFormat::Unknown;
  uint32_t width = 0;
  uint32_t height = 0;
  uint32_t depth = 1;
  uint32_t mipLevels = 1;

  /** Pointer to pixel data; not serialized, loaded from .texdata file. */
  void* pixelData = nullptr;
  size_t pixelDataSize = 0;

  /** Whether pixel data is HDR (e.g. float). */
  bool isHDR = false;

  bool IsValid() const {
    return width > 0 && height > 0 && format != rendercore::TextureFormat::Unknown && pixelDataSize > 0;
  }
};

}  // namespace texture
}  // namespace te

#endif  // TE_TEXTURE_TEXTURE_ASSET_DESC_H
