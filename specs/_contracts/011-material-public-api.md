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
| ParameterSlot | 参数槽位；set, binding；与 RenderCore Uniform/纹理槽对接 | 与材质或实例绑定 |
| VariantKey | 与 Shader 变体对应；BindToPSO、GetVariantKey、SubmitToPipeline | 由调用方管理 |

### 能力（提供方保证）

| 序号 | 能力 | 说明 |
|------|------|------|
| 1 | MaterialDef | IMaterialSystem::Load、GetParameters、GetDefaultValues、GetShaderRef、GetTextureRefs、GetUniformLayout；入参由 013 传入 |
| 2 | Parameters | IMaterialSystem::SetScalar、SetTexture、SetBuffer、GetSlotMapping；与 RenderCore Uniform/纹理槽对接 |
| 3 | Instancing | IMaterialSystem::CreateInstance、ReleaseInstance |
| 4 | Binding | IMaterialSystem::GetVariantKey、SubmitToPipeline；与 Shader 变体、RHI PSO、Pipeline 对接 |
| 5 | **Material 资源（013 统一加载）** | MaterialResource 实现 resource::IMaterialResource；**仅通过 Shader GUID** 引用 ShaderCollection；.material 为明文 JSON（UTF-8），顶层 shader/textures/parameters；无 GPU 资源；**GetShaderGuid()**、**SetShaderGuid()**、**GetPipelineStateDesc()**、**SetPipelineStateDesc()**；**SetParameter(name, type, data, count)**、**GetParameter()**、**SetTextureGuid(name, guid)**；**GetParams()**、**GetTextureSlots()** 供 RenderMaterial 读取；**EnsureDeviceResources** 空实现；**IsDeviceReady** 表示 Load 成功或程序化创建有效 |
| 6 | **RenderMaterial** | **RenderMaterial** 实现 IRenderMaterial 接口，持有 GPU 资源（UB、DescriptorSet、PSO）；SetDataParameter/SetDataTexture 设置 CPU 数据；CreateDeviceResource 创建 GPU 资源；UpdateDeviceResource 每帧上传数据；GetGraphicsPSO/GetDescriptorSet 供渲染使用；CreateRenderMaterial/DestroyRenderMaterial 工厂函数 |
| 7 | **参数与管线状态** | **MaterialParam**（te/material/MaterialParam.hpp）：type、count、data，与 UniformMemberType 及一维数组对齐；**BlendFactor/BlendOp/CompareOp/CullMode/FrontFace** 枚举；**BlendAttachmentDesc/DepthStencilStateDesc/RasterizationStateDesc/PipelineStateDesc** 结构体；**CreateMaterialResourceFromShader(shaderGuid, pipelineState)** 程序化创建 MaterialResource |
| 8 | **.material JSON 格式** | MaterialJSONData 结构体；ParseMaterialJSON/ParseMaterialJSONFromMemory 解析；SerializeMaterialJSON/SerializeMaterialJSONToString 序列化；guid、shader、textures（name→GUID）、parameters（name→values） |
| 9 | **模块初始化** | InitializeMaterialModule(manager) 向 013 注册 Material 工厂；InitializeResourceModulesForEngine(manager, shaderManifestPath) 依次调用 InitializeShaderModule、LoadAllShaders、InitializeMaterialModule，供引擎在 ResourceManager 就绪后调用一次 |
| 10 | **IMaterialSystem** | IMaterialSystem 接口：Load、GetParameters、GetDefaultValues、GetShaderRef、GetTextureRefs、GetUniformLayout、SetScalar、SetTexture、SetBuffer、GetSlotMapping、CreateInstance、ReleaseInstance、GetVariantKey、SubmitToPipeline；CreateMaterialSystem/DestroyMaterialSystem 工厂函数 |

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
| 2026-02-10 | 能力 5：EnsureDeviceResources 创建 layout/descriptor set/PSO/defaultSampler；GetGraphicsPSO、GetDescriptorSet、UpdateDescriptorSetForFrame(slot)；020 使用 BindDescriptorSet 替代 ub->Bind |
| 2026-02-11 | 能力 6 运行时参数更新（按名称）SetParameter(name, type, data)；能力 7 运行时纹理覆盖 SetRuntimeTexture/SetRuntimeTextureByName；能力 8 模块初始化编号顺延 |
| 2026-02-11 | 能力 5：贴图解析改为经 028-TextureModule::GetOrCreate，descriptor 使用 IRenderTexture::GetRHITexture() |
| 2026-02-11 | 重构：MaterialResource 仅 shader GUID + params + texture GUIDs + PipelineStateDesc，无 GPU 资源；新增 MaterialRenderer（持所有 GPU 资源）、MaterialShadingState（ShaderCollection + pipeline state）；MaterialParam、PipelineStateDesc；CreateMaterialResourceFromShader、GetOrCreateMaterialRenderer；能力 5–7 重写为上述分工 |
| 2026-02-22 | 同步代码：新增能力 8（.material JSON 格式）、能力 10（IMaterialSystem）；RenderMaterial 替代 MaterialRenderer（实现 IRenderMaterial）；更新类型与句柄（ParameterSlot 添加 set/binding 字段）；补充 BlendFactor/BlendOp/CompareOp/CullMode/FrontFace 枚举；补充 BlendAttachmentDesc/DepthStencilStateDesc/RasterizationStateDesc 结构体 |
