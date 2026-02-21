# 010-Shader Module ABI

- **Contract**: [010-shader-public-api.md](./010-shader-public-api.md) (capabilities and types description)
- **This file**: 010-Shader external ABI explicit table.
- **CMake Target Name**: **`te_shader`**. Depends on upstream target: **`te_rhi`** (008-RHI), **`te_rendercore`** (009-RenderCore), **`te_core`** (001-Core), **`te_resource`** (013-Resource), **`te_object`** (002-Object).
- **Namespace**: **`te::shader`**. Implementation files use this namespace uniformly.
- **Header Path**: **`te/shader/`**.

## ABI Table

Column definitions: **Module | Namespace | Symbol/Type | Export Form | Interface Description | Header | Description**

### Types and Enums (te/shader/types.hpp)

| Module | Namespace | Symbol | Export Form | Interface Description | Header | Description |
|--------|-----------|--------|-------------|----------------------|--------|-------------|
| 010-Shader | te::shader | ShaderSourceFormat | enum | Source format | te/shader/types.hpp | `enum class ShaderSourceFormat : uint32_t { HLSL, GLSL };` Supports HLSL/GLSL loading and compilation |
| 010-Shader | te::shader | BackendType | enum | Target backend type | te/shader/types.hpp | `enum class BackendType : uint32_t { SPIRV, DXIL, DXBC, MSL, HLSL_SOURCE };` DXBC for D3D11 vertex/pixel shader bytecode; HLSL_SOURCE for SPIR-V -> HLSL via SPIRV-Cross |
| 010-Shader | te::shader | ShaderStage | enum | Shader stage | te/shader/types.hpp | `enum class ShaderStage : uint32_t { Vertex = 0, Fragment, Compute, Geometry, TessControl, TessEvaluation, Unknown };` Unknown = infer from path or default Vertex |
| 010-Shader | te::shader | MacroSet | struct | Macro name-value set | te/shader/types.hpp | `static constexpr size_t kMaxPairs = 32; char names[kMaxPairs][64]; char values[kMaxPairs][64]; uint32_t count = 0;` Used for macro switching code paths |
| 010-Shader | te::shader | VariantKey | struct | Variant key | te/shader/types.hpp | `uint64_t hash = 0; bool operator==(VariantKey const& o) const; bool operator!=(VariantKey const& o) const;` Keyword/macro combination; variant enumeration and precompilation |
| 010-Shader | te::shader | CompileOptions | struct | Compile options | te/shader/types.hpp | `BackendType targetBackend = BackendType::SPIRV; uint32_t optimizationLevel = 1; bool generateDebugInfo = false; ShaderStage stage = ShaderStage::Unknown; static constexpr size_t kMaxEntryPointLen = 64; char entryPoint[kMaxEntryPointLen] = "main";` Compile parameters and backend options |
| 010-Shader | te::shader | IVariantEnumerator | abstract interface | Variant enumeration callback | te/shader/types.hpp | `virtual ~IVariantEnumerator() = default; virtual void OnVariant(VariantKey key) = 0;` Used for EnumerateVariants output |
| 010-Shader | te::shader | SourceChangedCallback | type alias | Source change callback | te/shader/types.hpp | `using SourceChangedCallback = void (*)(char const* path, void* userData);` |

### Handle (te/shader/handle.hpp)

| Module | Namespace | Symbol | Export Form | Interface Description | Header | Description |
|--------|-----------|--------|-------------|----------------------|--------|-------------|
| 010-Shader | te::shader | IShaderHandle | abstract interface | Shader handle | te/shader/handle.hpp | See IShaderHandle members table below |
| 010-Shader | te::shader | IShaderHandle::SetMacros | member | Set macros | te/shader/handle.hpp | `void SetMacros(MacroSet const& macros) = 0;` |
| 010-Shader | te::shader | IShaderHandle::GetVariantKey | member | Get variant key | te/shader/handle.hpp | `VariantKey GetVariantKey() const = 0;` |
| 010-Shader | te::shader | IShaderHandle::SelectVariant | member | Select variant | te/shader/handle.hpp | `void SelectVariant(VariantKey key) = 0;` Dynamic macro switching in game |

### Compiler (te/shader/compiler.hpp)

