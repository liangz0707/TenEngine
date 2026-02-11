# 契约：028-Texture 模块对外 API

## 适用模块

- **实现方**：028-Texture（L2；贴图数据、格式、Mipmap、与 RHI 纹理资源对接；013 加载后交 028 创建，DResource 由 028 经 030-DeviceResourceManager 调 008 创建）
- **对应规格**：`docs/module-specs/028-texture.md`
- **依赖**：001-Core、002-Object、008-RHI、009-RenderCore、013-Resource、030-DeviceResourceManager

## 消费者

- 011-Material、013-Resource（Load(Texture) 时调用 028 CreateTexture）、020-Pipeline、021-Effects、022-2D、023-Terrain、024-Editor

## 能力列表

### 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| TextureHandle | 贴图句柄；CreateTexture(desc) 仅接受内存；CPU 数据由 detail::TextureData 持有；GPU 资源由 TextureModule 统一管理 | 创建后直至显式释放 |
| IRenderTexture | 继承 rhi::ITexture；Texture 模块内封装 GPU 贴图；GetRHITexture() 供 descriptor set、Destroy 等 RHI 调用 | 由 TextureModule 缓存持有 |
| TextureDesc | 格式、尺寸、Mip 级数（009-RenderCore） | 与 Texture 绑定 |
| TextureAssetDesc | .texture 资产描述；028 拥有并向 002 注册；013 反序列化 .texture 得到后交 028 CreateTexture 或组装 TextureResource | 与 .texture 资源绑定 |

### 能力（提供方保证）

| 序号 | 能力 | 说明 |
|------|------|------|
| 1 | CreateTexture | 接受像素/描述（TextureAssetDesc），创建逻辑 TextureHandle；入参由 013 传入 |
| 2 | TextureModule | 全局单例；维护已创建 GPU 资源的贴图缓存（IRenderTexture*）；替代/补充 013 对贴图的缓存；GetOrCreate(id, manager, device)、GetCached(id)、ReleaseTexture(id) |
| 3 | RegisterPendingTexture / FlushPendingResources | Load/Import 后调用 RegisterPendingTexture(handle, id)；EnsureDeviceResources 时通过 GetOrCreate 触发 FlushPendingResources，批量经 030 调用 008 创建 RHI 纹理并填入缓存 |
| 4 | EnsureDeviceResources | TextureResource::EnsureDeviceResources 请求 TextureModule::GetOrCreate，内部先 FlushPendingResources 再返回 IRenderTexture* |
| 5 | 格式/尺寸查询 | GetFormat、GetWidth、GetHeight、GetMipCount、GetPixelData、GetPixelDataSize |
| 6 | ReleaseTexture | 释放 TextureHandle 及关联内存；GPU 纹理由 TextureModule::ReleaseTexture(ResourceId) 释放 |
| 7 | 图像导入 | ImageImporterRegistry 单例；DetectFormat、ImportImage；可选 StbImageImporter（兜底）、LibPngImporter、LibJpegTurboImporter、LibWebPImporter（由 CMake 选项启用） |

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。

## 约束

- 须在 Core、Object、RHI、RenderCore、Resource、030-DeviceResourceManager 初始化之后使用。013 解析得到像素/描述后调用 028 CreateTexture；TextureResource Load/Import 后向 TextureModule 注册待创建；DResource 在 EnsureDeviceResources（或 GetOrCreate）时由 TextureModule::FlushPendingResources 经 030 批量创建。

## 第三方依赖（可选）

| ID | 用途 | CMake 选项 | 说明 |
|----|------|------------|------|
| stb | 图像加载兜底（PNG/JPG/BMP/TGA 等） | 默认启用（FetchContent） | StbImageImporter，priority 0 |
| libpng | PNG 导入 | TENENGINE_USE_LIBPNG | find_package(PNG)；LibPngImporter priority 10 |
| libjpeg-turbo | JPEG 导入 | TENENGINE_USE_JPEG_TURBO | find_package(JPEG)；LibJpegTurboImporter priority 10 |
| libwebp | WebP 导入 | TENENGINE_USE_LIBWEBP | FetchContent，目标 webp；LibWebPImporter priority 10 |

详见 `docs/third_party/README.md` 及各库 md。

## TODO 列表

（以下任务来自 `docs/asset/` 资源管理/加载/存储设计及原 ABI 数据相关 TODO；已实现项可标记完成。）

- [x] **数据约定**：TextureAssetDesc 归属 028；.texture + .texdata 格式；一目录一资源。
- [x] **TextureHandle**：仅持 CPU 像素与格式/尺寸；GPU 由 TextureModule（IRenderTexture）持有。
- [x] **接口**：CreateTexture(desc)→TextureHandle*；TextureModule::GetOrCreate/GetCached/RegisterPendingTexture/FlushPendingResources/ReleaseTexture；EnsureDeviceResources 经 TextureModule；GetFormat/GetWidth/GetHeight/GetMipCount；经 030 调 008。
- [x] **图像导入**：IImageImporter、ImageImporterRegistry；StbImageImporter + 可选 LibPng/LibJpegTurbo/LibWebP。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 028-Texture 契约 |
| 2026-02-05 | 统一目录；能力列表用表格 |
| 2026-02-05 | TextureAssetDesc 归属转入 028-Texture（原 013-Resource） |
| 2026-02-10 | 依赖增加 030-DeviceResourceManager；能力增加图像导入与可选第三方（stb/libpng/libjpeg-turbo/libwebp）；ABI 见 028-texture-ABI.md |
| 2026-02-11 | 重设计：IRenderTexture 继承 rhi::ITexture；TextureModule 全局缓存与待创建列表；RegisterPendingTexture、FlushPendingResources、GetOrCreate、ReleaseTexture；TextureData 不再持 deviceTexture |
