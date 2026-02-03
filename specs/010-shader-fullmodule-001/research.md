# Research: 010-Shader Full Module Implementation

## 1. Shader Compiler Backend Selection

**Decision**: Use glslang for GLSL/HLSL→SPIR-V, SPIRV-Cross for SPIR-V→MSL/HLSL, DXC for HLSL→DXIL.

**Rationale**: 
- **glslang**: Khronos reference compiler, mature, widely used (Vulkan SDK). Supports GLSL and HLSL to SPIR-V. Same ecosystem as Vulkan.
- **SPIRV-Cross**: De facto standard for SPIR-V cross-compilation; supports MSL (Metal), HLSL (D3D11), GLSL. Used by MoltenVK, many engines.
- **DXC**: Microsoft's official HLSL compiler for D3D12; produces DXIL. Required for D3D12 Shader Model 6+ and raytracing.

**Alternatives considered**:
- Shaderc: Wrapper over glslang+SPIRV-Tools; adds caching. Could simplify but glslang direct gives more control; Shaderc can be added later.
- ShaderConductor: Microsoft cross-compiler; less mature than glslang+SPIRV-Cross combo for multi-backend.
- Xcode metal compiler: For MSL only; SPIRV-Cross covers MSL from single SPIR-V source, keeping one intermediate format.

## 2. Compilation Pipeline Architecture

**Decision**: Single intermediate format (SPIR-V) for Vulkan path; direct HLSL→DXIL for D3D12; SPIR-V→MSL for Metal.

**Rationale**:
- **Vulkan**: GLSL/HLSL → glslang → SPIR-V (native).
- **D3D12**: HLSL → DXC → DXIL (native); GLSL → glslang → SPIR-V → SPIRV-Cross → HLSL → DXC → DXIL (fallback), or prefer HLSL source for D3D12.
- **Metal**: GLSL/HLSL → glslang → SPIR-V → SPIRV-Cross → MSL.
- Unity/Unreal use similar pipelines: single source (HLSL) with conditional compilation, or GLSL→SPIR-V→backends.

## 3. Variant Key and Macro Representation

**Decision**: VariantKey as hash or ordered tuple of (name,value) pairs; MacroSet as key-value map. Hash enables fast cache lookup.

**Rationale**:
- Unreal: FShaderMapContentKey hashes shader+target+parameters.
- Unity: ShaderKeywordSet, ShaderKeywordSpace.
- Hash-based VariantKey allows O(1) variant lookup; ordered representation needed for EnumerateVariants determinism.

## 4. Cache Format

**Decision**: Binary cache format: header (magic, version, variant count) + per-variant entries (VariantKey, bytecode size, bytecode). Optional: simple JSON manifest for debugging.

**Rationale**:
- Disk format must be versioned for forward compatibility.
- Binary for size and load speed; optional manifest for tooling.
- Invalidate: by handle/source path; cache stores path→handle mapping for invalidation.

## 5. Hot Reload Implementation

**Decision**: File system watcher (platform-specific: ReadDirectoryChangesW / inotify / FSEvents) or polling; OnSourceChanged callback invokes ReloadShader; NotifyShaderUpdated signals Material/Pipeline to re-bind.

**Rationale**:
- Unity: AssetDatabase refresh + reimport triggers recompile.
- Unreal: Hot reload via module reload; shaders recompile on file save.
- Polling simpler for MVP; file watcher for responsiveness. Callback-driven keeps Shader module decoupled from watcher implementation.

## 6. vulkan-headers Shared Dependency (008-RHI)

**Decision**: Reuse Vulkan::Headers from 008-RHI when 010-Shader is built with te_rhi. Use `if(NOT TARGET Vulkan::Headers)` before FetchContent for Vulkan-Headers; content name `Vulkan-Headers`, GIT_TAG v1.3.280 to match 008-RHI.

**Rationale**: Per `docs/third_party-integration-workflow.md` §7; avoid duplicate fetch and version mismatch.

## 7. Factory and Lifecycle

**Decision**: Module provides CreateShaderCompiler/DestroyShaderCompiler, CreateShaderCache/DestroyShaderCache, CreateShaderHotReload/DestroyShaderHotReload. IShaderCompiler::ReleaseHandle(IShaderHandle*) for handle lifecycle. No global singleton; caller manages instances.

**Rationale**: Unity/Unreal use factory patterns; explicit Create/Destroy matches te_rhi (CreateDevice/DestroyDevice) and constitution (no hidden global state).