| Module | Namespace | Symbol | Export Form | Interface Description | Header | Description |
|--------|-----------|--------|-------------|----------------------|--------|-------------|
| 010-Shader | te::shader | IShaderCompiler | abstract interface | Shader compiler | te/shader/compiler.hpp | See IShaderCompiler members table below |
| 010-Shader | te::shader | IShaderCompiler::LoadSource | member | Load source | te/shader/compiler.hpp | `IShaderHandle* LoadSource(char const* path, ShaderSourceFormat format) = 0;` Returns nullptr on failure |
| 010-Shader | te::shader | IShaderCompiler::LoadSourceFromMemory | member | Load from memory | te/shader/compiler.hpp | `IShaderHandle* LoadSourceFromMemory(void const* data, size_t size, ShaderSourceFormat format) = 0;` |
| 010-Shader | te::shader | IShaderCompiler::ReleaseHandle | member | Release handle | te/shader/compiler.hpp | `void ReleaseHandle(IShaderHandle* handle) = 0;` |
| 010-Shader | te::shader | IShaderCompiler::Compile | member | Compile | te/shader/compiler.hpp | `bool Compile(IShaderHandle* handle, CompileOptions const& options) = 0;` |
| 010-Shader | te::shader | IShaderCompiler::GetBytecode | member | Get bytecode | te/shader/compiler.hpp | `void const* GetBytecode(IShaderHandle* handle, size_t* out_size) = 0;` Returns SPIR-V/DXIL/MSL/DXBC |
| 010-Shader | te::shader | IShaderCompiler::GetBytecodeForStage | member | Get bytecode for stage | te/shader/compiler.hpp | `void const* GetBytecodeForStage(IShaderHandle* handle, ShaderStage stage, size_t* out_size);` Compile for given stage and return bytecode; result valid until next GetBytecodeForStage or Compile; does not change handle variant selection; default returns nullptr |
| 010-Shader | te::shader | IShaderCompiler::GetLastError | member | Get compile error | te/shader/compiler.hpp | `char const* GetLastError() const = 0;` |
| 010-Shader | te::shader | IShaderCompiler::GetTargetBackend | member | Get target backend | te/shader/compiler.hpp | `BackendType GetTargetBackend() const = 0;` SPIR-V/DXIL/MSL/DXBC |
| 010-Shader | te::shader | IShaderCompiler::DefineKeyword | member | Define macro | te/shader/compiler.hpp | `void DefineKeyword(char const* name, char const* value) = 0;` |
| 010-Shader | te::shader | IShaderCompiler::EnumerateVariants | member | Enumerate variants | te/shader/compiler.hpp | `void EnumerateVariants(IShaderHandle* handle, IVariantEnumerator* out) = 0;` |
| 010-Shader | te::shader | IShaderCompiler::Precompile | member | Precompile | te/shader/compiler.hpp | `bool Precompile(IShaderHandle* handle, VariantKey const* keys, size_t count) = 0;` |
| 010-Shader | te::shader | IShaderCompiler::SetCache | member | Set cache | te/shader/compiler.hpp | `void SetCache(IShaderCache* cache);` Optional; default no-op |
| 010-Shader | te::shader | IShaderCompiler::GetReflection | member | Get Uniform reflection | te/shader/compiler.hpp | `bool GetReflection(IShaderHandle* handle, void* outDesc);` outDesc is te::rendercore::UniformLayoutDesc*; default returns false |
| 010-Shader | te::shader | IShaderCompiler::GetShaderReflection | member | Get full reflection | te/shader/compiler.hpp | `bool GetShaderReflection(IShaderHandle* handle, void* outDesc);` outDesc is te::rendercore::ShaderReflectionDesc*; includes Uniform, Texture, Sampler; default returns false |
| 010-Shader | te::shader | IShaderCompiler::GetVertexInputReflection | member | Get vertex input reflection | te/shader/compiler.hpp | `bool GetVertexInputReflection(IShaderHandle* handle, void* outDesc);` outDesc is te::rendercore::VertexFormatDesc*; parses from SPIR-V vertex stage stage_inputs; for PSO and Mesh vertex layout comparison; default returns false |

### Cache (te/shader/cache.hpp)

| Module | Namespace | Symbol | Export Form | Interface Description | Header | Description |
|--------|-----------|--------|-------------|----------------------|--------|-------------|
| 010-Shader | te::shader | IShaderCache | abstract interface | Shader cache | te/shader/cache.hpp | See IShaderCache members table below |
| 010-Shader | te::shader | IShaderCache::LoadCache | member | Load cache | te/shader/cache.hpp | `bool LoadCache(char const* path) = 0;` |
| 010-Shader | te::shader | IShaderCache::SaveCache | member | Save cache | te/shader/cache.hpp | `bool SaveCache(char const* path) = 0;` |
| 010-Shader | te::shader | IShaderCache::Invalidate | member | Invalidate | te/shader/cache.hpp | `void Invalidate(IShaderHandle* handle) = 0;` Invalidate on hot reload |

### Hot Reload (te/shader/hot_reload.hpp) (Optional)

| Module | Namespace | Symbol | Export Form | Interface Description | Header | Description |
|--------|-----------|--------|-------------|----------------------|--------|-------------|
| 010-Shader | te::shader | IShaderHotReload | abstract interface | Hot reload | te/shader/hot_reload.hpp | Optional; see IShaderHotReload members table below |
| 010-Shader | te::shader | IShaderHotReload::ReloadShader | member | Reload shader | te/shader/hot_reload.hpp | `bool ReloadShader(IShaderHandle* handle) = 0;` |
| 010-Shader | te::shader | IShaderHotReload::OnSourceChanged | member | Source change callback | te/shader/hot_reload.hpp | `void OnSourceChanged(char const* path, SourceChangedCallback callback, void* userData = nullptr) = 0;` |
| 010-Shader | te::shader | IShaderHotReload::NotifyShaderUpdated | member | Notify shader updated | te/shader/hot_reload.hpp | `void NotifyShaderUpdated(IShaderHandle* handle) = 0;` Takes effect at runtime |

