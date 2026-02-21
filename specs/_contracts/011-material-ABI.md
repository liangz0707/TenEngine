# 011-Material 模块 ABI

- **契约**：[011-material-public-api.md](./011-material-public-api.md)（能力与类型描述）
- **本文件**：011-Material 对外 ABI 显式表。
- **依赖**：009-RenderCore、010-Shader、013-Resource（001-Core、002-Object 经 013/010 传递）。

## ABI 表

列定义：**模块名 | 命名空间 | 符号 | 导出形式 | 接口说明 | 头文件 | 说明**

### .material JSON 与材质资源

| 模块名 | 命名空间 | 符号 | 导出形式 | 接口说明 | 头文件 | 说明 |
|--------|----------|------|----------|----------|--------|------|
| 011-Material | te::material | MaterialParamValue | 结构体 | 参数值（标量或数组） | te/material/material_json.hpp | `struct { std::vector<double> values; }` 对应 JSON number 或 number 数组 |
| 011-Material | te::material | MaterialJSONData | 结构体 | .material JSON 内存表示 | te/material/material_json.hpp | guid（可选）、shader（GUID 字符串）、textures（name→GUID）、parameters（name→MaterialParamValue） |
| 011-Material | te::material | ParseMaterialJSON | 自由函数 | 从文件解析 .material JSON | te/material/material_json.hpp | `bool ParseMaterialJSON(char const* path, MaterialJSONData& out);` UTF-8 明文，顶层 shader/textures/parameters |
| 011-Material | te::material | ParseMaterialJSONFromMemory | 自由函数 | 从内存解析 | te/material/material_json.hpp | `bool ParseMaterialJSONFromMemory(char const* data, std::size_t size, MaterialJSONData& out);` |
| 011-Material | te::material | SerializeMaterialJSON | 自由函数 | 序列化并写入文件 | te/material/material_json.hpp | `bool SerializeMaterialJSON(char const* path, MaterialJSONData const& data);` |
| 011-Material | te::material | SerializeMaterialJSONToString | 自由函数 | 序列化为 JSON 字符串 | te/material/material_json.hpp | `std::string SerializeMaterialJSONToString(MaterialJSONData const& data);` |
| 011-Material | te::material | MaterialParam | 结构体 | 单参数：type/count/data，与 UniformMemberType 及一维数组对齐 | te/material/MaterialParam.hpp | 用于 MaterialResource 参数存储；包含 IsValid()、GetElementSize()、GetTotalSize() 方法 |
| 011-Material | te::material | BlendFactor | 枚举 | 混合因子 | te/material/MaterialParam.hpp | `enum class BlendFactor : std::uint8_t { Zero, One, SrcColor, ... }` |
| 011-Material | te::material | BlendOp | 枚举 | 混合操作 | te/material/MaterialParam.hpp | `enum class BlendOp : std::uint8_t { Add, Subtract, ReverseSubtract, Min, Max }` |
| 011-Material | te::material | CompareOp | 枚举 | 深度比较操作 | te/material/MaterialParam.hpp | `enum class CompareOp : std::uint8_t { Never, Less, Equal, ... }` |
| 011-Material | te::material | CullMode | 枚举 | 背面剔除模式 | te/material/MaterialParam.hpp | `enum class CullMode : std::uint8_t { None, Front, Back, FrontAndBack }` |
| 011-Material | te::material | FrontFace | 枚举 | 正面朝向 | te/material/MaterialParam.hpp | `enum class FrontFace : std::uint8_t { CounterClockwise, Clockwise }` |
| 011-Material | te::material | BlendAttachmentDesc | 结构体 | 每个附件的混合状态 | te/material/MaterialParam.hpp | blendEnable, srcColorBlend, dstColorBlend, colorBlendOp, srcAlphaBlend, dstAlphaBlend, alphaBlendOp, colorWriteMask |
| 011-Material | te::material | DepthStencilStateDesc | 结构体 | 深度模板状态 | te/material/MaterialParam.hpp | depthTestEnable, depthWriteEnable, depthCompareOp |
| 011-Material | te::material | RasterizationStateDesc | 结构体 | 光栅化状态 | te/material/MaterialParam.hpp | cullMode, frontFace |
| 011-Material | te::material | PipelineStateDesc | 结构体 | 管线状态描述（blend, depth, cull 等）；CPU 侧仅描述 | te/material/MaterialParam.hpp | blendEnable, blendAttachments[kMaxBlendAttachments], blendAttachmentCount, depthStencil, rasterization |
| 011-Material | te::material | MaterialResource | 类 | 材质资源（仅 CPU：shader GUID、params、texture GUIDs、pipeline state） | te/material/MaterialResource.h | 实现 resource::IMaterialResource；Load/Save 填 params/textureSlots 从 JSON；GetShaderGuid、SetShaderGuid、GetPipelineStateDesc、SetPipelineStateDesc、SetParameter、GetParameter、SetTextureGuid、GetParams、GetTextureSlots；无 EnsureDeviceResources 重写；CreateMaterialResourceFromShader(shaderGuid, pipelineState) |
| 011-Material | te::material | InitializeMaterialModule | 自由函数 | 初始化 Material 模块 | te/material/MaterialModuleInit.h | `void InitializeMaterialModule(resource::IResourceManager* manager);` 向 013 注册 Material 工厂；在 LoadAllShaders 之后调用 |
| 011-Material | te::material | InitializeResourceModulesForEngine | 自由函数 | 一次性初始化 Shader+Material | te/material/MaterialModuleInit.h | `bool InitializeResourceModulesForEngine(resource::IResourceManager* manager, char const* shaderManifestPath);` 依次 InitializeShaderModule、LoadAllShaders（path 非空时）、InitializeMaterialModule |

