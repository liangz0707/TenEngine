# 契约：010-Shader 模块对外 API

## 适用模块

- **实现方**：010-Shader（L2；Shader 编译、变体、预编译、可选 Shader Graph；作为资产由 013 加载，010 不发起加载）
- **对应规格**：`docs/module-specs/010-shader.md`
- **依赖**：001-Core、008-RHI、009-RenderCore、013-Resource

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
| 1 | Source & Compilation | HLSL、GLSL；Compile、GetBytecode、TargetBackend、ErrorReport；多后端编译与错误报告 |
| 2 | Macros & Variants | DefineKeyword、SetMacros、GetVariantKey、EnumerateVariants、Precompile；游戏中可动态切换宏 |
| 3 | Cache | LoadCache、SaveCache、Invalidate；预编译缓存、与 Resource 集成（可选） |
| 4 | Hot Reload（可选） | ReloadShader、OnSourceChanged、NotifyShaderUpdated；源码或宏变更后重新编译并通知下游 |
| 5 | Graph（可选） | NodeGraph、ExportSource/IR；与 Material 联动 |

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。

## 约束

- 须在 Core、RHI、RenderCore、Resource 初始化之后使用。013 Load(shaderGuid) 后将 ShaderAssetDesc 或源码交 010 CreateShader/Compile；010 不读文件、不调用 013 Load。

## TODO 列表

（以下任务来自 `docs/asset/` 资源管理/加载/存储设计。）

- [ ] **描述归属**：ShaderAssetDesc 归属 010；.shader 描述格式与 002 注册；一目录一资源（.shader + 可选 .hlsl/.glsl）。
- [ ] **CreateShader**：013 加载后交 010 CreateShader/Compile；产出 Bytecode 供 008 创建 ShaderModule/PSO；010 不发起加载。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 010-Shader 契约 |
| 2026-02-05 | 统一目录；能力列表用表格；去除 ABI 引用 |
