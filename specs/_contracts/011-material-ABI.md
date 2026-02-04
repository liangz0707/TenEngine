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

（依据 [docs/assets/011-material-data-model.md](../../docs/assets/011-material-data-model.md)；本模块上游：009-RenderCore、010-Shader。）

### 注册

| 需提供 | 需调用上游 |
|--------|------------|
| [ ] 在引擎启动时完成 MaterialAssetDesc 类型注册 | 002：`RegisterType<MaterialAssetDesc>`（若 011 依赖 002；否则由 013 注册） |

### 序列化 / 反序列化

- [ ] **MaterialAssetDesc** 须符合 002 可序列化类型约定；跨资源引用仅存 **shaderGuid**、**textureGuid**
- 反序列化由 013 调用 002.Deserialize 完成；011 接收已反序列化的 desc

### 数据

- [ ] **MaterialAssetDesc**：formatVersion、debugDescription、shaderGuid、textureSlots（textureGuid）、scalarParams、variantKeywords
- [ ] **MaterialHandle**：内持 IUniformBuffer*（009）、贴图绑定等

### 需提供的对外接口

| 接口 | 说明 |
|------|------|
| [ ] `CreateMaterial(desc, shaderHandle, textureHandles) → MaterialHandle*` | 创建材质；需 shaderHandle（010）、textureHandles（调用方加载） |
| [ ] `UpdateMaterialParams(handle, scalarParams)` | 上传 scalarParams 到 Uniform 缓冲 |
| [ ] `EnsureDeviceResources(handle, device) → bool` | 按需创建 DResource；返回是否就绪 |
| [ ] `IsDeviceReady(handle) → bool` | 查询 DResource 是否可用 |

### 需调用上游

| 场景 | 调用 009 接口 |
|------|---------------|
| CreateMaterial | `CreateUniformBuffer(layout, device)`；layout 来自 010.GetReflection(shaderHandle) |
| UpdateMaterialParams | `IUniformBuffer::Update(data, size)` |
| EnsureDeviceResources | `CreateUniformBuffer`（若尚未创建） |
