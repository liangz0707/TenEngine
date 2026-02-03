# Data Model: 010-Shader

## Entities

### ShaderHandle (IShaderHandle)

- **Role**: Opaque handle representing a loaded shader module (source + metadata).
- **Lifecycle**: Created by IShaderCompiler::LoadSource / LoadSourceFromMemory; released by IShaderCompiler::ReleaseHandle.
- **State**: Holds source path or memory ref, macro set, compiled bytecode (per variant), variant cache.

### VariantKey

- **Role**: Unique identifier for a shader variant (macro combination).
- **Representation**: Hash (e.g., uint64_t) derived from MacroSet; or ordered (name,value) pairs for enumeration.
- **Uniqueness**: Same MacroSet → same VariantKey.

### MacroSet

- **Role**: Set of macro name-value pairs (e.g., QUALITY=HIGH, PLATFORM_PC=1).
- **Representation**: Key-value container; order may affect VariantKey for enumeration.
- **Validation**: Names non-empty; values arbitrary string.

### CompileOptions

- **Role**: Compilation parameters.
- **Fields**: BackendType targetBackend; uint32_t optimizationLevel; bool generateDebugInfo; (extensible).
- **Defaults**: Backend from RHI selection; optimizationLevel=1; generateDebugInfo=off in Release.

### Bytecode

- **Role**: Compiled shader binary (SPIR-V, DXIL, or MSL).
- **Ownership**: Owned by IShaderHandle variant cache; valid until handle released or variant invalidated.
- **Format**: Opaque byte blob; format determined by BackendType.

### Cache Entry

- **Role**: Persistent storage for compiled bytecode.
- **Structure**: (sourcePath, VariantKey) → (bytecode, timestamp).
- **Invalidation**: By source path (all variants) or by handle (single handle's variants).

## Relationships

```
IShaderCompiler 1──* IShaderHandle
IShaderHandle *──* (VariantKey → Bytecode)
IShaderCache 1──* CacheEntry
IShaderHotReload ──depends on── IShaderCompiler, IShaderCache
```

## State Transitions

### IShaderHandle

- **Loaded**: After LoadSource; no bytecode yet.
- **Compiled**: After Compile; bytecode for current variant.
- **Precompiled**: After Precompile; bytecode for specified variants.
- **Invalidated**: After Invalidate or source change; must recompile.

### Cache

- **Empty**: Before LoadCache or first SaveCache.
- **Loaded**: After LoadCache; entries available for lookup.
- **Dirty**: After Invalidate; entries marked for rebuild.
