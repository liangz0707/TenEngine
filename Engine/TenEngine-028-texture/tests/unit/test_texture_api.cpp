/**
 * Unit tests for texture API: CreateTexture, ReleaseTexture, GetFormat, GetWidth, GetHeight, GetMipCount.
 */
#include <te/texture/Texture.h>
#include <te/texture/TextureFactory.h>
#include <te/texture/TextureAssetDesc.h>
#include <te/rendercore/resource_desc.hpp>
#include <te/core/alloc.h>
#include <cstring>
#include <cassert>

int main() {
  using namespace te::texture;

  /* Create a minimal valid descriptor: 4x4 RGBA8. */
  uint8_t pixels[4 * 4 * 4];
  std::memset(pixels, 0x80, sizeof(pixels));

  TextureAssetDesc desc;
  desc.formatVersion = 1;
  desc.format = te::rendercore::TextureFormat::RGBA8_UNorm;
  desc.width = 4;
  desc.height = 4;
  desc.depth = 1;
  desc.mipLevels = 1;
  desc.pixelData = pixels;
  desc.pixelDataSize = sizeof(pixels);
  desc.isHDR = false;

  assert(desc.IsValid());

  TextureHandle h = CreateTexture(&desc);
  assert(h != nullptr);
  assert(GetFormat(h) == te::rendercore::TextureFormat::RGBA8_UNorm);
  assert(GetWidth(h) == 4);
  assert(GetHeight(h) == 4);
  assert(GetMipCount(h) == 1);
  assert(GetPixelData(h) != nullptr);
  assert(GetPixelDataSize(h) == sizeof(pixels));

  ReleaseTexture(h);
  return 0;
}
