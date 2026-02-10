/**
 * @file LibPngImporter.cpp
 * @brief PNG image importer implementation using libpng.
 */
#ifdef TENENGINE_USE_LIBPNG

#include <te/texture/import/LibPngImporter.h>
#include <te/texture/import/ImageImporterRegistry.h>
#include <te/core/alloc.h>
#include <te/core/log.h>
#include <png.h>
#include <cstring>
#include <cstdint>
#include <cstdio>
#include <vector>

namespace te {
namespace texture {
namespace import {

bool LibPngImporter::CanImport(char const* filePath) const {
  if (!filePath) return false;
  char const* ext = std::strrchr(filePath, '.');
  if (!ext) return false;
  ++ext;
  return (std::strcmp(ext, "png") == 0 || std::strcmp(ext, "PNG") == 0);
}

bool LibPngImporter::Import(char const* filePath, ImageImportResult& result) {
  if (!filePath) return false;
  result = ImageImportResult{};

  FILE* fp = std::fopen(filePath, "rb");
  if (!fp) {
    te::core::Log(te::core::LogLevel::Error, "LibPngImporter: Failed to open file");
    return false;
  }

  unsigned char sig[8];
  if (std::fread(sig, 1, 8, fp) != 8 || png_sig_cmp(sig, 0, 8) != 0) {
    std::fclose(fp);
    te::core::Log(te::core::LogLevel::Error, "LibPngImporter: Not a PNG file");
    return false;
  }

  png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
  if (!png) {
    std::fclose(fp);
    return false;
  }
  png_infop info = png_create_info_struct(png);
  if (!info) {
    png_destroy_read_struct(&png, nullptr, nullptr);
    std::fclose(fp);
    return false;
  }

  if (setjmp(png_jmpbuf(png))) {
    png_destroy_read_struct(&png, &info, nullptr);
    std::fclose(fp);
    if (result.pixelData) {
      te::core::Free(result.pixelData);
      result.pixelData = nullptr;
    }
    return false;
  }

  png_init_io(png, fp);
  png_set_sig_bytes(png, 8);
  png_read_info(png, info);

  png_uint_32 width = png_get_image_width(png, info);
  png_uint_32 height = png_get_image_height(png, info);
  png_byte color_type = png_get_color_type(png, info);
  png_byte bit_depth = png_get_bit_depth(png, info);

  if (bit_depth == 16)
    png_set_strip_16(png);
  if (color_type == PNG_COLOR_TYPE_PALETTE)
    png_set_palette_to_rgb(png);
  if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
    png_set_expand_gray_1_2_4_to_8(png);
  if (png_get_valid(png, info, PNG_INFO_tRNS))
    png_set_tRNS_to_alpha(png);
  if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY)
    png_set_add_alpha(png, 0xFF, PNG_FILLER_AFTER);
  if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
    png_set_gray_to_rgb(png);

  png_read_update_info(png, info);

  png_size_t rowbytes = png_get_rowbytes(png, info);
  size_t total_size = static_cast<size_t>(rowbytes) * height;
  void* pixels = te::core::Alloc(total_size, 16);
  if (!pixels) {
    png_destroy_read_struct(&png, &info, nullptr);
    std::fclose(fp);
    return false;
  }

  std::vector<png_bytep> row_ptrs(height);
  for (png_uint_32 y = 0; y < height; ++y)
    row_ptrs[y] = static_cast<png_bytep>(pixels) + y * rowbytes;

  png_read_image(png, row_ptrs.data());
  png_read_end(png, info);
  png_destroy_read_struct(&png, &info, nullptr);
  std::fclose(fp);

  result.pixelData = pixels;
  result.pixelDataSize = total_size;
  result.width = width;
  result.height = height;
  result.depth = 1;
  result.channels = 4;
  result.format = ImageFormat::PNG;
  result.isHDR = false;
  return true;
}

}  // namespace import
}  // namespace texture
}  // namespace te

#endif  // TENENGINE_USE_LIBPNG
