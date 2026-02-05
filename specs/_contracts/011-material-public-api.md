# 契约：011-Material 模块对外 API

## 适用模块

- **实现方**：011-Material（L2；材质定义、参数、与 Shader 绑定、材质实例；不发起加载，013 调用 CreateMaterial）
- **对应规格**：`docs/module-specs/011-material.md`
- **依赖**：009-RenderCore、010-Shader、028-Texture、013-Resource

## 消费者

- 013-Resource（CreateMaterial 时通过注册调用）、020-Pipeline

## 能力列表

### 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| MaterialHandle | 材质句柄；CreateMaterial(MaterialAssetDesc, shaderRef, textureRefs)、GetParameters、GetShaderRef、GetTextureRefs；入参由 013 传入 | 创建后直至显式释放 |
| MaterialInstanceHandle | 材质实例句柄；CreateInstance、SetOverride、Release、Pool | 创建后直至显式释放 |
| ParameterSlot | 参数槽位；SetScalar、SetTexture、SetBuffer、与 RenderCore Uniform/纹理槽对接 | 与材质或实例绑定 |
| VariantKey | 与 Shader 变体对应；BindToPSO、GetVariantKey、SubmitToPipeline | 由调用方管理 |

### 能力（提供方保证）

| 序号 | 能力 | 说明 |
|------|------|------|
| 1 | MaterialDef | CreateMaterial(MaterialAssetDesc, shaderRef, textureRefs)、GetParameters、GetDefaultValues、GetShaderRef、GetTextureRefs；入参由 013 传入 |
| 2 | Parameters | SetScalar、SetTexture、SetBuffer、GetSlotMapping；与 RenderCore Uniform/纹理槽对接 |
| 3 | Instancing | CreateInstance、SetOverride、Release、Pool |
| 4 | Binding | BindToPSO、GetVariantKey、SubmitToPipeline；与 Shader 变体、RHI PSO、Pipeline 对接 |

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。

## 约束

- 须在 RenderCore、Shader、Texture、Resource 初始化之后使用。013 Load(materialGuid) 后调用 011 CreateMaterial(MaterialAssetDesc, shaderRef, textureRefs)；011 仅接受内存中的描述与引用。材质实例释放顺序须与 Pipeline DrawCall 生命周期协调。

## TODO 列表

（以下任务来自 `docs/asset/` 资源管理/加载/存储设计。）

- [ ] **描述归属**：MaterialAssetDesc 归属 011；.material 描述格式与 002 注册；一目录一资源（.material + 贴图用 GUID 引用）。
- [ ] **CreateMaterial**：CreateMaterial(MaterialAssetDesc, shaderRef, textureRefs)；依赖由 013 递归加载后传入；EnsureDeviceResources 时先对依赖链 Ensure 再创建材质 DResource（绑定贴图、采样器等），调用 008。
- [ ] **接口与数据**：MaterialAssetDesc 类型与 002 注册；MaterialHandle 内持 IUniformBuffer* 与贴图绑定；CreateMaterial(desc, shaderHandle, textureHandles)、UpdateMaterialParams(handle, scalarParams)、EnsureDeviceResources(handle, device)、IsDeviceReady(handle)；调用 009 CreateUniformBuffer(layout, device)、IUniformBuffer::Update，layout 来自 010.GetReflection。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 011-Material 契约 |
| 2026-02-05 | 统一目录；能力列表用表格 |
