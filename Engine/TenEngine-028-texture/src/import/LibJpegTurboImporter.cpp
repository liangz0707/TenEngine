/**
 * @file LibJpegTurboImporter.cpp
 * @brief JPEG image importer implementation using libjpeg-turbo.
 */
#ifdef TENENGINE_USE_JPEG_TURBO

#include <te/texture/import/LibJpegTurboImporter.h>
#include <te/texture/import/ImageImporterRegistry.h>
#include <te/core/alloc.h>
#include <te/core/log.h>
#include <jpeglib.h>
#include <cstring>
#include <cstdio>
#include <csetjmp>

namespace te {
namespace texture {
namespace import {

namespace {

struct JpegErrorMgr {
  struct jpeg_error_mgr pub;
  std::jmp_buf jmpBuf;
};

void JpegErrorExit(j_common_ptr cinfo) {
  JpegErrorMgr* err = reinterpret_cast<JpegErrorMgr*>(cinfo->err);
  std::longjmp(err->jmpBuf, 1);
}

}  // namespace

bool LibJpegTurboImporter::CanImport(char const* filePath) const {
  if (!filePath) return false;
  char const* ext = std::strrchr(filePath, '.');
  if (!ext) return false;
  ++ext;
  return (std::strcmp(ext, "jpg") == 0 || std::strcmp(ext, "jpeg") == 0 ||
          std::strcmp(ext, "JPG") == 0 || std::strcmp(ext, "JPEG") == 0);
}

bool LibJpegTurboImporter::Import(char const* filePath, ImageImportResult& result) {
  if (!filePath) return false;
  result = ImageImportResult{};

  FILE* fp = std::fopen(filePath, "rb");
  if (!fp) {
    te::core::Log(te::core::LogLevel::Error, "LibJpegTurboImporter: Failed to open file");
    return false;
  }

  struct jpeg_decompress_struct cinfo;
  JpegErrorMgr jerr;
  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = JpegErrorExit;

  if (setjmp(jerr.jmpBuf)) {
    jpeg_destroy_decompress(&cinfo);
    std::fclose(fp);
    if (result.pixelData) {
      te::core::Free(result.pixelData);
      result.pixelData = nullptr;
    }
    return false;
  }

  jpeg_create_decompress(&cinfo);
  jpeg_stdio_src(&cinfo, fp);
  jpeg_read_header(&cinfo, TRUE);
  cinfo.out_color_space = JCS_EXT_RGBX;  /* request RGBA output (alpha unused) */
  jpeg_start_decompress(&cinfo);

  uint32_t width = cinfo.output_width;
  uint32_t height = cinfo.output_height;
  uint32_t channels = 4;
  size_t row_stride = width * channels;
  size_t total_size = row_stride * height;

  void* pixels = te::core::Alloc(total_size, 16);
  if (!pixels) {
    jpeg_destroy_decompress(&cinfo);
    std::fclose(fp);
    return false;
  }

  unsigned char* dst = static_cast<unsigned char*>(pixels);
  while (cinfo.output_scanline < height) {
    unsigned char* row = dst + cinfo.output_scanline * row_stride;
    if (jpeg_read_scanlines(&cinfo, &row, 1) != 1) break;
  }

  jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);
  std::fclose(fp);

  result.pixelData = pixels;
  result.pixelDataSize = total_size;
  result.width = width;
  result.height = height;
  result.depth = 1;
  result.channels = channels;
  result.format = ImageFormat::JPEG;
  result.isHDR = false;
  return true;
}

}  // namespace import
}  // namespace texture
}  // namespace te

#endif  // TENENGINE_USE_JPEG_TURBO
