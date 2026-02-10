# 契约：010-Shader 模块对外 API

## 适用模块

- **实现方**：010-Shader（L2；Shader 编译、变体、预编译、可选 Shader Graph；作为资产由 013 加载，010 不发起加载）
- **对应规格**：`docs/module-specs/010-shader.md`
- **依赖**：001-Core、008-RHI、009-RenderCore、013-Resource、002-Object

## 消费者

- 011-Material、020-Pipeline、021-Effects（013 在 Load(Shader) 时调用 010 CreateShader/Compile）

## 第三方依赖

- glslang（GLSL/HLSL→SPIR-V）、spirv-cross（SPIR-V→MSL/HLSL）、vulkan-headers、dxc（HLSL→DXIL）、spirv-tools。按后端与平台选择；详见 `docs/third_party/`。

## 能力列表

### 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| ShaderHandle | 着色器或 Shader 模块句柄；CreateShader/Compile（入参由 013 传入 ShaderAssetDesc 或源码，仅内存）产出；用于 PSO 创建与绑定 | 创建后直至显式释放 |
| ShaderSourceFormat | 源码格式枚举；HLSL、GLSL | 编译时 |
| VariantKey / MacroSet | 变体键（宏组合）；变体枚举与预编译；运行时可动态切换 | 由调用方管理 |
| Bytecode | 编译产物（SPIR-V/DXIL/MSL）；提交给 RHI 创建 PSO/ShaderModule | 由调用方或缓存管理 |
| Reflection（可选） | Uniform 布局、槽位、与 RenderCore 对接 | 与 Shader 或缓存绑定 |

### 能力（提供方保证）

| 序号 | 能力 | 说明 |
|------|------|------|
| 1 | Source & Compilation | HLSL、GLSL；LoadSource/LoadSourceFromMemory、Compile(handle, options)、GetBytecode、GetTargetBackend、GetLastError；四后端 SPIRV/DXIL/MSL/HLSL_SOURCE；CompileOptions 含 targetBackend、optimizationLevel、generateDebugInfo、stage、entryPoint |
| 2 | Macros & Variants | DefineKeyword、SetMacros、GetVariantKey、SelectVariant、EnumerateVariants、Precompile；游戏中可动态切换宏 |
| 3 | Cache | SetCache、LoadCache、SaveCache、Invalidate；预编译缓存、与 Resource 集成（可选） |
| 4 | Hot Reload（可选） | CreateShaderHotReload、ReloadShader、OnSourceChanged、NotifyShaderUpdated；源码或宏变更后重新编译并通知下游 |
| 5 | Graph（可选） | NodeGraph、ExportSource/IR；与 Material 联动 |
| 6 | Reflection | GetReflection(handle, outDesc)→UniformLayoutDesc；GetShaderReflection(handle, outDesc)→ShaderReflectionDesc（Uniform+Texture+Sampler）；需 TE_SHADER_USE_CORE 且链接 te_rendercore |
| 7 | Vertex Input 反射 | GetVertexInputReflection(handle, outDesc)；outDesc 为 te::rendercore::VertexFormatDesc*；从 SPIR-V vertex stage 的 stage_inputs 解析 location/format/offset/stride；PSO 创建时与 Mesh vertexLayout 比对 |

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。

## 约束

- 须在 Core、RHI、RenderCore、Resource 初始化之后使用。013 Load(shaderGuid) 后将 ShaderAssetDesc 或源码交 010 CreateShader/Compile；010 不读文件、不调用 013 Load。

### Shader 资源（013 统一加载）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| **ShaderAssetDesc** | Shader 资产描述；guid (ResourceId)、sourceFileName[256]、sourceFormat、compileOptions；与 002 注册；一目录一 Shader（.shader + 同目录源码文件） | 010 拥有，002 序列化 |
| **ShaderResource** | 实现 resource::IShaderResource；Load（LoadAssetDesc + 同目录读源码 + LoadSourceFromMemory + Compile）、Save（SaveAssetDesc + 写同目录源码）、Import（读源码、编译、生成 GUID、写 .shader 与源码）；GetShaderHandle() 返回 IShaderHandle* | 013 缓存持有 |
| **InitializeShaderModule(manager)** | 注册 Shader 资源工厂与 ShaderAssetDesc/CompileOptions（002）；ResourceManager 就绪后调用 | 引擎初始化时调用一次 |
| **LoadAllShaders(manager, manifestPath)** | 按清单文件逐行 LoadSync(.shader)；任一行失败即中止返回 false | 引擎初始化时调用（可选） |

## TODO 列表

（以下任务来自 `docs/asset/` 资源管理/加载/存储设计。）

- [x] **描述归属**：ShaderAssetDesc 归属 010；.shader 描述格式与 002 注册；一目录一资源（.shader + 同目录 .hlsl/.glsl）。
- [x] **Shader 资源**：010 实现 ShaderResource（IShaderResource）；013 初始化时调用 InitializeShaderModule、LoadAllShaders；Material 经 GUID 从 013 缓存取 Shader。
- [x] **接口与数据**（已实现）：LoadSource(path, format)、LoadSourceFromMemory、Compile(handle, options)、GetBytecode、GetReflection(handle, outDesc→UniformLayoutDesc)、GetShaderReflection(handle, outDesc→ShaderReflectionDesc)、GetVertexInputReflection(handle, outDesc→VertexFormatDesc)；与 009 UniformLayoutDesc/ShaderReflectionDesc/VertexFormatDesc 对接；LoadCache/SaveCache 使用 001 FileRead/FileWrite（TE_SHADER_USE_CORE 时）。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 010-Shader 契约 |
| 2026-02-05 | 统一目录；能力列表用表格；去除 ABI 引用 |
| 2026-02-10 | 能力 1–4、6–7 与实现对齐；TODO 接口与数据标为已实现 |
| 2026-02-10 | 增加 Shader 资源：ShaderAssetDesc、ShaderResource、InitializeShaderModule、LoadAllShaders；依赖 002-Object；TODO 描述归属与 Shader 资源标为已实现 |
