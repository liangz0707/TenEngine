/**
 * @file LibWebPImporter.cpp
 * @brief WebP image importer implementation using libwebp.
 */
#ifdef TENENGINE_USE_LIBWEBP

#include <te/texture/import/LibWebPImporter.h>
#include <te/texture/import/ImageImporterRegistry.h>
#include <te/core/alloc.h>
#include <te/core/log.h>
#include <webp/decode.h>
#include <cstring>
#include <cstdio>
#include <cstdint>

namespace te {
namespace texture {
namespace import {

bool LibWebPImporter::CanImport(char const* filePath) const {
  if (!filePath) return false;
  char const* ext = std::strrchr(filePath, '.');
  if (!ext) return false;
  ++ext;
  return (std::strcmp(ext, "webp") == 0 || std::strcmp(ext, "WEBP") == 0);
}

bool LibWebPImporter::Import(char const* filePath, ImageImportResult& result) {
  if (!filePath) return false;
  result = ImageImportResult{};

  FILE* fp = std::fopen(filePath, "rb");
  if (!fp) {
    te::core::Log(te::core::LogLevel::Error, "LibWebPImporter: Failed to open file");
    return false;
  }

  std::fseek(fp, 0, SEEK_END);
  long fileLen = std::ftell(fp);
  std::fseek(fp, 0, SEEK_SET);
  if (fileLen <= 0) {
    std::fclose(fp);
    return false;
  }

  size_t size = static_cast<size_t>(fileLen);
  void* fileData = te::core::Alloc(size, 1);
  if (!fileData) {
    std::fclose(fp);
    return false;
  }
  if (std::fread(fileData, 1, size, fp) != size) {
    te::core::Free(fileData);
    std::fclose(fp);
    return false;
  }
  std::fclose(fp);

  int width = 0;
  int height = 0;
  uint8_t* decoded = WebPDecodeRGBA(static_cast<uint8_t const*>(fileData), size, &width, &height);
  te::core::Free(fileData);
  if (!decoded || width <= 0 || height <= 0) {
    if (decoded) WebPFree(decoded);
    te::core::Log(te::core::LogLevel::Error, "LibWebPImporter: WebP decode failed");
    return false;
  }

  size_t pixelSize = static_cast<size_t>(width) * height * 4;
  void* pixels = te::core::Alloc(pixelSize, 16);
  if (!pixels) {
    WebPFree(decoded);
    return false;
  }
  std::memcpy(pixels, decoded, pixelSize);
  WebPFree(decoded);

  result.pixelData = pixels;
  result.pixelDataSize = pixelSize;
  result.width = static_cast<uint32_t>(width);
  result.height = static_cast<uint32_t>(height);
  result.depth = 1;
  result.channels = 4;
  result.format = ImageFormat::WebP;
  result.isHDR = false;
  return true;
}

}  // namespace import
}  // namespace texture
}  // namespace te

#endif  // TENENGINE_USE_LIBWEBP
