# 契约：010-Shader 模块对外 API

## 适用模块

- **实现方**：**010-Shader**（着色器编译、变体与预编译，可选 Shader Graph）
- **对应规格**：`docs/module-specs/010-shader.md`
- **依赖**：001-Core（001-core-public-api）、008-RHI（008-rhi-public-api）、009-RenderCore（009-rendercore-public-api）

## 消费者（T0 下游）

- 011-Material（字节码、变体键、PSO 绑定）
- 020-Pipeline（提交 Shader 字节码、PSO 创建）
- 021-Effects（变体、预编译）

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。
- 当前契约版本：（由实现或计划阶段填写）

## 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| ShaderHandle | 着色器或 Shader 模块句柄；用于 PSO 创建与绑定 | 创建后直至显式释放 |
| ShaderSourceFormat | 源码格式枚举；HLSL、GLSL；按扩展名或显式指定 | 编译时 |
| VariantKey / MacroSet | 变体键（关键字/宏组合）；变体枚举与预编译；运行时可动态切换 | 由调用方管理 |
| Bytecode | 编译产物（SPIR-V/DXIL/MSL）；提交给 RHI 创建 PSO/ShaderModule | 由调用方或缓存管理 |
| Reflection（可选） | 反射信息；Uniform 布局、槽位、与 RenderCore 对接 | 与 Shader 或缓存绑定 |

下游仅通过上述类型与句柄访问；不直接持有 GPU 资源，字节码提交给 RHI 创建 PSO。

## 能力列表（提供方保证）

1. **Source & Compilation**：支持 **HLSL**、**GLSL** 两种源码格式加载；Compile、GetBytecode、TargetBackend、ErrorReport；多后端编译与错误报告。
2. **Macros & Variants**：支持**宏**切换代码路径；DefineKeyword、SetMacros、GetVariantKey、EnumerateVariants、Precompile；**游戏中可动态切换宏**（SetMacros/SelectVariant），按新宏组合选择或编译变体并生效。
3. **Cache**：LoadCache、SaveCache、Invalidate；预编译缓存、与 Resource 集成（可选）；热重载时按需失效。
4. **Hot Reload（可选）**：**实时更新 Shader**；ReloadShader、OnSourceChanged、NotifyShaderUpdated；源码或宏变更后重新编译并通知下游（Material/Pipeline），无需重启应用。
5. **Graph（可选）**：NodeGraph、ExportSource/IR；与 Material 联动，导出 Shader 或中间表示。

## 调用顺序与约束

- 须在 Core、RHI、RenderCore 初始化之后使用；Uniform 布局须与 RenderCore 约定一致。
- 字节码与 RHI 后端一一对应；PSO 创建由 RHI 或上层（Pipeline/Material）完成，本模块仅提供字节码与变体。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 按 010-Shader 模块规格与依赖表新增契约；类型与能力与 docs/module-specs/010-shader.md 一致 |
| 2026-01-28 | 根据 010-shader-ABI 反向更新：补充参考与命名约定、导出形式列；能力与类型与 ABI 表一致 |