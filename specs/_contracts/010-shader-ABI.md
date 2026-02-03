# 010-Shader 模块 ABI

- **契约**：[010-shader-public-api.md](./010-shader-public-api.md)（能力与类型描述）
- **本文件**：010-Shader 对外 ABI 显式表。
- **CMake Target 名称**：**`te_shader`**（与 `te_rhi`、`te_rendercore` 命名风格一致）。下游在 `target_link_libraries` 中应使用 **`te_shader`**。依赖上游 target: **`te_rhi`** (008-RHI)、**`te_rendercore`** (009-RenderCore)、**`te_core`** (001-Core)。
- **命名空间**：**`te::shader`**（与 `te::rhi`、`te::rendercore` 风格一致）。实现文件统一使用此命名空间。
- **头文件路径**：**`te/shader/`**（与 `te/rhi/`、`te/rendercore/` 风格一致）。

## ABI 表

列定义：**模块名 | 命名空间 | 符号/类型 | 导出形式 | 接口说明 | 头文件 | 说明**

### 类型与枚举（te/shader/types.hpp）

| 模块名 | 命名空间 | 符号 | 导出形式 | 接口说明 | 头文件 | 说明 |
|--------|----------|------|----------|----------|--------|------|
| 010-Shader | te::shader | ShaderSourceFormat | 枚举 | 源码格式 | te/shader/types.hpp | `enum class ShaderSourceFormat { HLSL, GLSL };` 支持 HLSL/GLSL 加载与编译 |
| 010-Shader | te::shader | MacroSet | struct | 宏名-值集合 | te/shader/types.hpp | 用于宏切换代码路径 |
| 010-Shader | te::shader | VariantKey | struct | 变体键 | te/shader/types.hpp | 关键字/宏组合；变体枚举与预编译 |
| 010-Shader | te::shader | CompileOptions | struct | 编译选项 | te/shader/types.hpp | 编译参数描述 |
| 010-Shader | te::shader | BackendType | 枚举 | 目标后端类型 | te/shader/types.hpp | `enum class BackendType { SPIRV, DXIL, MSL };` 与 RHI 后端对应 |
| 010-Shader | te::shader | IVariantEnumerator | 抽象接口 | 变体枚举回调 | te/shader/types.hpp | 虚析构；用于 EnumerateVariants 输出 |

### 编译器（te/shader/compiler.hpp）

| 模块名 | 命名空间 | 符号 | 导出形式 | 接口说明 | 头文件 | 说明 |
|--------|----------|------|----------|----------|--------|------|
| 010-Shader | te::shader | IShaderCompiler | 抽象接口 | Shader 编译器 | te/shader/compiler.hpp | 见下表 IShaderCompiler 成员 |
| 010-Shader | te::shader | IShaderCompiler::LoadSource | 成员函数 | 加载源码 | te/shader/compiler.hpp | `IShaderHandle* LoadSource(char const* path, ShaderSourceFormat format) = 0;` 失败返回 nullptr |
| 010-Shader | te::shader | IShaderCompiler::Compile | 成员函数 | 编译 | te/shader/compiler.hpp | `bool Compile(IShaderHandle* handle, CompileOptions const& options) = 0;` |
| 010-Shader | te::shader | IShaderCompiler::GetBytecode | 成员函数 | 取字节码 | te/shader/compiler.hpp | `void const* GetBytecode(IShaderHandle* handle, size_t* out_size) = 0;` 返回 SPIR-V/DXIL/MSL |
| 010-Shader | te::shader | IShaderCompiler::GetLastError | 成员函数 | 取编译错误 | te/shader/compiler.hpp | `char const* GetLastError() const = 0;` |
| 010-Shader | te::shader | IShaderCompiler::GetTargetBackend | 成员函数 | 取目标后端 | te/shader/compiler.hpp | `BackendType GetTargetBackend() const = 0;` SPIR-V/DXIL/MSL |
| 010-Shader | te::shader | IShaderCompiler::DefineKeyword | 成员函数 | 定义宏 | te/shader/compiler.hpp | `void DefineKeyword(char const* name, char const* value) = 0;` |
| 010-Shader | te::shader | IShaderCompiler::EnumerateVariants | 成员函数 | 枚举变体 | te/shader/compiler.hpp | `void EnumerateVariants(IShaderHandle* handle, IVariantEnumerator* out) = 0;` |
| 010-Shader | te::shader | IShaderCompiler::Precompile | 成员函数 | 预编译 | te/shader/compiler.hpp | `bool Precompile(IShaderHandle* handle, VariantKey const* keys, size_t count) = 0;` |

