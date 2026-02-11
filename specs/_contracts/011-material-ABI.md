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
| 011-Material | te::material | MaterialResource | 类 | 材质资源实现 | te/material/MaterialResource.h | 实现 resource::IMaterialResource；Load/Save/Import 同上；**SetDevice(IDevice*)** 设置 RHI 设备；**EnsureDeviceResources** 对贴图链 Ensure 后，从 reflection 构建 DescriptorSetLayoutDesc，创建 descriptorSetLayout_、descriptorSet_、defaultSampler_，按 device 后端取 vertex/fragment bytecode（GetBytecodeForStage），**CreateGraphicsPSO(desc, descriptorSetLayout_)** 得到 graphicsPSO_；**GetGraphicsPSO()** 返回 IPSO*；**GetDescriptorSet()** 返回 IDescriptorSet*；**UpdateDescriptorSetForFrame(device, frameSlot)** 写 UB 的 ring offset 与各 texture 到 descriptorSet_；**GetUniformBuffer()** 返回 IUniformBuffer*；析构销毁 PSO、descriptor set、layout、sampler；**IsDeviceReady** 同上 |
| 011-Material | te::material | InitializeMaterialModule | 自由函数 | 初始化 Material 模块 | te/material/MaterialModuleInit.h | `void InitializeMaterialModule(resource::IResourceManager* manager);` 向 013 注册 Material 工厂；在 LoadAllShaders 之后调用 |
| 011-Material | te::material | InitializeResourceModulesForEngine | 自由函数 | 一次性初始化 Shader+Material | te/material/MaterialModuleInit.h | `bool InitializeResourceModulesForEngine(resource::IResourceManager* manager, char const* shaderManifestPath);` 依次 InitializeShaderModule、LoadAllShaders（path 非空时）、InitializeMaterialModule |

### 既有类型与接口（material_def、binding、parameters、instancing 等）

| 模块名 | 命名空间 | 符号 | 导出形式 | 接口说明 | 头文件 | 说明 |
|--------|----------|------|----------|----------|--------|------|
| 011-Material | te::material | IMaterialSystem | 抽象接口 | 材质系统 | te/material/material_def.hpp | Load、GetParameters、GetDefaultValues、GetShaderRef、GetTextureRefs、SetScalar/SetTexture/SetBuffer、CreateInstance、ReleaseInstance、GetVariantKey、BindToPSO、SubmitToPipeline |
| 011-Material | te::material | ParameterSlot | 结构体 | 参数槽位 | te/material/types.hpp | set, binding |
| 011-Material | te::material | MaterialHandle / MaterialInstanceHandle | 句柄 | 材质定义/实例句柄 | te/material/types.hpp | 与 MaterialResource / 实例对应 |

---

*来源：契约能力 MaterialDef、Parameters、Instancing、Binding；.material 明文 JSON 与 MaterialResource 实现；与 013-Resource、010-Shader、009-RenderCore 对接。*

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| 2026-02-10 | ABI 同步：MaterialResource 增加 SetDevice、GetUniformBuffer、EnsureDeviceResources 创建 IUniformLayout/IUniformBuffer、IsDeviceReady 重写 |
| 2026-02-10 | 渲染管线完备化：EnsureDeviceResources 创建 descriptorSetLayout_、descriptorSet_、defaultSampler_、graphicsPSO_（CreateGraphicsPSO(desc, layout)）；GetGraphicsPSO、GetDescriptorSet、UpdateDescriptorSetForFrame(device, frameSlot) |
