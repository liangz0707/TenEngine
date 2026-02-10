# 契约：011-Material 模块对外 API

## 适用模块

- **实现方**：011-Material（L2；材质定义、参数、与 Shader 绑定、材质实例；不发起加载，013 调用 CreateMaterial）
- **对应规格**：`docs/module-specs/011-material.md`
- **依赖**：009-RenderCore、010-Shader、013-Resource（028-Texture 经 013 获取贴图引用；001-Core、002-Object 经 013/010 传递）

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
| 5 | **Material 资源（013 统一加载）** | MaterialResource 实现 resource::IMaterialResource；.material 为**明文 JSON**（UTF-8），顶层仅三键：shader（GUID 字符串）、textures（binding 名称→贴图 GUID 字符串）、parameters（uniform 名称→标量或数组）；Load 解析 JSON、按 shader GUID 从 013 GetCached 取 IShaderResource、按反射 name→(set,binding) 解析贴图并 GetCached、按反射写参数缓冲；Save 写回 JSON；**GetTextureRefs** 的 outPaths 输出贴图 **GUID 字符串**；Import 可空实现；**SetDevice(te::rhi::IDevice*)** 在 EnsureDeviceResources 前调用；**EnsureDeviceResources** 对贴图链 SetDevice+Ensure 后，根据 shader 反射创建 IUniformLayout/IUniformBuffer 并 Update；**GetUniformBuffer()** 返回 IUniformBuffer*（020 按材质 Bind(cmd, slot)）；**IsDeviceReady** 重写为 device 已设置、已执行 Ensure 且所有贴图 IsDeviceReady |
| 6 | **模块初始化** | InitializeMaterialModule(manager) 向 013 注册 Material 工厂；InitializeResourceModulesForEngine(manager, shaderManifestPath) 依次调用 InitializeShaderModule、LoadAllShaders、InitializeMaterialModule，供引擎在 ResourceManager 就绪后调用一次 |

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。

## 约束

- 须在 RenderCore、Shader、Texture、Resource 初始化之后使用。013 Load(materialGuid) 后调用 011 CreateMaterial(MaterialAssetDesc, shaderRef, textureRefs)；011 仅接受内存中的描述与引用。材质实例释放顺序须与 Pipeline DrawCall 生命周期协调。

## TODO 列表

（以下任务来自 `docs/asset/` 资源管理/加载/存储设计。）

- [x] **.material 格式**：.material 为明文 JSON（UTF-8），顶层 shader/textures/parameters；贴图按 Shader 反射名称→GUID 字符串；参数按 Uniform 名称→值；一目录一资源；**不**使用 002 注册 MaterialAssetDesc，011 自解析 JSON（material_json.hpp/cpp）。
- [ ] **CreateMaterial（IMaterialSystem）**：CreateMaterial(MaterialAssetDesc, shaderRef, textureRefs)；依赖由 013 递归加载后传入；EnsureDeviceResources 时先对依赖链 Ensure 再创建材质 DResource（绑定贴图、采样器等），调用 008。
- [x] **MaterialResource 与 GetTextureRefs**：MaterialResource 实现 IMaterialResource；Load/Save 完整、Import 空实现；GetTextureRefs(outSlots, outPaths, maxCount) 的 outPaths 填贴图 **GUID 字符串**；InitializeMaterialModule(manager)、InitializeResourceModulesForEngine(manager, shaderManifestPath) 已实现。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 011-Material 契约 |
| 2026-02-05 | 统一目录；能力列表用表格 |
| 2026-02-10 | 增加能力 5–6：.material JSON、MaterialResource、GetTextureRefs(GUID)、InitializeMaterialModule、InitializeResourceModulesForEngine；依赖明确 013-Resource；TODO .material 格式与 MaterialResource 标为已实现；能力 5 补充 SetDevice、GetUniformBuffer、EnsureDeviceResources 创建 UB、IsDeviceReady |