### 句柄与变体（te/shader/handle.hpp）

| 模块名 | 命名空间 | 符号 | 导出形式 | 接口说明 | 头文件 | 说明 |
|--------|----------|------|----------|----------|--------|------|
| 010-Shader | te::shader | IShaderHandle | 抽象接口 | Shader 句柄 | te/shader/handle.hpp | 见下表 IShaderHandle 成员 |
| 010-Shader | te::shader | IShaderHandle::SetMacros | 成员函数 | 设置宏 | te/shader/handle.hpp | `void SetMacros(MacroSet const& macros) = 0;` |
| 010-Shader | te::shader | IShaderHandle::GetVariantKey | 成员函数 | 取变体键 | te/shader/handle.hpp | `VariantKey GetVariantKey() const = 0;` |
| 010-Shader | te::shader | IShaderHandle::SelectVariant | 成员函数 | 选择变体 | te/shader/handle.hpp | `void SelectVariant(VariantKey key) = 0;` 游戏中动态切换宏 |

### 缓存（te/shader/cache.hpp）

| 模块名 | 命名空间 | 符号 | 导出形式 | 接口说明 | 头文件 | 说明 |
|--------|----------|------|----------|----------|--------|------|
| 010-Shader | te::shader | IShaderCache | 抽象接口 | Shader 缓存 | te/shader/cache.hpp | 见下表 IShaderCache 成员 |
| 010-Shader | te::shader | IShaderCache::LoadCache | 成员函数 | 加载缓存 | te/shader/cache.hpp | `bool LoadCache(char const* path) = 0;` |
| 010-Shader | te::shader | IShaderCache::SaveCache | 成员函数 | 保存缓存 | te/shader/cache.hpp | `bool SaveCache(char const* path) = 0;` |
| 010-Shader | te::shader | IShaderCache::Invalidate | 成员函数 | 失效 | te/shader/cache.hpp | `void Invalidate(IShaderHandle* handle) = 0;` 热重载时按需 Invalidate |

### 热重载（te/shader/hot_reload.hpp）（可选）

| 模块名 | 命名空间 | 符号 | 导出形式 | 接口说明 | 头文件 | 说明 |
|--------|----------|------|----------|----------|--------|------|
| 010-Shader | te::shader | IShaderHotReload | 抽象接口 | 热重载 | te/shader/hot_reload.hpp | 可选；见下表 IShaderHotReload 成员 |
| 010-Shader | te::shader | IShaderHotReload::ReloadShader | 成员函数 | 重载 Shader | te/shader/hot_reload.hpp | `bool ReloadShader(IShaderHandle* handle) = 0;` |
| 010-Shader | te::shader | IShaderHotReload::OnSourceChanged | 成员函数 | 源码变更回调 | te/shader/hot_reload.hpp | `void OnSourceChanged(char const* path, SourceChangedCallback callback) = 0;` |
| 010-Shader | te::shader | IShaderHotReload::NotifyShaderUpdated | 成员函数 | 通知 Shader 已更新 | te/shader/hot_reload.hpp | `void NotifyShaderUpdated(IShaderHandle* handle) = 0;` 运行中实时生效 |

### 头文件与包含关系

| 头文件 | 依赖 | 说明 |
|--------|------|------|
| te/shader/types.hpp | \<cstddef\> | ShaderSourceFormat, MacroSet, VariantKey, CompileOptions |
| te/shader/compiler.hpp | te/shader/types.hpp, te/shader/handle.hpp | IShaderCompiler |
| te/shader/handle.hpp | te/shader/types.hpp | IShaderHandle |
| te/shader/cache.hpp | te/shader/handle.hpp | IShaderCache |
| te/shader/hot_reload.hpp | te/shader/handle.hpp | IShaderHotReload（可选） |
| te/shader/api.hpp | 以上所有 | 聚合头，下游只需 `#include <te/shader/api.hpp>` |

---

*来源：契约能力 Source & Compilation、Macros & Variants、Cache、Hot Reload；与 008-RHI、009-RenderCore、011-Material、020-Pipeline 对接。*
*命名空间与头文件路径与 008-RHI (te::rhi, te/rhi/) 及 009-RenderCore (te::rendercore, te/rendercore/) 保持一致。*
