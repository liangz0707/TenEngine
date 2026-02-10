/**
 * @file texture_data.hpp
 * @brief Internal texture data structure (028-Texture module).
 */
#ifndef TE_TEXTURE_DETAIL_TEXTURE_DATA_HPP
#define TE_TEXTURE_DETAIL_TEXTURE_DATA_HPP

#include <te/texture/TextureAssetDesc.h>
#include <te/rendercore/resource_desc.hpp>
#include <te/rhi/resources.hpp>
#include <te/core/alloc.h>
#include <cstdint>
#include <cstddef>
#include <memory>

namespace te {
namespace texture {
namespace detail {

/** Custom deleter for te::core::Alloc-allocated memory. */
struct CoreAllocDeleter {
  void operator()(uint8_t* ptr) const {
    if (ptr) {
      te::core::Free(ptr);
    }
  }
};

/**
 * Internal texture data structure.
 * Stores pixel data, format/size, and optional RHI texture handle.
 */
struct TextureData {
  std::unique_ptr<uint8_t[], CoreAllocDeleter> pixelData;
  size_t pixelDataSize = 0;

  rendercore::TextureFormat format = rendercore::TextureFormat::Unknown;
  uint32_t width = 0;
  uint32_t height = 0;
  uint32_t depth = 1;
  uint32_t mipLevels = 1;
  bool isHDR = false;

  /** GPU texture created by EnsureDeviceResources; not owned by 030 after creation. */
  rhi::ITexture* deviceTexture = nullptr;

  uint32_t refCount = 1;

  TextureData() = default;
  ~TextureData();

  TextureData(TextureData const&) = delete;
  TextureData& operator=(TextureData const&) = delete;
};

}  // namespace detail
}  // namespace texture
}  // namespace te

#endif  // TE_TEXTURE_DETAIL_TEXTURE_DATA_HPP
