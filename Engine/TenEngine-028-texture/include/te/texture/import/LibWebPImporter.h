/**
 * @file LibWebPImporter.h
 * @brief WebP image importer using libwebp (optional; requires TENENGINE_USE_LIBWEBP).
 */
#ifndef TE_TEXTURE_IMPORT_LIBWEBP_IMPORTER_H
#define TE_TEXTURE_IMPORT_LIBWEBP_IMPORTER_H

#include <te/texture/import/ImageImporter.h>

namespace te {
namespace texture {
namespace import {

/** WebP importer using libwebp. Registered when TENENGINE_USE_LIBWEBP is defined. */
class LibWebPImporter : public IImageImporter {
 public:
  bool CanImport(char const* filePath) const override;
  bool Import(char const* filePath, ImageImportResult& result) override;
};

}  // namespace import
}  // namespace texture
}  // namespace te

#endif  // TE_TEXTURE_IMPORT_LIBWEBP_IMPORTER_H
