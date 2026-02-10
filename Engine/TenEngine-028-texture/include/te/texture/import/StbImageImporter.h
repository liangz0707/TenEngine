/**
 * @file StbImageImporter.h
 * @brief STB image importer (PNG, JPEG, BMP, TGA, HDR, etc.).
 */
#ifndef TE_TEXTURE_IMPORT_STB_IMAGE_IMPORTER_H
#define TE_TEXTURE_IMPORT_STB_IMAGE_IMPORTER_H

#include <te/texture/import/ImageImporter.h>
#include <cstddef>

namespace te {
namespace texture {
namespace import {

/** Image importer using stb_image (single-header library). */
class StbImageImporter : public IImageImporter {
 public:
  bool CanImport(char const* filePath) const override;
  bool Import(char const* filePath, ImageImportResult& result) override;

  /** Convenience: import and return raw pointers; caller must free pixelData with te::core::Free. */
  static bool ImportImageFile(char const* filePath, void** outData, size_t* outSize,
                              int* outWidth, int* outHeight, int* outChannels);
};

}  // namespace import
}  // namespace texture
}  // namespace te

#endif  // TE_TEXTURE_IMPORT_STB_IMAGE_IMPORTER_H
