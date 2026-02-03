# 010-Shader 模块 ABI（全量 - 实现参考）

> **用途**：本文件为 plan 产出的**全量 ABI 内容**，供 tasks 与 implement 阶段实现使用。  
> **契约写回**：plan.md「契约更新」小节仅包含相对于现有 `specs/_contracts/010-shader-ABI.md` 的新增/修改；本文件为完整参考。

---

- **契约**：`specs/_contracts/010-shader-public-api.md`
- **CMake Target**：`te_shader`
- **命名空间**：`te::shader`
- **头文件路径**：`te/shader/`
- **依赖**：te_rhi, te_rendercore, te_core

## ABI 表（全量）

### 类型与枚举（te/shader/types.hpp）

| 模块名 | 命名空间 | 符号 | 导出形式 | 接口说明 | 头文件 | 说明 |
|--------|----------|------|----------|----------|--------|------|
| 010-Shader | te::shader | ShaderSourceFormat | 枚举 | 源码格式 | te/shader/types.hpp | `enum class ShaderSourceFormat { HLSL, GLSL };` |
| 010-Shader | te::shader | MacroSet | struct | 宏名-值集合 | te/shader/types.hpp | 用于宏切换代码路径 |
| 010-Shader | te::shader | VariantKey | struct | 变体键 | te/shader/types.hpp | 关键字/宏组合 |
| 010-Shader | te::shader | CompileOptions | struct | 编译选项 | te/shader/types.hpp | targetBackend, optimizationLevel, generateDebugInfo 等 |
| 010-Shader | te::shader | BackendType | 枚举 | 目标后端类型 | te/shader/types.hpp | `enum class BackendType { SPIRV, DXIL, MSL };` |
| 010-Shader | te::shader | IVariantEnumerator | 抽象接口 | 变体枚举回调 | te/shader/types.hpp | 虚析构；用于 EnumerateVariants 输出 |
| 010-Shader | te::shader | SourceChangedCallback | 类型别名 | 源码变更回调 | te/shader/types.hpp | `using SourceChangedCallback = void (*)(char const* path, void* userData);` |

### 工厂与生命周期（te/shader/factory.hpp）【新增】

| 模块名 | 命名空间 | 符号 | 导出形式 | 接口说明 | 头文件 | 说明 |
|--------|----------|------|----------|----------|--------|------|
| 010-Shader | te::shader | CreateShaderCompiler | 自由函数 | 创建编译器 | te/shader/factory.hpp | `IShaderCompiler* CreateShaderCompiler();` 失败返回 nullptr |
| 010-Shader | te::shader | DestroyShaderCompiler | 自由函数 | 销毁编译器 | te/shader/factory.hpp | `void DestroyShaderCompiler(IShaderCompiler* c);` nullptr 为 no-op |
| 010-Shader | te::shader | CreateShaderCache | 自由函数 | 创建缓存 | te/shader/factory.hpp | `IShaderCache* CreateShaderCache();` 失败返回 nullptr |
| 010-Shader | te::shader | DestroyShaderCache | 自由函数 | 销毁缓存 | te/shader/factory.hpp | `void DestroyShaderCache(IShaderCache* c);` nullptr 为 no-op |
| 010-Shader | te::shader | CreateShaderHotReload | 自由函数 | 创建热重载 | te/shader/factory.hpp | `IShaderHotReload* CreateShaderHotReload(IShaderCompiler* compiler, IShaderCache* cache);` 失败返回 nullptr |
| 010-Shader | te::shader | DestroyShaderHotReload | 自由函数 | 销毁热重载 | te/shader/factory.hpp | `void DestroyShaderHotReload(IShaderHotReload* h);` nullptr 为 no-op |

### 编译器（te/shader/compiler.hpp）

| 模块名 | 命名空间 | 符号 | 导出形式 | 接口说明 | 头文件 | 说明 |
|--------|----------|------|----------|----------|--------|------|
| 010-Shader | te::shader | IShaderCompiler | 抽象接口 | Shader 编译器 | te/shader/compiler.hpp | 见下表成员 |
| 010-Shader | te::shader | IShaderCompiler::LoadSource | 成员函数 | 加载源码（文件） | te/shader/compiler.hpp | `IShaderHandle* LoadSource(char const* path, ShaderSourceFormat format) = 0;` 路径不存在返回 nullptr，GetLastError 提供错误 |
| 010-Shader | te::shader | IShaderCompiler::LoadSourceFromMemory | 成员函数 | 加载源码（内存）【新增】 | te/shader/compiler.hpp | `IShaderHandle* LoadSourceFromMemory(void const* data, size_t size, ShaderSourceFormat format) = 0;` 失败返回 nullptr |
| 010-Shader | te::shader | IShaderCompiler::ReleaseHandle | 成员函数 | 释放句柄【新增】 | te/shader/compiler.hpp | `void ReleaseHandle(IShaderHandle* handle) = 0;` nullptr 为 no-op |
| 010-Shader | te::shader | IShaderCompiler::Compile | 成员函数 | 编译 | te/shader/compiler.hpp | `bool Compile(IShaderHandle* handle, CompileOptions const& options) = 0;` 不支持的格式或后端不匹配返回 false，GetLastError 提供错误 |
| 010-Shader | te::shader | IShaderCompiler::GetBytecode | 成员函数 | 取字节码 | te/shader/compiler.hpp | `void const* GetBytecode(IShaderHandle* handle, size_t* out_size) = 0;` |
| 010-Shader | te::shader | IShaderCompiler::GetLastError | 成员函数 | 取编译错误 | te/shader/compiler.hpp | `char const* GetLastError() const = 0;` |
| 010-Shader | te::shader | IShaderCompiler::GetTargetBackend | 成员函数 | 取目标后端 | te/shader/compiler.hpp | `BackendType GetTargetBackend() const = 0;` |
| 010-Shader | te::shader | IShaderCompiler::DefineKeyword | 成员函数 | 定义宏 | te/shader/compiler.hpp | `void DefineKeyword(char const* name, char const* value) = 0;` |
| 010-Shader | te::shader | IShaderCompiler::EnumerateVariants | 成员函数 | 枚举变体 | te/shader/compiler.hpp | `void EnumerateVariants(IShaderHandle* handle, IVariantEnumerator* out) = 0;` |
| 010-Shader | te::shader | IShaderCompiler::Precompile | 成员函数 | 预编译 | te/shader/compiler.hpp | `bool Precompile(IShaderHandle* handle, VariantKey const* keys, size_t count) = 0;` 空或无效列表返回 false，不修改内部状态 |

