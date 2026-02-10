/**
 * @file StbImageImporter.cpp
 * @brief STB image importer implementation.
 */
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <te/texture/import/StbImageImporter.h>
#include <te/texture/import/ImageImporterRegistry.h>
#include <te/core/alloc.h>
#include <te/core/log.h>
#include <cstring>
#include <cstddef>

namespace te {
namespace texture {
namespace import {

bool StbImageImporter::CanImport(const char* filePath) const {
  if (!filePath) {
    return false;
  }

  // Check file extension
  const char* ext = std::strrchr(filePath, '.');
  if (ext) {
    ++ext;  // Skip '.'
    // stb_image supports: PNG, JPG, BMP, TGA, PSD, GIF, HDR, PIC
    if (std::strcmp(ext, "png") == 0 || std::strcmp(ext, "PNG") == 0 ||
        std::strcmp(ext, "jpg") == 0 || std::strcmp(ext, "jpeg") == 0 ||
        std::strcmp(ext, "JPG") == 0 || std::strcmp(ext, "JPEG") == 0 ||
        std::strcmp(ext, "bmp") == 0 || std::strcmp(ext, "BMP") == 0 ||
        std::strcmp(ext, "tga") == 0 || std::strcmp(ext, "TGA") == 0 ||
        std::strcmp(ext, "psd") == 0 || std::strcmp(ext, "PSD") == 0 ||
        std::strcmp(ext, "gif") == 0 || std::strcmp(ext, "GIF") == 0 ||
        std::strcmp(ext, "hdr") == 0 || std::strcmp(ext, "HDR") == 0 ||
        std::strcmp(ext, "pic") == 0 || std::strcmp(ext, "PIC") == 0) {
      return true;
    }
  }

  return false;
}

bool StbImageImporter::Import(const char* filePath, ImageImportResult& result) {
  if (!filePath) {
    return false;
  }

  // Check if file is HDR format
  bool isHDRFile = false;
  const char* ext = std::strrchr(filePath, '.');
  if (ext) {
    ++ext;
    isHDRFile = (std::strcmp(ext, "hdr") == 0 || std::strcmp(ext, "HDR") == 0);
  }

  int width = 0;
  int height = 0;
  int channels = 0;
  int desiredChannels = 4;  // Always load as RGBA
  void* data = nullptr;
  size_t dataSize = 0;

  if (isHDRFile) {
    // Load HDR file using stbi_loadf
    float* imageData = stbi_loadf(filePath, &width, &height, &channels, desiredChannels);
    if (!imageData) {
      te::core::Log(te::core::LogLevel::Error, "StbImageImporter::Import: Failed to load HDR image file");
      return false;
    }

    if (width <= 0 || height <= 0) {
      te::core::Log(te::core::LogLevel::Error, "StbImageImporter::Import: Invalid HDR image dimensions");
      stbi_image_free(imageData);
      return false;
    }

    // Calculate data size (RGBA32_Float = 4 * sizeof(float) bytes per pixel)
    dataSize = static_cast<size_t>(width) * height * desiredChannels * sizeof(float);

    // Allocate output buffer (aligned to 16 bytes for optimal GPU transfer)
    data = te::core::Alloc(dataSize, 16);
    if (!data) {
      te::core::Log(te::core::LogLevel::Error, "StbImageImporter::Import: Failed to allocate memory for HDR");
      stbi_image_free(imageData);
      return false;
    }

    // Copy image data
    std::memcpy(data, imageData, dataSize);

    // Free stb_image data
    stbi_image_free(imageData);

    result.isHDR = true;
  } else {
    // Load LDR image using stbi_load
    unsigned char* imageData = stbi_load(filePath, &width, &height, &channels, desiredChannels);
    if (!imageData) {
      te::core::Log(te::core::LogLevel::Error, "StbImageImporter::Import: Failed to load image file");
      return false;
    }

    if (width <= 0 || height <= 0) {
      te::core::Log(te::core::LogLevel::Error, "StbImageImporter::Import: Invalid image dimensions");
      stbi_image_free(imageData);
      return false;
    }

    // Calculate data size (RGBA8 = 4 bytes per pixel)
    dataSize = static_cast<size_t>(width) * height * desiredChannels;

    // Allocate output buffer (aligned to 16 bytes for optimal GPU transfer)
    data = te::core::Alloc(dataSize, 16);
    if (!data) {
      te::core::Log(te::core::LogLevel::Error, "StbImageImporter::Import: Failed to allocate memory");
      stbi_image_free(imageData);
      return false;
    }

    // Copy image data
    std::memcpy(data, imageData, dataSize);

    // Free stb_image data
    stbi_image_free(imageData);

    result.isHDR = false;
  }

  // Fill result structure
  result.pixelData = data;
  result.pixelDataSize = dataSize;
  result.width = static_cast<uint32_t>(width);
  result.height = static_cast<uint32_t>(height);
  result.depth = 1;
  result.channels = static_cast<uint32_t>(desiredChannels);
  result.format = ImageImporterRegistry::GetInstance().DetectFormat(filePath);

  return true;
}

bool StbImageImporter::ImportImageFile(char const* filePath, void** outData, size_t* outSize,
                                        int* outWidth, int* outHeight, int* outChannels) {
  if (!filePath || !outData || !outSize || !outWidth || !outHeight || !outChannels) {
    return false;
  }

  StbImageImporter importer;
  ImageImportResult result;
  if (!importer.Import(filePath, result)) {
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
