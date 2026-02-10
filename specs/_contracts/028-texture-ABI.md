# 028-Texture 模块 ABI

- **契约**：[028-texture-public-api.md](./028-texture-public-api.md)（能力与类型描述）
- **本文件**：028-Texture 对外 ABI 显式表。

## ABI 表

列定义：**模块名 | 命名空间 | 符号/类型 | 说明 | 头文件**

### 核心句柄与工厂

| 模块名 | 命名空间 | 符号 | 说明 | 头文件 |
|--------|----------|------|------|--------|
| 028-Texture | te::texture | TextureHandle | 不透明贴图句柄；CreateTexture 返回，ReleaseTexture 释放 | te/texture/Texture.h |
| 028-Texture | te::texture | TextureDesc | 纹理描述（009-RenderCore）；格式/尺寸查询 | te/texture/Texture.h |
| 028-Texture | te::texture | GetFormat, GetWidth, GetHeight, GetMipCount | 格式/尺寸查询 | te/texture/Texture.h |
| 028-Texture | te::texture | GetPixelData, GetPixelDataSize | 像素数据只读访问 | te/texture/Texture.h |
| 028-Texture | te::texture | CreateTexture(TextureAssetDesc const*) | 从资产描述创建句柄（仅内存） | te/texture/TextureFactory.h |
| 028-Texture | te::texture | ReleaseTexture(TextureHandle) | 释放句柄及关联内存 | te/texture/TextureFactory.h |

### 设备资源（GPU 纹理）

| 模块名 | 命名空间 | 符号 | 说明 | 头文件 |
|--------|----------|------|------|--------|
| 028-Texture | te::texture | EnsureDeviceResources(TextureHandle, IDevice*) | 同步创建 GPU 纹理（经 030） | te/texture/TextureDevice.h |
| 028-Texture | te::texture | EnsureDeviceResourcesAsync(...) | 异步创建 GPU 纹理 | te/texture/TextureDevice.h |
| 028-Texture | te::texture | GetTextureHandle(TextureHandle) | 取 RHI 纹理句柄 | te/texture/TextureDevice.h |
| 028-Texture | te::texture | DestroyDeviceTexture(TextureHandle, IDevice*) | 销毁 GPU 纹理 | te/texture/TextureDevice.h |

### 资产描述与序列化

| 模块名 | 命名空间 | 符号 | 说明 | 头文件 |
|--------|----------|------|------|--------|
| 028-Texture | te::texture | TextureAssetDesc | .texture 资产描述；028 拥有并向 002 注册；013 反序列化后交 028 | te/texture/TextureAssetDesc.h |
| 028-Texture | te::texture | SerializeTextureToBuffer | 将像素数据写入缓冲区（.texdata） | te/texture/TextureSerialize.h |
| 028-Texture | te::texture | GetTexturePixelDataSize | 序列化所需字节数 | te/texture/TextureSerialize.h |

### 资源实现与模块初始化

| 模块名 | 命名空间 | 符号 | 说明 | 头文件 |
|--------|----------|------|------|--------|
| 028-Texture | te::texture | TextureResource | 实现 ITextureResource；Load/Save/Import、EnsureDeviceResources | te/texture/TextureResource.h |
| 028-Texture | te::texture | InitializeTextureModule(IResourceManager*) | 注册 002 类型与 013 资源工厂 | te/texture/TextureModuleInit.h |

### 图像导入（import 子命名空间）

| 模块名 | 命名空间 | 符号 | 说明 | 头文件 |
|--------|----------|------|------|--------|
| 028-Texture | te::texture::import | ImageFormat | 枚举：Unknown, PNG, JPEG, WebP, … | te/texture/import/ImageImporter.h |
| 028-Texture | te::texture::import | ImageImportResult | 导入结果；pixelData 由 importer 分配，调用方 te::core::Free | te/texture/import/ImageImporter.h |
| 028-Texture | te::texture::import | IImageImporter | 接口：CanImport(path), Import(path, result) | te/texture/import/ImageImporter.h |
| 028-Texture | te::texture::import | ImageImporterRegistry | 单例；RegisterImporter, DetectFormat, ImportImage | te/texture/import/ImageImporterRegistry.h |
| 028-Texture | te::texture::import | StbImageImporter | 默认注册；多格式兜底（PNG/JPG/BMP/TGA 等） | te/texture/import/StbImageImporter.h |
| 028-Texture | te::texture::import | LibPngImporter | 可选；TENENGINE_USE_LIBPNG 时注册 | te/texture/import/LibPngImporter.h |
| 028-Texture | te::texture::import | LibJpegTurboImporter | 可选；TENENGINE_USE_JPEG_TURBO 时注册 | te/texture/import/LibJpegTurboImporter.h |
| 028-Texture | te::texture::import | LibWebPImporter | 可选；TENENGINE_USE_LIBWEBP 时注册 | te/texture/import/LibWebPImporter.h |

## 实现说明

- 数据与接口 TODO 已迁移至 [028-texture-public-api.md](./028-texture-public-api.md)；本文件仅保留 ABI 表。
- 可选导入器由 CMake 选项控制：`TENENGINE_USE_LIBPNG`、`TENENGINE_USE_JPEG_TURBO`、`TENENGINE_USE_LIBWEBP`；libpng/libjpeg-turbo 通过 find_package，libwebp 通过 FetchContent（目标名 `webp`）。