### RenderMaterial（GPU 资源管理）

| 模块名 | 命名空间 | 符号 | 导出形式 | 接口说明 | 头文件 | 说明 |
|--------|----------|------|----------|----------|--------|------|
| 011-Material | te::material | RenderMaterial | 类 | GPU 材质实现，持 UB/DescriptorSet/PSO | te/material/RenderMaterial.hpp | 实现 rendercore::IRenderMaterial；SetDataParameter/SetDataTexture、CreateDeviceResource、UpdateDeviceResource、GetGraphicsPSO、GetDescriptorSet、GetUniformBuffer、IsDeviceReady |
| 011-Material | te::material | CreateRenderMaterial | 自由函数 | 创建 RenderMaterial | te/material/RenderMaterial.hpp | `RenderMaterial* CreateRenderMaterial(rendercore::IShaderEntry* shaderEntry, rendercore::PipelineStateDesc const& pipelineState);` |
| 011-Material | te::material | DestroyRenderMaterial | 自由函数 | 销毁 RenderMaterial | te/material/RenderMaterial.hpp | `void DestroyRenderMaterial(RenderMaterial* material);` |

### 既有类型与接口（IMaterialSystem）

| 模块名 | 命名空间 | 符号 | 导出形式 | 接口说明 | 头文件 | 说明 |
|--------|----------|------|----------|----------|--------|------|
| 011-Material | te::material | IMaterialSystem | 抽象接口 | 材质系统 | te/material/material_def.hpp | Load、GetParameters、GetDefaultValues、GetShaderRef、GetTextureRefs、GetUniformLayout、SetScalar、SetTexture、SetBuffer、GetSlotMapping、CreateInstance、ReleaseInstance、GetVariantKey、SubmitToPipeline |
| 011-Material | te::material | ParameterSlot | 结构体 | 参数槽位 | te/material/types.hpp | `struct { uint32_t set; uint32_t binding; }` |
| 011-Material | te::material | MaterialHandle | 句柄 | 材质定义句柄 | te/material/types.hpp | `struct MaterialHandle { uint64_t id; bool IsValid() const; }` |
| 011-Material | te::material | MaterialInstanceHandle | 句柄 | 材质实例句柄 | te/material/types.hpp | `struct MaterialInstanceHandle { uint64_t id; bool IsValid() const; }` |
| 011-Material | te::material | CreateMaterialSystem | 自由函数 | 创建材质系统 | te/material/factory.hpp | `IMaterialSystem* CreateMaterialSystem(te::shader::IShaderCompiler* compiler);` |
| 011-Material | te::material | DestroyMaterialSystem | 自由函数 | 销毁材质系统 | te/material/factory.hpp | `void DestroyMaterialSystem(IMaterialSystem* sys);` |

---

*来源：契约能力 MaterialDef、Parameters、Instancing、Binding；.material 明文 JSON 与 MaterialResource 实现；RenderMaterial 实现 IRenderMaterial；与 013-Resource、010-Shader、009-RenderCore 对接。*

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| 2026-02-10 | ABI 同步：MaterialResource 增加 SetDevice、GetUniformBuffer、EnsureDeviceResources 创建 IUniformLayout/IUniformBuffer、IsDeviceReady 重写 |
| 2026-02-10 | 渲染管线完备化：EnsureDeviceResources 创建 descriptorSetLayout_、descriptorSet_、defaultSampler_、graphicsPSO_（CreateGraphicsPSO(desc, layout)）；GetGraphicsPSO、GetDescriptorSet、UpdateDescriptorSetForFrame(device, frameSlot) |
| 2026-02-11 | 运行时参数更新：MaterialResource::SetParameter(name, UniformMemberType, data)；能力 6 运行时参数更新（按名称）、能力 7 模块初始化编号顺延 |
| 2026-02-11 | 运行时纹理覆盖：MaterialResource::SetRuntimeTextureByName(name, texture)；能力 7 运行时纹理覆盖、能力 8 模块初始化编号顺延 |
| 2026-02-11 | 重构：MaterialParam、PipelineStateDesc（MaterialParam.hpp）；MaterialResource 仅 CPU 数据；MaterialShadingState、MaterialRenderer；CreateMaterialRenderer、DestroyMaterialRenderer、GetOrCreateMaterialRenderer；ABI 表更新为上述符号 |
| 2026-02-22 | 同步代码：新增 RenderMaterial（替代 MaterialRenderer，实现 IRenderMaterial）；新增 BlendFactor/BlendOp/CompareOp/CullMode/FrontFace 枚举；新增 BlendAttachmentDesc/DepthStencilStateDesc/RasterizationStateDesc 结构体；新增 CreateRenderMaterial/DestroyRenderMaterial；更新 ParameterSlot 为包含 set/binding 的结构体；补充 IMaterialSystem::GetUniformLayout |