### 句柄与变体（te/shader/handle.hpp）

| 模块名 | 命名空间 | 符号 | 导出形式 | 接口说明 | 头文件 | 说明 |
|--------|----------|------|----------|----------|--------|------|
| 010-Shader | te::shader | IShaderHandle | 抽象接口 | Shader 句柄 | te/shader/handle.hpp | 见下表成员 |
| 010-Shader | te::shader | IShaderHandle::SetMacros | 成员函数 | 设置宏 | te/shader/handle.hpp | `void SetMacros(MacroSet const& macros) = 0;` |
| 010-Shader | te::shader | IShaderHandle::GetVariantKey | 成员函数 | 取变体键 | te/shader/handle.hpp | `VariantKey GetVariantKey() const = 0;` |
| 010-Shader | te::shader | IShaderHandle::SelectVariant | 成员函数 | 选择变体 | te/shader/handle.hpp | `void SelectVariant(VariantKey key) = 0;` |

### 缓存（te/shader/cache.hpp）

| 模块名 | 命名空间 | 符号 | 导出形式 | 接口说明 | 头文件 | 说明 |
|--------|----------|------|----------|----------|--------|------|
| 010-Shader | te::shader | IShaderCache | 抽象接口 | Shader 缓存 | te/shader/cache.hpp | 见下表成员 |
| 010-Shader | te::shader | IShaderCache::LoadCache | 成员函数 | 加载缓存 | te/shader/cache.hpp | `bool LoadCache(char const* path) = 0;` 不保证线程安全 |
| 010-Shader | te::shader | IShaderCache::SaveCache | 成员函数 | 保存缓存 | te/shader/cache.hpp | `bool SaveCache(char const* path) = 0;` 不保证线程安全 |
| 010-Shader | te::shader | IShaderCache::Invalidate | 成员函数 | 失效 | te/shader/cache.hpp | `void Invalidate(IShaderHandle* handle) = 0;` |

### 热重载（te/shader/hot_reload.hpp）

| 模块名 | 命名空间 | 符号 | 导出形式 | 接口说明 | 头文件 | 说明 |
|--------|----------|------|----------|----------|--------|------|
| 010-Shader | te::shader | IShaderHotReload | 抽象接口 | 热重载 | te/shader/hot_reload.hpp | 见下表成员 |
| 010-Shader | te::shader | IShaderHotReload::ReloadShader | 成员函数 | 重载 Shader | te/shader/hot_reload.hpp | `bool ReloadShader(IShaderHandle* handle) = 0;` |
| 010-Shader | te::shader | IShaderHotReload::OnSourceChanged | 成员函数 | 源码变更回调 | te/shader/hot_reload.hpp | `void OnSourceChanged(char const* path, SourceChangedCallback callback, void* userData = nullptr) = 0;` |
| 010-Shader | te::shader | IShaderHotReload::NotifyShaderUpdated | 成员函数 | 通知 Shader 已更新 | te/shader/hot_reload.hpp | `void NotifyShaderUpdated(IShaderHandle* handle) = 0;` |

### 头文件与包含关系

| 头文件 | 依赖 | 说明 |
|--------|------|------|
| te/shader/types.hpp | \<cstddef\> | ShaderSourceFormat, MacroSet, VariantKey, CompileOptions, BackendType, IVariantEnumerator, SourceChangedCallback |
| te/shader/factory.hpp | te/shader/types.hpp, te/shader/compiler.hpp, te/shader/cache.hpp, te/shader/hot_reload.hpp | Create/Destroy 工厂 |
| te/shader/compiler.hpp | te/shader/types.hpp, te/shader/handle.hpp | IShaderCompiler |
| te/shader/handle.hpp | te/shader/types.hpp | IShaderHandle |
| te/shader/cache.hpp | te/shader/handle.hpp | IShaderCache |
| te/shader/hot_reload.hpp | te/shader/handle.hpp, te/shader/types.hpp | IShaderHotReload |
| te/shader/api.hpp | 以上所有 | 聚合头 |
