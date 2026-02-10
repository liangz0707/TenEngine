/**
 * @file LibJpegTurboImporter.h
 * @brief JPEG image importer using libjpeg-turbo (optional; requires TENENGINE_USE_JPEG_TURBO).
 */
#ifndef TE_TEXTURE_IMPORT_LIBJPEG_TURBO_IMPORTER_H
#define TE_TEXTURE_IMPORT_LIBJPEG_TURBO_IMPORTER_H

#include <te/texture/import/ImageImporter.h>

namespace te {
namespace texture {
namespace import {

/** JPEG importer using libjpeg-turbo. Registered when TENENGINE_USE_JPEG_TURBO is defined. */
class LibJpegTurboImporter : public IImageImporter {
 public:
  bool CanImport(char const* filePath) const override;
  bool Import(char const* filePath, ImageImportResult& result) override;
};

}  // namespace import
}  // namespace texture
}  // namespace te

#endif  // TE_TEXTURE_IMPORT_LIBJPEG_TURBO_IMPORTER_H
