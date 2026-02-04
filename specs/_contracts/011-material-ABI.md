# 011-Material 模块 ABI

- **契约**：[011-material-public-api.md](./011-material-public-api.md)（能力与类型描述）
- **本文件**：011-Material 对外 ABI 显式表。

## ABI 表

列定义：**模块名 | 命名空间 | 类名 | 接口说明 | 头文件 | 符号 | 说明**

| 模块名 | 命名空间 | 类名 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|--------|------|------|
| （待补充） | 见本模块契约 | — | — | — | — | 由本模块契约与实现填入 |

---

## 数据相关 TODO

（依据 [docs/assets/011-material-data-model.md](../../docs/assets/011-material-data-model.md)、[013-resource-data-model.md](../../docs/assets/013-resource-data-model.md) §Material 的 UniformBuffer、[resource-loading-flow.md](../../docs/assets/resource-loading-flow.md)。）

- [ ] **MaterialAssetDesc**：与 docs/assets/011-material-data-model.md 一致；002-Object 已注册类型；含 formatVersion、debugDescription、**shaderGuid**、textureSlots（textureGuid）、scalarParams、variantKeywords。
- [ ] **CreateMaterial(MaterialAssetDesc)**：013 反序列化 .material 后调用；内部按 **textureGuid** 向 013 RequestLoadAsync(ResourceId, Texture)、按 **shaderGuid** 向 013 或 010 请求 Shader，再调用 008 或 009 创建 Uniform 缓冲（DResource），组装 **MaterialHandle** 返回 013。
- [ ] **Material 的 UniformBuffer**：创建（009 UniformBuffer.Create 或 008 CreateBuffer）、设置（scalarParams 上传）、绑定（Pipeline/009 在 Draw 前绑定到 Shader slot）；详见 013-resource-data-model §Material 的 UniformBuffer。
- [ ] **EnsureDeviceResources(MaterialHandle)** 或按需阶段：支持“先有 MaterialHandle、后填 DResource”；与 013 EnsureDeviceResourcesAsync 对接时，011 创建 Uniform 缓冲并填入 MaterialHandle。
