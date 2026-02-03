# Feature Specification: 010-Shader Full Module Implementation

**Feature Branch**: `010-shader-fullmodule-001`  
**Created**: 2026-02-03  
**Status**: Draft  
**Input**: 本 feature 的完整模块规约见 `docs/module-specs/010-shader.md`，契约见 `specs/_contracts/010-shader-public-api.md`；**本 feature 实现完整模块内容**。

## Clarifications

### Session 2026-02-03

- Q: 首版实现范围是否包含 Hot Reload 与 Shader Graph？ → A: 仅实现 Hot Reload；Shader Graph 延后。
- Q: LoadSource 收到非存在路径时如何行为？ → A: 返回 nullptr，并通过 GetLastError 提供错误信息。
- Q: Cache 并发 LoadCache/SaveCache 如何行为？ → A: 不保证线程安全；调用方负责串行化；首版单线程调用。
- Q: Precompile 收到空或无效 VariantKey 列表时如何行为？ → A: 返回 false，不修改内部状态；GetLastError 可提供错误信息。
- Q: 不支持的 shader 格式或目标后端不匹配时如何行为？ → A: Compile 返回 false；GetLastError 提供错误信息；不产生字节码。

## 规约与契约引用 *(模块切片时必填)*

- **完整模块规约**：`docs/module-specs/010-shader.md`（着色器编译、变体与预编译；HLSL/GLSL 加载、多后端 SPIR-V/DXIL/MSL 产出、宏与变体、缓存、热重载，可选 Shader Graph）。
- **本模块范围**（本 feature 实现完整模块内容）：
  1. **Source & Compilation**：LoadSource(HLSL/GLSL)、Compile、GetBytecode、TargetBackend、ErrorReport；多后端编译与错误报告。
  2. **Macros & Variants**：DefineKeyword、SetMacros、GetVariantKey、EnumerateVariants、Precompile；游戏中 SetMacros/SelectVariant 动态切换宏。
  3. **Cache**：LoadCache、SaveCache、Invalidate；预编译缓存、与资源管线集成（可选）。
  4. **Hot Reload**：ReloadShader、OnSourceChanged、NotifyShaderUpdated；源码或宏变更后实时更新 Shader（首版实现）。
  5. **Graph（延后）**：NodeGraph、ExportSource/IR；与 Material 联动；不在本 feature 首版范围内。

实现时只使用**本 feature 依赖的上游契约**（`specs/_contracts/001-core-public-api.md`、`specs/_contracts/008-rhi-public-api.md`、`specs/_contracts/009-rendercore-public-api.md`）中已声明的类型与 API；不实现本规约未列出的能力。

- **ABI 与构建规约**：本模块须实现其 **ABI 文件**（`specs/_contracts/010-shader-ABI.md`）中的**全部**符号与能力；构建须通过**引入真实子模块源码**（CMake `add_subdirectory`）满足依赖，**禁止**使用 stub、mock 或与契约不一致的代替实现作为长期方案。契约更新：接口变更须在 ABI 文件中增补或替换对应条目；下游所需接口须在**上游模块的 ABI 文件**中以 **TODO** 登记。详见 `specs/_contracts/README.md`。

- **第三方依赖**：见 `specs/_contracts/010-shader-public-api.md`「第三方依赖」小节（glslang、spirv-cross、vulkan-headers、dxc、spirv-tools）；Plan 从 public-api 读取并填入「第三方依赖」表，Task 生成版本选择、自动下载、配置、安装、编译测试、部署、配置实现等任务。详见 `docs/third_party-integration-workflow.md`。

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Load and Compile Shader Source (Priority: P1)

Users load HLSL or GLSL shader source from file or memory and compile to backend bytecode (SPIR-V/DXIL/MSL) for use in PSO creation.

**Why this priority**: Core capability without which no other feature works.

**Independent Test**: Load a vertex shader file, compile to target backend bytecode, retrieve bytecode; verify bytecode length and validity for target backend.

**Acceptance Scenarios**:

1. **Given** a valid HLSL file, **When** LoadSource(path, ShaderSourceFormat::HLSL) and Compile are called, **Then** GetBytecode returns non-null bytecode for the selected backend.
2. **Given** a valid GLSL file, **When** LoadSource(path, ShaderSourceFormat::GLSL) and Compile are called, **Then** GetBytecode returns SPIR-V bytecode.
3. **Given** invalid shader source, **When** Compile is called, **Then** GetLastError returns a non-empty error message.

---

### User Story 2 - Macro Variants and Precompile (Priority: P2)

Users define macros, enumerate variants, and precompile shader variants for faster runtime selection.

**Why this priority**: Essential for quality and performance in games with multiple quality levels or platform-specific paths.

**Independent Test**: DefineKeyword("QUALITY", "HIGH"), SetMacros on handle, EnumerateVariants, Precompile a subset of variant keys; verify bytecode availability at runtime.

**Acceptance Scenarios**:

1. **Given** a shader with `#ifdef QUALITY`, **When** DefineKeyword("QUALITY","HIGH") and SetMacros are applied, **Then** GetVariantKey returns a key representing the macro set.
2. **Given** a shader handle, **When** EnumerateVariants is called, **Then** all valid variant keys are reported.
3. **Given** a list of VariantKeys, **When** Precompile is called, **Then** subsequent SelectVariant + GetBytecode returns cached bytecode without recompilation.

