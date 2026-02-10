/**
 * @file ImageImporter.h
 * @brief Image importer interface for texture import (PNG, JPEG, etc.).
 */
#ifndef TE_TEXTURE_IMPORT_IMAGE_IMPORTER_H
#define TE_TEXTURE_IMPORT_IMAGE_IMPORTER_H

#include <cstddef>
#include <cstdint>

namespace te {
namespace texture {
namespace import {

enum class ImageFormat {
  Unknown,
  PNG,
  JPEG,
  WebP,
  TIFF,
  EXR,
  AVIF,
  HEIF,
  BMP,
  TGA,
  PSD,
  GIF,
  HDR
};

/** Result of importing an image file; pixelData is allocated by importer (caller must free via te::core::Free). */
struct ImageImportResult {
  void* pixelData = nullptr;
  size_t pixelDataSize = 0;
  uint32_t width = 0;
  uint32_t height = 0;
  uint32_t depth = 1;
  uint32_t channels = 0;
  ImageFormat format = ImageFormat::Unknown;
  bool isHDR = false;
};

/** Interface for image format importers. */
class IImageImporter {
 public:
  virtual ~IImageImporter() = default;
  virtual bool CanImport(char const* filePath) const = 0;
  virtual bool Import(char const* filePath, ImageImportResult& result) = 0;
};

}  // namespace import
}  // namespace texture
}  // namespace te

#endif  // TE_TEXTURE_IMPORT_IMAGE_IMPORTER_H
