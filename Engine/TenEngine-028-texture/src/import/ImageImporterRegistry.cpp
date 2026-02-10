/**
 * @file ImageImporterRegistry.cpp
 * @brief Image importer registry implementation.
 */

#include <te/texture/import/ImageImporterRegistry.h>
#include <te/texture/import/ImageImporter.h>
#include <te/core/log.h>
#include <te/core/platform.h>
#include <algorithm>
#include <cstring>
#include <cstddef>

namespace te {
namespace texture {
namespace import {

ImageImporterRegistry& ImageImporterRegistry::GetInstance() {
  static ImageImporterRegistry instance;
  return instance;
}

void ImageImporterRegistry::RegisterImporter(IImageImporter* importer, int priority) {
  if (!importer) {
    return;
  }

  if (m_importerCount >= 16) {
    te::core::Log(te::core::LogLevel::Warn, "ImageImporterRegistry: Maximum importer count reached");
    return;
  }

  m_importers[m_importerCount].importer = importer;
  m_importers[m_importerCount].priority = priority;
  ++m_importerCount;

  SortImporters();
}

void ImageImporterRegistry::SortImporters() {
  std::sort(m_importers, m_importers + m_importerCount,
    [](const ImporterEntry& a, const ImporterEntry& b) {
      return a.priority > b.priority;  // Higher priority first
    });
}

ImageFormat ImageImporterRegistry::DetectFormat(const char* filePath) const {
  if (!filePath) {
    return ImageFormat::Unknown;
  }

  // Try to detect by file extension first
  const char* ext = std::strrchr(filePath, '.');
  if (ext) {
    ++ext;  // Skip '.'
    
    // Case-insensitive comparison
    if (std::strcmp(ext, "png") == 0 || std::strcmp(ext, "PNG") == 0) {
      return ImageFormat::PNG;
    } else if (std::strcmp(ext, "jpg") == 0 || std::strcmp(ext, "jpeg") == 0 ||
               std::strcmp(ext, "JPG") == 0 || std::strcmp(ext, "JPEG") == 0) {
      return ImageFormat::JPEG;
    } else if (std::strcmp(ext, "webp") == 0 || std::strcmp(ext, "WEBP") == 0) {
      return ImageFormat::WebP;
    } else if (std::strcmp(ext, "tiff") == 0 || std::strcmp(ext, "tif") == 0 ||
               std::strcmp(ext, "TIFF") == 0 || std::strcmp(ext, "TIF") == 0) {
      return ImageFormat::TIFF;
    } else if (std::strcmp(ext, "exr") == 0 || std::strcmp(ext, "EXR") == 0) {
      return ImageFormat::EXR;
    } else if (std::strcmp(ext, "avif") == 0 || std::strcmp(ext, "AVIF") == 0) {
      return ImageFormat::AVIF;
    } else if (std::strcmp(ext, "heif") == 0 || std::strcmp(ext, "heic") == 0 ||
               std::strcmp(ext, "HEIF") == 0 || std::strcmp(ext, "HEIC") == 0) {
      return ImageFormat::HEIF;
    } else if (std::strcmp(ext, "bmp") == 0 || std::strcmp(ext, "BMP") == 0) {
      return ImageFormat::BMP;
    } else if (std::strcmp(ext, "tga") == 0 || std::strcmp(ext, "TGA") == 0) {
      return ImageFormat::TGA;
    } else if (std::strcmp(ext, "psd") == 0 || std::strcmp(ext, "PSD") == 0) {
      return ImageFormat::PSD;
    } else if (std::strcmp(ext, "gif") == 0 || std::strcmp(ext, "GIF") == 0) {
      return ImageFormat::GIF;
    } else if (std::strcmp(ext, "hdr") == 0 || std::strcmp(ext, "HDR") == 0) {
      return ImageFormat::HDR;
    }
  }

  // Try to detect by file header (magic bytes)
  // Note: This is a simplified implementation. Full implementation would read file header.
  // For now, we rely on extension-based detection and importer's CanImport method.

  return ImageFormat::Unknown;
}

bool ImageImporterRegistry::ImportImage(const char* filePath, ImageImportResult& result) const {
  if (!filePath) {
    return false;
  }

  // Try each importer in priority order
  for (size_t i = 0; i < m_importerCount; ++i) {
    if (m_importers[i].importer && m_importers[i].importer->CanImport(filePath)) {
      if (m_importers[i].importer->Import(filePath, result)) {
        return true;
      }
    }
  }

  te::core::Log(te::core::LogLevel::Error, "ImageImporterRegistry: Failed to import image file");
  return false;
}

bool ImageImporterRegistry::ImportImageFile(char const* filePath, void** outData, size_t* outSize,
                                            int* outWidth, int* outHeight, int* outChannels) const {
  if (!filePath || !outData || !outSize || !outWidth || !outHeight || !outChannels) {
    return false;
  }

  ImageImportResult result;
  if (!ImportImage(filePath, result)) {
    return false;
  }

  *outData = result.pixelData;
  *outSize = result.pixelDataSize;
  *outWidth = static_cast<int>(result.width);
  *outHeight = static_cast<int>(result.height);
  *outChannels = static_cast<int>(result.channels);

  return true;
}

}  // namespace import
}  // namespace texture
}  // namespace te
