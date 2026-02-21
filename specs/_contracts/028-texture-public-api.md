# Contract: 028-Texture Module Public API

## Applicable Module

- **Implementor**: 028-Texture (L2; texture data, format, mipmap; CPU-only module; GPU resources created via 030-DeviceResourceManager)
- **Spec**: `docs/module-specs/028-texture.md`
- **Dependencies**: 001-Core, 002-Object, 009-RenderCore, 013-Resource

## Consumers

- 011-Material, 013-Resource (Load(Texture) calls 028 CreateTexture), 020-Pipeline, 021-Effects, 022-2D, 023-Terrain, 024-Editor

## Capabilities

### Types and Handles (Cross-Boundary)

| Name | Semantics | Lifecycle |
|------|-----------|-----------|
| TextureHandle | Texture handle; CreateTexture(desc) accepts memory only; CPU data held by detail::TextureData | Created until explicitly released |
| TextureDesc | Format, dimensions, mip levels (from 009-RenderCore) | Bound to Texture |
| TextureAssetDesc | .texture asset description; owned by 028 and registered with 002; 013 deserializes .texture and passes to 028 CreateTexture or assembles TextureResource | Bound to .texture resource |

### Capabilities (Provider Guarantees)

| ID | Capability | Description |
|----|------------|-------------|
| 1 | CreateTexture | Accepts pixels/description (TextureAssetDesc), creates logical TextureHandle; input from 013 |
| 2 | TextureModule | Global singleton; CPU-only; holds IResourceManager reference via SetResourceManager/GetResourceManager |
| 3 | Format/Size Query | GetFormat, GetWidth, GetHeight, GetMipCount, GetPixelData, GetPixelDataSize |
| 4 | ReleaseTexture | Releases TextureHandle and associated memory |
| 5 | Image Import | ImageImporterRegistry singleton; DetectFormat, ImportImage; optional StbImageImporter (fallback), LibPngImporter, LibJpegTurboImporter, LibWebPImporter (enabled via CMake options) |
| 6 | Serialization | SerializeTextureToBuffer writes pixel data to buffer; GetTexturePixelDataSize returns required bytes |

## Version / ABI

- Follows Constitution: Public API versioning; breaking changes increment MAJOR.

## Constraints

- Must be used after Core, Object, RenderCore, Resource initialization.
- 013 parses pixels/description and calls 028 CreateTexture.
- GPU resources are created by 030-DeviceResourceManager, not by this module.

## Third-Party Dependencies (Optional)

| ID | Purpose | CMake Option | Notes |
|----|---------|--------------|-------|
| stb | Image loading fallback (PNG/JPG/BMP/TGA/HDR etc.) | Default enabled (FetchContent) | StbImageImporter, priority 0 |
| libpng | PNG import | TENENGINE_USE_LIBPNG | find_package(PNG); LibPngImporter priority 10 |
| libjpeg-turbo | JPEG import | TENENGINE_USE_JPEG_TURBO | find_package(JPEG); LibJpegTurboImporter priority 10 |
| libwebp | WebP import | TENENGINE_USE_LIBWEBP | FetchContent, target webp; LibWebPImporter priority 10 |

See `docs/third_party/README.md` and individual library docs.

## Design Notes

### CPU-Only Module

- 028-Texture is a CPU-only module that manages texture data in system memory.
- GPU texture creation is handled by 030-DeviceResourceManager.
- TextureModule provides SetResourceManager/GetResourceManager for integration with 013-Resource.
- No IRenderTexture interface or GPU texture cache in this module.

## Change Log

| Date | Change |
|------|--------|
| T0 | 028-Texture contract created |
| 2026-02-05 | Unified directory; capabilities in table format |
| 2026-02-05 | TextureAssetDesc ownership transferred to 028-Texture (from 013-Resource) |
| 2026-02-10 | Added dependency on 030-DeviceResourceManager; added image import and optional third-party (stb/libpng/libjpeg-turbo/libwebp); ABI in 028-texture-ABI.md |
| 2026-02-11 | Redesign: CPU-only module; TextureModule holds IResourceManager reference; removed IRenderTexture and GPU cache |
| 2026-02-22 | Updated to match actual implementation: TextureModule is CPU-only (no GetOrCreate/GetCached/RegisterPendingTexture/FlushPendingResources/ReleaseTexture); SerializeTextureToBuffer signature updated (buffer + size params); ImageFormat enum expanded (TIFF/EXR/AVIF/HEIF/BMP/TGA/PSD/GIF/HDR) |