### Factory (te/shader/factory.hpp)

| Module | Namespace | Symbol | Export Form | Interface Description | Header | Description |
|--------|-----------|--------|-------------|----------------------|--------|-------------|
| 010-Shader | te::shader | CreateShaderCompiler | free function | Create compiler | te/shader/factory.hpp | `IShaderCompiler* CreateShaderCompiler();` |
| 010-Shader | te::shader | DestroyShaderCompiler | free function | Destroy compiler | te/shader/factory.hpp | `void DestroyShaderCompiler(IShaderCompiler* c);` |
| 010-Shader | te::shader | CreateShaderCache | free function | Create cache | te/shader/factory.hpp | `IShaderCache* CreateShaderCache();` |
| 010-Shader | te::shader | DestroyShaderCache | free function | Destroy cache | te/shader/factory.hpp | `void DestroyShaderCache(IShaderCache* c);` |
| 010-Shader | te::shader | CreateShaderHotReload | free function | Create hot reload | te/shader/factory.hpp | `IShaderHotReload* CreateShaderHotReload(IShaderCompiler* compiler, IShaderCache* cache);` |
| 010-Shader | te::shader | DestroyShaderHotReload | free function | Destroy hot reload | te/shader/factory.hpp | `void DestroyShaderHotReload(IShaderHotReload* h);` |

### Aggregate Header (te/shader/api.hpp)

| Module | Namespace | Symbol | Export Form | Interface Description | Header | Description |
|--------|-----------|--------|-------------|----------------------|--------|-------------|
| 010-Shader | te::shader | api.hpp | aggregate header | API aggregate | te/shader/api.hpp | `#include <te/shader/cache.hpp> #include <te/shader/compiler.hpp> #include <te/shader/factory.hpp> #include <te/shader/handle.hpp> #include <te/shader/hot_reload.hpp> #include <te/shader/types.hpp>` Downstream only needs this header |

### Header Files and Include Relationships

| Header | Dependencies | Description |
|--------|--------------|-------------|
| te/shader/types.hpp | <cstddef>, <cstdint> | ShaderSourceFormat, BackendType, ShaderStage, MacroSet, VariantKey, CompileOptions, IVariantEnumerator, SourceChangedCallback |
| te/shader/handle.hpp | te/shader/types.hpp | IShaderHandle |
| te/shader/cache.hpp | te/shader/handle.hpp | IShaderCache |
| te/shader/compiler.hpp | te/shader/handle.hpp, te/shader/types.hpp, <cstddef>, IShaderCache (forward decl) | IShaderCompiler |
| te/shader/hot_reload.hpp | te/shader/handle.hpp, te/shader/types.hpp, IShaderCompiler (forward decl), IShaderCache (forward decl) | IShaderHotReload (optional) |
| te/shader/factory.hpp | te/shader/compiler.hpp, te/shader/cache.hpp, te/shader/hot_reload.hpp | Create/Destroy factories |
| te/shader/api.hpp | all above | Aggregate header; downstream only needs `#include <te/shader/api.hpp>` |

---

*Source: Contract capabilities Source & Compilation, Macros & Variants, Cache, Hot Reload, Shader resource and 013 integration; interfaces with 008-RHI, 009-RenderCore, 013-Resource, 002-Object, 011-Material, 020-Pipeline.*

## 001-Core Interface Usage

This module **must** use the following 001-Core interfaces (when linking te_core, TE_SHADER_USE_CORE=1):

| Usage | Symbol | Header |
|--------|--------|--------|
| LoadSource file read | te::core::FileRead | te/core/platform.h |
| Error logging | te::core::Log | te/core/log.h |
| Factory memory allocation | te::core::Alloc, te::core::Free | te/core/alloc.h |
| LoadCache/SaveCache | te::core::FileRead, te::core::FileWrite | te/core/platform.h |

STANDALONE build without te_core falls back to std::ifstream / new-delete.

---

Data and interface TODOs have been migrated to this module's contract [010-shader-public-api.md](./010-shader-public-api.md) TODO list; this file only retains the ABI table and implementation notes.

## Change Log

| Date | Change Description |
|------|-------------------|
| 2026-02-10 | Added BackendType::DXBC; IShaderCompiler::GetBytecodeForStage(handle, stage, out_size) for per-stage bytecode for 011 PSO creation |
| 2026-02-22 | Code-aligned update: clarified IShaderHandle methods (SetMacros, GetVariantKey, SelectVariant), IShaderCompiler methods (ReleaseHandle, LoadSourceFromMemory), IShaderCache methods (LoadCache, SaveCache, Invalidate), IShaderHotReload methods (ReloadShader, OnSourceChanged, NotifyShaderUpdated), factory functions (CreateShaderCompiler, DestroyShaderCompiler, CreateShaderCache, DestroyShaderCache, CreateShaderHotReload, DestroyShaderHotReload), aggregate header api.hpp; all symbols match te/shader/*.hpp implementation |
