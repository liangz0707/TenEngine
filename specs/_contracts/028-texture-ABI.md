# 028-Texture Module ABI

- **Contract**: [028-texture-public-api.md](./028-texture-public-api.md) (capabilities and type descriptions)
- **This file**: 028-Texture public ABI explicit table.
- **Note**: This module is CPU-only; GPU resources are created via 030-DeviceResourceManager.

## ABI Table

Column definitions: **Module | Namespace | Symbol/Type | Description | Header**

### Core Handle and Factory

| Module | Namespace | Symbol | Description | Header |
|--------|-----------|--------|-------------|--------|
| 028-Texture | te::texture | TextureHandle | Opaque texture handle; `using TextureHandle = detail::TextureData*;` | te/texture/Texture.h |
| 028-Texture | te::texture | TextureDesc | Texture description (009-RenderCore); `using TextureDesc = rendercore::TextureDesc;` | te/texture/Texture.h |
| 028-Texture | te::texture | GetFormat | `rendercore::TextureFormat GetFormat(TextureHandle h);` | te/texture/Texture.h |
| 028-Texture | te::texture | GetWidth | `uint32_t GetWidth(TextureHandle h);` | te/texture/Texture.h |
| 028-Texture | te::texture | GetHeight | `uint32_t GetHeight(TextureHandle h);` | te/texture/Texture.h |
| 028-Texture | te::texture | GetMipCount | `uint32_t GetMipCount(TextureHandle h);` | te/texture/Texture.h |
| 028-Texture | te::texture | GetPixelData | `void const* GetPixelData(TextureHandle h);` Returns nullptr if invalid | te/texture/Texture.h |
| 028-Texture | te::texture | GetPixelDataSize | `size_t GetPixelDataSize(TextureHandle h);` | te/texture/Texture.h |
| 028-Texture | te::texture | CreateTexture(TextureAssetDesc const*) | `TextureHandle CreateTexture(TextureAssetDesc const* desc);` Returns nullptr on failure | te/texture/TextureFactory.h |
| 028-Texture | te::texture | ReleaseTexture(TextureHandle) | `void ReleaseTexture(TextureHandle h);` Frees CPU pixel data | te/texture/TextureFactory.h |

### TextureModule (CPU-Only)

| Module | Namespace | Symbol | Description | Header |
|--------|-----------|--------|-------------|--------|
| 028-Texture | te::texture | TextureModule | CPU-only module; no GPU texture cache or device | te/texture/TextureModule.h |
| 028-Texture | te::texture | TextureModule::GetInstance() | `static TextureModule& GetInstance();` Global singleton | te/texture/TextureModule.h |
| 028-Texture | te::texture | TextureModule::SetResourceManager | `void SetResourceManager(resource::IResourceManager* manager);` | te/texture/TextureModule.h |
| 028-Texture | te::texture | TextureModule::GetResourceManager | `resource::IResourceManager* GetResourceManager() const;` | te/texture/TextureModule.h |

### Asset Description and Serialization

| Module | Namespace | Symbol | Description | Header |
|--------|-----------|--------|-------------|--------|
| 028-Texture | te::texture | TextureAssetDesc | .texture asset description; owned by 028 and registered with 002; fields: formatVersion, debugDescription, format, width, height, depth, mipLevels, pixelData, pixelDataSize, isHDR, IsValid() | te/texture/TextureAssetDesc.h |
| 028-Texture | te::texture | SerializeTextureToBuffer | `bool SerializeTextureToBuffer(TextureHandle h, void* buffer, size_t* size);` Writes pixel data to buffer; returns true on success | te/texture/TextureSerialize.h |
| 028-Texture | te::texture | GetTexturePixelDataSize | `size_t GetTexturePixelDataSize(TextureHandle h);` Bytes required for serialization | te/texture/TextureSerialize.h |

### Resource Implementation and Module Init

| Module | Namespace | Symbol | Description | Header |
|--------|-----------|--------|-------------|--------|
| 028-Texture | te::texture | TextureResource | Implements ITextureResource; Load/Save/Import; CPU-only; GetTextureHandle, GetPixelData, GetPixelDataSize, GetWidth, GetHeight, GetMipCount, GetFormat, SetResourceManager, SetTextureHandle | te/texture/TextureResource.h |
| 028-Texture | te::texture | InitializeTextureModule | `void InitializeTextureModule(resource::IResourceManager* manager);` Registers 002 types and 013 resource factory | te/texture/TextureModuleInit.h |

### Image Import (import sub-namespace)

| Module | Namespace | Symbol | Description | Header |
|--------|-----------|--------|-------------|--------|
| 028-Texture | te::texture::import | ImageFormat | Enum: Unknown, PNG, JPEG, WebP, TIFF, EXR, AVIF, HEIF, BMP, TGA, PSD, GIF, HDR | te/texture/import/ImageImporter.h |
| 028-Texture | te::texture::import | ImageImportResult | Import result; fields: pixelData, pixelDataSize, width, height, depth, channels, format, isHDR; pixelData allocated by importer, caller frees via te::core::Free | te/texture/import/ImageImporter.h |
| 028-Texture | te::texture::import | IImageImporter | Interface: `virtual bool CanImport(char const* filePath) const = 0;` `virtual bool Import(char const* filePath, ImageImportResult& result) = 0;` | te/texture/import/ImageImporter.h |
| 028-Texture | te::texture::import | ImageImporterRegistry | Singleton; RegisterImporter(importer, priority), DetectFormat(path), ImportImage(path, result), ImportImageFile(path, outData, outSize, outWidth, outHeight, outChannels) | te/texture/import/ImageImporterRegistry.h |
| 028-Texture | te::texture::import | StbImageImporter | Default registered; multi-format fallback (PNG/JPG/BMP/TGA/HDR etc.); static ImportImageFile convenience method | te/texture/import/StbImageImporter.h |
| 028-Texture | te::texture::import | LibPngImporter | Optional; registered when TENENGINE_USE_LIBPNG defined | te/texture/import/LibPngImporter.h |
| 028-Texture | te::texture::import | LibJpegTurboImporter | Optional; registered when TENENGINE_USE_JPEG_TURBO defined | te/texture/import/LibJpegTurboImporter.h |
| 028-Texture | te::texture::import | LibWebPImporter | Optional; registered when TENENGINE_USE_LIBWEBP defined | te/texture/import/LibWebPImporter.h |

## Implementation Notes

- Data and interface TODOs have been migrated to [028-texture-public-api.md](./028-texture-public-api.md); this file only contains the ABI table.
- Optional importers controlled by CMake options: `TENENGINE_USE_LIBPNG`, `TENENGINE_USE_JPEG_TURBO`, `TENENGINE_USE_LIBWEBP`.
- libpng/libjpeg-turbo via find_package; libwebp via FetchContent (target name `webp`).

## Change Log

| Date | Change |
|------|--------|
| T0 | 028-Texture ABI created |
| 2026-02-10 | Added image import ABI; IRenderTexture, TextureModule GPU methods |
| 2026-02-11 | Removed IRenderTexture; TextureModule becomes CPU-only with GPU cache |
| 2026-02-22 | Updated to match actual implementation: TextureModule is CPU-only (no GPU methods); removed IRenderTexture/RenderTextureImpl; SerializeTextureToBuffer signature corrected (buffer, size params); ImageFormat enum expanded |
