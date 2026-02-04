# 010-Shader 模块 ABI

- **契约**：[010-shader-public-api.md](./010-shader-public-api.md)（能力与类型描述）
- **本文件**：010-Shader 对外 ABI 显式表。

## ABI 表

列定义：**模块名 | 命名空间 | 类名 | 接口说明 | 头文件 | 符号 | 说明**

| 模块名 | 命名空间 | 类名 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|--------|------|------|
| 010-Shader | te::shader | — | 枚举 | 源码格式 | te/shader/ShaderTypes.h | ShaderSourceFormat | `enum class ShaderSourceFormat { HLSL, GLSL };` 支持 HLSL/GLSL 加载与编译 |
| 010-Shader | te::shader | IShaderCompiler | 接口 | 加载与编译 | te/shader/ShaderCompiler.h | IShaderCompiler::LoadSource, Compile, GetBytecode | `IShaderHandle* LoadSource(char const* path, ShaderSourceFormat format);` `bool Compile(IShaderHandle* handle, CompileOptions const& options);` `void const* GetBytecode(IShaderHandle* handle, size_t* out_size);` format 可显式指定或按扩展名推断 |
| 010-Shader | te::shader | IShaderCompiler | 接口 | 错误与后端 | te/shader/ShaderCompiler.h | IShaderCompiler::GetLastError, GetTargetBackend | `char const* GetLastError() const;` `BackendType GetTargetBackend() const;` 编译错误报告与目标后端（SPIR-V/DXIL/MSL） |
| 010-Shader | te::shader | — | 类型 | 宏/变体键 | te/shader/ShaderTypes.h | MacroSet、VariantKey | 宏名-值集合与变体键；用于宏切换代码路径 |
| 010-Shader | te::shader | IShaderHandle | 接口 | 变体与宏 | te/shader/ShaderHandle.h | IShaderHandle::SetMacros, GetVariantKey, SelectVariant | `void SetMacros(MacroSet const& macros);` `VariantKey GetVariantKey() const;` `void SelectVariant(VariantKey key);` 游戏中动态切换宏 |
| 010-Shader | te::shader | IShaderCompiler | 接口 | 变体枚举与预编译 | te/shader/ShaderCompiler.h | IShaderCompiler::DefineKeyword, EnumerateVariants, Precompile | `void DefineKeyword(char const* name, char const* value);` `void EnumerateVariants(IShaderHandle* handle, IVariantEnumerator* out);` `bool Precompile(IShaderHandle* handle, VariantKey const* keys, size_t count);` |
| 010-Shader | te::shader | IShaderCache | 接口 | 缓存 | te/shader/ShaderCache.h | IShaderCache::LoadCache, SaveCache, Invalidate | `bool LoadCache(char const* path);` `bool SaveCache(char const* path);` `void Invalidate(IShaderHandle* handle);` 热重载时按需 Invalidate |
| 010-Shader | te::shader | IShaderHotReload（可选） | 接口 | 实时更新 | te/shader/ShaderHotReload.h | IShaderHotReload::ReloadShader, OnSourceChanged, NotifyShaderUpdated | `bool ReloadShader(IShaderHandle* handle);` `void OnSourceChanged(char const* path, SourceChangedCallback callback);` `void NotifyShaderUpdated(IShaderHandle* handle);` Shader 热重载，运行中实时生效 |
