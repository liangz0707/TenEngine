/**
 * @file ImageImporterRegistry.h
 * @brief Registry of image importers for texture Import.
 */
#ifndef TE_TEXTURE_IMPORT_IMAGE_IMPORTER_REGISTRY_H
#define TE_TEXTURE_IMPORT_IMAGE_IMPORTER_REGISTRY_H

#include <te/texture/import/ImageImporter.h>
#include <cstddef>

namespace te {
namespace texture {
namespace import {

class IImageImporter;

/** Singleton registry; use GetInstance() and register importers (e.g. StbImageImporter). */
class ImageImporterRegistry {
 public:
  static ImageImporterRegistry& GetInstance();

  void RegisterImporter(IImageImporter* importer, int priority = 0);
  ImageFormat DetectFormat(char const* filePath) const;
  bool ImportImage(char const* filePath, ImageImportResult& result) const;
  bool ImportImageFile(char const* filePath, void** outData, size_t* outSize,
                      int* outWidth, int* outHeight, int* outChannels) const;

 private:
  ImageImporterRegistry() = default;
  void SortImporters();

  struct ImporterEntry {
    IImageImporter* importer = nullptr;
    int priority = 0;
  };
  static constexpr size_t kMaxImporters = 16;
  ImporterEntry m_importers[kMaxImporters];
  size_t m_importerCount = 0;
};

}  // namespace import
}  // namespace texture
}  // namespace te

#endif  // TE_TEXTURE_IMPORT_IMAGE_IMPORTER_REGISTRY_H