---

### User Story 3 - Cache Load and Save (Priority: P3)

Users persist compiled shader bytecode to disk and load from cache to avoid recompilation on subsequent runs.

**Why this priority**: Improves iteration speed and startup time.

**Independent Test**: Compile shader, SaveCache to path, Invalidate handle, LoadCache from path; verify bytecode is restored.

**Acceptance Scenarios**:

1. **Given** compiled shader bytecode, **When** SaveCache(path) is called, **Then** cache file is written to disk.
2. **Given** a valid cache path, **When** LoadCache(path) is called, **Then** previously saved bytecode can be retrieved.
3. **Given** modified shader source, **When** Invalidate(handle) is called, **Then** cached entry for that handle is invalidated and next Compile produces fresh bytecode.

---

### User Story 4 - Hot Reload (Optional, Priority: P4)

Users change shader source or macros at runtime and see updates without restarting the application.

**Why this priority**: Improves developer workflow; optional per module spec.

**Independent Test**: Load shader, start render loop, modify source file, trigger OnSourceChanged; verify ReloadShader succeeds and NotifyShaderUpdated notifies downstream.

**Acceptance Scenarios**:

1. **Given** a loaded shader, **When** source file is modified and ReloadShader(handle) is called, **Then** handle is recompiled and bytecode is updated.
2. **Given** OnSourceChanged callback registered, **When** source changes, **Then** callback is invoked.
3. **Given** shader updated via hot reload, **When** NotifyShaderUpdated(handle) is called, **Then** downstream (Material/Pipeline) receives update notification.

---

### Edge Cases

- **LoadSource 非存在路径**：返回 nullptr，并通过 GetLastError 提供错误信息；下游可通过 `if (!handle)` 检测失败。
- **不支持的格式或后端不匹配**：Compile 返回 false；GetLastError 提供错误信息；不产生字节码。
- **Precompile 空或无效 VariantKey 列表**：返回 false，不修改内部状态；GetLastError 可提供错误信息。
- **Cache 并发访问**：不保证线程安全；调用方负责串行化；首版单线程调用 LoadCache/SaveCache/Invalidate。
- What happens when Hot Reload is triggered while a frame is being rendered?

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: System MUST support loading HLSL and GLSL shader source from file path or memory. When path does not exist, return nullptr and set error message via GetLastError.
- **FR-002**: System MUST compile shader source to SPIR-V, DXIL, or MSL bytecode based on target backend. For unsupported format or target backend mismatch, Compile returns false; GetLastError provides error message; no bytecode is produced.
- **FR-003**: System MUST expose GetBytecode for retrieving compiled bytecode for PSO creation.
- **FR-004**: System MUST support DefineKeyword, SetMacros, GetVariantKey for macro-driven variant selection.
- **FR-005**: System MUST support EnumerateVariants and Precompile for offline or on-demand variant compilation. Precompile with empty or invalid VariantKey list returns false without modifying internal state; GetLastError may provide details.
- **FR-006**: System MUST support SelectVariant for runtime dynamic macro switching.
- **FR-007**: System MUST support LoadCache, SaveCache, and Invalidate for bytecode persistence. Thread safety not required; caller must serialize access (first release: single-threaded usage).
- **FR-008**: System MUST report compilation errors via GetLastError.
- **FR-009**: System MUST return target backend type (SPIR-V/DXIL/MSL) via GetTargetBackend.
- **FR-010**: System MUST support Hot Reload (ReloadShader, OnSourceChanged, NotifyShaderUpdated) in first release.
- **FR-011**: System SHALL NOT implement Shader Graph in first release; Shader Graph (NodeGraph, ExportSource/IR) is deferred to a future feature.

### Key Entities

- **ShaderHandle**: Opaque handle representing loaded shader; used for PSO creation and binding.
- **VariantKey / MacroSet**: Key representing macro combination; used for variant enumeration and selection.
- **Bytecode**: Compiled output (SPIR-V/DXIL/MSL); passed to RHI for PSO/ShaderModule creation.
- **CompileOptions**: Compilation parameters (backend, optimization level, etc.).

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: Users can load and compile a simple vertex shader in under 500ms on typical development hardware.
- **SC-002**: Precompiled variants can be selected at runtime without blocking the render thread.
- **SC-003**: Cache load restores bytecode without recompilation for unchanged shaders.
- **SC-004**: All ABI symbols in `specs/_contracts/010-shader-ABI.md` are implemented and pass linkage.

## Interface Contracts *(multi-agent sync)*

- **本模块实现的契约**: `specs/_contracts/010-shader-public-api.md`
- **本模块依赖的契约**: 见下方 Dependencies
- **ABI/构建**：须实现 ABI 中全部符号；构建须引入真实子模块代码（te_core, te_rhi, te_rendercore），禁止长期使用 stub。接口变更须在 ABI 文件中更新。

## Dependencies

- **001-Core**: `specs/_contracts/001-core-public-api.md`（文件 I/O、字符串、平台抽象）
- **008-RHI**: `specs/_contracts/008-rhi-public-api.md`（后端类型、字节码提交、PSO 创建）
- **009-RenderCore**: `specs/_contracts/009-rendercore-public-api.md`（Uniform 布局约定、资源描述）
