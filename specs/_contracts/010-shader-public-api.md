# Contract: 010-Shader Module Public API

## Applicable Modules

- **Implementer**: 010-Shader (L2; Shader compilation, variants, precompilation, optional Shader Graph; loaded as asset by 013, 010 does not initiate loading)
- **Corresponding Spec**: `docs/module-specs/010-shader.md`
- **Dependencies**: 001-Core, 008-RHI, 009-RenderCore, 013-Resource, 002-Object

## Consumers

- 011-Material, 020-Pipeline, 021-Effects (013 calls 010 CreateShader/Compile during Load(Shader))

## Third-Party Dependencies

- glslang (GLSL/HLSL->SPIR-V), spirv-cross (SPIR-V->MSL/HLSL), vulkan-headers, dxc (HLSL->DXIL), spirv-tools. Selected by backend and platform; see `docs/third_party/`.

## Capability List

### Types and Handles (Cross-Boundary)

| Name | Semantics | Lifetime |
|------|-----------|----------|
| IShaderHandle | Shader or Shader module handle; CreateShader/Compile produces (input from 013 via ShaderAssetDesc or source, memory only); used for PSO creation and binding | Created until explicitly released |
| ShaderSourceFormat | Source format enumeration; HLSL, GLSL | Compile-time |
| BackendType | Target backend enumeration; SPIRV, DXIL, DXBC (D3D11), MSL, HLSL_SOURCE (SPIR-V -> HLSL via SPIRV-Cross) | Compile-time |
| ShaderStage | Shader stage enumeration; Vertex, Fragment, Compute, Geometry, TessControl, TessEvaluation, Unknown | Compile-time |
| MacroSet | Macro name-value set; max 32 pairs, names/values[64] | Managed by caller |
| VariantKey | Variant key (macro combination); hash-based comparison | Managed by caller |
| CompileOptions | Compile options; targetBackend, optimizationLevel, generateDebugInfo, stage, entryPoint[64] ("main" default) | Per-compile |
| IVariantEnumerator | Variant enumeration callback interface; OnVariant(VariantKey) | Callback |
| SourceChangedCallback | Source change callback type; `void (*)(char const* path, void* userData)` | Callback |
| IShaderCompiler | Shader compiler interface; LoadSource, Compile, GetBytecode, GetBytecodeForStage, GetLastError, GetTargetBackend, DefineKeyword, EnumerateVariants, Precompile, SetCache, GetReflection, GetShaderReflection, GetVertexInputReflection | Application lifetime |
| IShaderCache | Shader cache interface; LoadCache, SaveCache, Invalidate | Application lifetime |
| IShaderHotReload | Hot reload interface; ReloadShader, OnSourceChanged, NotifyShaderUpdated | Application lifetime |
| Bytecode | Compile output (SPIR-V/DXIL/MSL/DXBC); submitted to RHI for PSO/ShaderModule creation | Managed by caller or cache |
| Reflection (optional) | Uniform layout, slots, interfaces with RenderCore | Bound to Shader or cache |

### Capabilities (Provider Guarantees)

| # | Capability | Description |
|---|------------|-------------|
| 1 | Source & Compilation | HLSL, GLSL; LoadSource/LoadSourceFromMemory, Compile(handle, options), GetBytecode, GetBytecodeForStage(handle, stage, out_size); GetTargetBackend, GetLastError; backend SPIRV/DXIL/MSL/HLSL_SOURCE/DXBC (D3D11); CompileOptions includes targetBackend, optimizationLevel, generateDebugInfo, stage, entryPoint |
| 2 | Macros & Variants | DefineKeyword, SetMacros (via IShaderHandle), GetVariantKey, SelectVariant, EnumerateVariants, Precompile; dynamic macro switching at runtime |
| 3 | Cache | SetCache (optional), LoadCache, SaveCache, Invalidate; precompiled cache, integration with Resource (optional) |
| 4 | Hot Reload (optional) | CreateShaderHotReload, ReloadShader, OnSourceChanged, NotifyShaderUpdated; recompile after source or macro change and notify downstream |
| 5 | Graph (optional) | NodeGraph, ExportSource/IR; interfaces with Material |
| 6 | Reflection | GetReflection(handle, outDesc)->UniformLayoutDesc; GetShaderReflection(handle, outDesc)->ShaderReflectionDesc (Uniform+Texture+Sampler); requires TE_SHADER_USE_CORE and link te_rendercore |
| 7 | Vertex Input Reflection | GetVertexInputReflection(handle, outDesc); outDesc is te::rendercore::VertexFormatDesc*; parses location/format/offset from SPIR-V vertex stage stage_inputs; used for PSO creation and Mesh vertex layout comparison |

## Version / ABI

- Follows Constitution: Public API versioned; breaking changes increment MAJOR.

## Constraints

- Must be used after Core, RHI, RenderCore, Resource initialization. 013 Load(shaderGuid) passes ShaderAssetDesc or source to 010 CreateShader/Compile; 010 does not read files, does not call 013 Load.

### Shader Resource (013 Unified Loading)

| Name | Semantics | Lifetime |
|------|-----------|----------|
| **ShaderAssetDesc** | Shader asset description; guid (ResourceId), sourceFileName[256], sourceFormat, compileOptions; registered with 002; one directory one Shader (.shader + source files in same directory) | 010 owns, 002 serializes |
| **ShaderResource** | Implements resource::IShaderResource; Load (LoadAssetDesc + read source from same directory + LoadSourceFromMemory + Compile), Save (SaveAssetDesc + write source), Import (read source, compile, generate GUID, write .shader and source); GetShaderHandle() returns IShaderHandle* | 013 cache holds |
| **InitializeShaderModule(manager)** | Register Shader resource factory and ShaderAssetDesc/CompileOptions (002); call after ResourceManager ready | Called once at engine init |
| **LoadAllShaders(manager, manifestPath)** | LoadSync(.shader) per line from manifest file; aborts on any line failure returning false | Called at engine init (optional) |

## Change Log

| Date | Change Description |
|------|-------------------|
| T0 Initial | 010-Shader contract |
| 2026-02-05 | Unified directory; capability list in table format; removed ABI reference |
| 2026-02-10 | Capabilities 1-4, 6-7 aligned with implementation; TODO interface and data marked as implemented |
| 2026-02-10 | Added Shader resource: ShaderAssetDesc, ShaderResource, InitializeShaderModule, LoadAllShaders; dependency 002-Object; TODO description and Shader resource marked as implemented |
| 2026-02-10 | Capability 1: GetBytecodeForStage(handle, stage, out_size); BackendType::DXBC for D3D11 |
| 2026-02-22 | Code-aligned update: clarified IShaderHandle methods (SetMacros, GetVariantKey, SelectVariant), IShaderCompiler methods (ReleaseHandle, LoadSourceFromMemory), IShaderCache methods, IShaderHotReload methods, factory functions; all symbols match te/shader/*.hpp implementation |
