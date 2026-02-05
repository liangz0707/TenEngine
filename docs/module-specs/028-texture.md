# 028-Texture 模块描述

## 1. 模块简要说明

Texture 提供**贴图数据与 GPU 纹理资源**：格式、尺寸、Mipmap、与 RHI 纹理对接，对应 Unreal 的 **Texture/Texture2D**、Unity 的 **Texture/Texture2D**。**Texture 为可加载资产类型**；013-Resource 加载并解析 .texture 后，将**像素/描述**交本模块 **CreateTexture**；DResource（RHI 纹理）在 **EnsureDeviceResources** 时由本模块调用 008-RHI 创建。依赖 Core、RHI、RenderCore。

## 2. 详细功能描述

- **来源格式**：贴图来源于图片文件（如 PNG、JPG）或引擎 .texture，经 013 导入/加载并解析后，013 将**像素数据与描述**交本模块 CreateTexture；本模块不读文件、不解析格式，仅接受内存数据。DResource 在 EnsureDeviceResources 时由本模块调用 008-RHI 创建纹理。
- **贴图数据**：格式（R8G8B8A8 等）、尺寸、Mipmap 级数、数组/立方体等。
- **与 Resource**：013 加载 .texture 后交 CreateTexture；流式/卸载通过 013 与句柄对接；Material 等通过贴图句柄引用。

## 3. 实现难度

**中**。格式与 RenderCore/RHI 需一致；Mipmap 生成与流式需清晰。

## 4. 操作的资源类型

- **输入**：013 解析 .texture 后将**像素、格式、尺寸、Mip 描述**等交本模块 CreateTexture；本模块不解析文件。DResource 在 EnsureDeviceResources 时由 028 调用 008-RHI 创建。
- **与 Resource**：028 被 013-Resource 依赖（贴图为可加载资产）；013 加载并解析 .texture 后交 CreateTexture。
- **.texture 格式归属**：**.texture 格式定义归属 028-Texture**；013 按 028 约定解析后交 028 CreateTexture。

## 5. 是否有子模块

可选（Format、Mipmap、Sampler 等可按子模块组织）。

## 6. 模块上下游

### 6.1 和上下游交互、传递的数据类型

- **上游**：Core（内存、容器）、RenderCore（纹理格式、描述）、RHI（EnsureDeviceResources 时创建纹理）。
- **下游**：011-Material（贴图引用）、013-Resource（CreateTexture）、020-Pipeline、021-Effects、022-2D、023-Terrain、024-Editor。向下游提供：TextureHandle、格式/尺寸查询、EnsureDeviceResources。

### 6.2 上下游依赖图

- 依赖关系见 `specs/_contracts/000-module-dependency-map.md`。

## 7. 依赖的外部内容

| 类别 | 内容 |
|------|------|
| **RenderCore** | 纹理格式、描述符描述 |
| **RHI** | EnsureDeviceResources 时由 028 调用 008 创建纹理（DResource） |
| **Resource** | 013 加载并解析 .texture 后交 028 CreateTexture |
| **协议** | 无 |
