/**
 * @file LibPngImporter.h
 * @brief PNG image importer using libpng (optional; requires TENENGINE_USE_LIBPNG).
 */
#ifndef TE_TEXTURE_IMPORT_LIBPNG_IMPORTER_H
#define TE_TEXTURE_IMPORT_LIBPNG_IMPORTER_H

#include <te/texture/import/ImageImporter.h>

namespace te {
namespace texture {
namespace import {

/** PNG importer using libpng. Registered when TENENGINE_USE_LIBPNG is defined. */
class LibPngImporter : public IImageImporter {
 public:
  bool CanImport(char const* filePath) const override;
  bool Import(char const* filePath, ImageImportResult& result) override;
};

}  // namespace import
}  // namespace texture
}  // namespace te

#endif  // TE_TEXTURE_IMPORT_LIBPNG_IMPORTER_H
