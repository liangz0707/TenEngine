# 028-Texture 模块 ABI

- **契约**：[028-texture-public-api.md](./028-texture-public-api.md)（能力与类型描述）
- **本文件**：028-Texture 对外 ABI 显式表。

## ABI 表

列定义：**模块名 | 命名空间 | 类名 | 接口说明 | 头文件 | 符号 | 说明**

| 模块名 | 命名空间 | 类名 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|--------|------|------|
| （待补充） | 见本模块契约 | — | — | — | — | 由本模块契约与实现填入 |

---

## 数据相关 TODO

### 数据

- [ ] **Texture 输入格式约定**：与 013 约定 .texture 解析后的结构（formatVersion、format、width、height、mipCount、pixelData 或引用）
- [ ] **TextureHandle**：内持 RHI 纹理（DResource）、格式/尺寸信息

### 需提供的对外接口

| 接口 | 说明 |
|------|------|
| [ ] `CreateTexture(pixelData, format, width, height, mipCount, …) → TextureHandle*` | 创建 Texture；入参由 013 传入 |
| [ ] `EnsureDeviceResources(handle, device) → bool` | 按需创建 DResource；返回是否就绪 |
| [ ] `ReleaseTexture(handle)` | 释放 TextureHandle 及 GPU 纹理 |
| [ ] `GetFormat(handle)` / `GetWidth` / `GetHeight` / `GetMipCount` | 格式与尺寸查询 |

### 需调用上游

| 场景 | 调用 008 / 009 |
|------|----------------|
| CreateTexture | 使用 009 纹理格式描述；EnsureDeviceResources 时调用 008 CreateTexture |
| ReleaseTexture | 调用设备 DestroyTexture |

### 调用流程

1. 013 解析 .texture → 得到 pixelData、format、width、height、mipCount → 028.CreateTexture(..., device)
2. 013 EnsureDeviceResourcesAsync → 028.EnsureDeviceResources(handle, device)
3. 013 Unload → 028.ReleaseTexture(handle)
