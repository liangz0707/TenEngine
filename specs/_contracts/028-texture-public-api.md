# 契约：028-Texture 模块对外 API

## 适用模块

- **实现方**：028-Texture（L2；贴图数据、格式、Mipmap、与 RHI 纹理资源对接；013 加载后交 028 创建，DResource 由 028 调 008 创建）
- **对应规格**：`docs/module-specs/028-texture.md`
- **依赖**：001-Core、008-RHI、009-RenderCore、013-Resource

## 消费者

- 011-Material、013-Resource（Load(Texture) 时调用 028 CreateTexture）、020-Pipeline、021-Effects、022-2D、023-Terrain、024-Editor

## 能力列表

### 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| TextureHandle | 贴图句柄；CreateTexture(pixelData, format, width, height, mipCount, …) 仅接受内存；EnsureDeviceResources 时 028 调用 008-RHI 创建 DResource（纹理） | 创建后直至显式释放 |
| TextureDesc | 格式、尺寸、Mip 级数、数组/立方体等描述 | 与 Texture 绑定 |

### 能力（提供方保证）

| 序号 | 能力 | 说明 |
|------|------|------|
| 1 | CreateTexture | 接受像素/描述，创建逻辑 TextureHandle；入参由 013 传入 |
| 2 | EnsureDeviceResources | 按需调用 008-RHI 创建 RHI 纹理（DResource） |
| 3 | 格式/尺寸查询 | GetFormat、GetWidth、GetHeight、GetMipCount 等 |
| 4 | ReleaseTexture | 释放 TextureHandle 及 GPU 纹理 |

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。

## 约束

- 须在 Core、RHI、RenderCore、Resource 初始化之后使用。013 解析得到像素/描述后调用 028 CreateTexture；DResource 在下游触发 EnsureDeviceResources 时由 028 调用 008 创建。

## TODO 列表

（以下任务来自 `docs/asset/` 资源管理/加载/存储设计。）

- [ ] **数据约定**：与 013 约定 .texture 解析后结构（formatVersion、format、width、height、mipCount、pixelData 或引用）；一目录一资源（.texture + .texdata + 可选源图）。
- [ ] **CreateTexture**：接受像素/描述创建 TextureHandle，入参仅内存、由 013 传入；EnsureDeviceResources 时调用 008 创建 RHI 纹理；ReleaseTexture 时销毁 DResource。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 028-Texture 契约 |
| 2026-02-05 | 统一目录；能力列表用表格 |
