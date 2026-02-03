# Tasks: 010-Shader Full Module Implementation

**Input**: plan.md, spec.md, contracts/010-shader-ABI-full.md, specs/_contracts/010-shader-public-api.md, specs/_contracts/010-shader-ABI.md  
**Prerequisites**: plan.md (required), spec.md (required), contracts/010-shader-ABI-full.md (full ABI for implementation)

**模块标识**：010-shader

**Organization**: Tasks grouped by user story; implementation based on full ABI in `contracts/010-shader-ABI-full.md`.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: User story (US1–US4)
- Include exact file paths in descriptions

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Project initialization, CMake structure, include layout

- [x] T001 Create directory structure per plan.md: `include/te/shader/`, `src/`, `src/backends/`, `tests/` at worktree root

- [x] T002 Create CMakeLists.txt at worktree root with project(te_shader), C++17, tenengine_resolve_my_dependencies("010-shader") for 001-core, 008-rhi, 009-rendercore. **注意**：执行 cmake 配置前须已与用户确认**构建根目录**（本 worktree 路径）；各子模块使用源码方式。规约见 `docs/engine-build-module-convention.md` §3。

- [x] T003 [P] Add cmake/TenEngineModuleDependencies.cmake entry for TENENGINE_010_SHADER_DEPS (009-rendercore, 008-rhi, 001-core) if not present

---

## Phase 2: Foundational (Third-Party Dependencies)

**Purpose**: Configure all third-party deps per plan; no stub or empty impl. **共享依赖**：vulkan-headers 与 008-RHI 共享，见 `docs/third_party-integration-workflow.md` §7。

### vulkan-headers（与 008-RHI 共享）

- [x] T004 版本选择：采用 v1.3.280（与 008-RHI 一致），见 docs/third_party/vulkan-headers.md

- [x] T005 自动下载与配置：若 `NOT TARGET Vulkan::Headers`，则 FetchContent_Declare(Vulkan-Headers, GIT_TAG v1.3.280) + FetchContent_MakeAvailable；若已存在（008-RHI 已拉取）则跳过，直接使用。内容名须为 `Vulkan-Headers`。

- [x] T006 配置实现：te_shader target_link_libraries 添加 Vulkan::Headers；在 CMakeLists.txt 中实现

### glslang

- [x] T007 版本选择：采用 14.0.0 或与 vulkan-headers v1.3.280 兼容的 tag，见 docs/third_party/glslang.md

- [x] T008 自动下载：FetchContent_Declare(glslang) + FetchContent_MakeAvailable(glslang)；禁止假设已存在

- [x] T009 配置：SKIP_GLSLANG_INSTALL OFF；glslang 依赖 Vulkan-Headers，须先于 glslang 可用

- [x] T010 配置实现：te_shader target_link_libraries 添加 glslang::glslang（及 SPIRV 等按需）

### spirv-tools

- [x] T011 版本选择：采用与 glslang 兼容的 sdk-* tag，见 docs/third_party/spirv-tools.md（可选：ENABLE_OPT OFF 时不需要）

- [x] T012 自动下载：FetchContent 或由 glslang BUILD_EXTERNAL 拉取；若 glslang 已自动拉取则复用（当前 ENABLE_OPT OFF，跳过）

- [x] T013 配置实现：确保 SPIRV-Tools-opt 等 target 对 te_shader 可用（ENABLE_OPT ON 时再启用）

### spirv-cross

- [x] T014 版本选择：采用 main 或与 glslang 兼容的 tag，见 docs/third_party/spirv-cross.md

- [x] T015 自动下载：FetchContent_Declare(spirv_cross) + FetchContent_MakeAvailable；禁止假设已存在

- [x] T016 配置：SPIRV_CROSS_CLI OFF

- [x] T017 配置实现：te_shader target_link_libraries 添加 spirv-cross-msl spirv-cross-hlsl

### dxc（D3D12 后端，按 TE_RHI_D3D12 启用）

- [x] T018 版本选择：vcpkg directx-dxc 或文档推荐版本，见 docs/third_party/dxc.md

- [x] T019 自动下载/安装：vcpkg install directx-dxc 或 find_package；用户负责安装

- [x] T020 配置实现：若 TE_RHI_D3D12 则 find_package(directx-dxc) 并 target_link_libraries；平台条件 WIN32

### 编译测试（验证第三方集成）

- [x] T021 编译 te_shader：执行 cmake 配置与构建，确保所有第三方正确链接；测试代码中须调用 glslang/spirv-cross 等 API 以验证集成

**Checkpoint**: 第三方依赖就绪，可开始 ABI 实现

---

## Phase 3: User Story 1 - Load and Compile Shader Source (Priority: P1) MVP

**Goal**: Load HLSL/GLSL, compile to SPIR-V/DXIL/MSL, retrieve bytecode

**Independent Test**: Load vertex shader file, compile, GetBytecode; verify bytecode length and validity

### Implementation

- [x] T022 [P] [US1] Implement te/shader/types.hpp: ShaderSourceFormat, MacroSet, VariantKey, CompileOptions, BackendType, IVariantEnumerator, SourceChangedCallback per contracts/010-shader-ABI-full.md

- [x] T023 [P] [US1] Implement te/shader/handle.hpp: IShaderHandle interface with SetMacros, GetVariantKey, SelectVariant

- [x] T024 [P] [US1] Implement te/shader/compiler.hpp: IShaderCompiler interface with LoadSource, LoadSourceFromMemory, ReleaseHandle, Compile, GetBytecode, GetLastError, GetTargetBackend, DefineKeyword, EnumerateVariants, Precompile

- [x] T025 [US1] Implement src/backends/glslang_backend.cpp: GLSL/HLSL→SPIR-V using glslang API；实现 IShaderCompiler 具体类并调用 glslang

- [x] T026 [US1] Implement src/backends/spirv_cross_backend.cpp: SPIR-V→MSL/HLSL using SPIRV-Cross；用于 Metal/D3D11 路径

- [x] T027 [US1] Implement src/backends/dxc_backend.cpp（TE_RHI_D3D12）：HLSL→DXIL using DXC；平台条件 WIN32

- [x] T028 [US1] Implement src/compiler_impl.cpp: ShaderCompilerImpl 整合 glslang/spirv-cross/dxc，实现 LoadSource（路径不存在返回 nullptr、GetLastError 设错误）、LoadSourceFromMemory、ReleaseHandle、Compile（不支持的格式/后端返回 false）、GetBytecode、GetLastError、GetTargetBackend

- [x] T029 [US1] Implement te/shader/factory.hpp and src/factory.cpp: CreateShaderCompiler, DestroyShaderCompiler

- [x] T030 [US1] Implement te/shader/api.hpp: 聚合 include 所有头文件

- [x] T031 [US1] Add tests/test_compile.cpp: 加载 GLSL/HLSL 文件、Compile、GetBytecode；验证 bytecode 非空；调用 glslang/第三方 API 以验证集成；link te_shader（依赖传递）

**Checkpoint**: US1 可独立测试，LoadSource+Compile+GetBytecode 工作

---

## Phase 4: User Story 2 - Macro Variants and Precompile (Priority: P2)

**Goal**: DefineKeyword, SetMacros, GetVariantKey, EnumerateVariants, Precompile, SelectVariant

**Independent Test**: DefineKeyword+SetMacros, EnumerateVariants, Precompile, SelectVariant+GetBytecode 返回缓存字节码

### Implementation

- [x] T032 [US2] Extend ShaderCompilerImpl: DefineKeyword 存储宏定义；EnumerateVariants 收集 `#ifdef` 组合并输出到 IVariantEnumerator

- [x] T033 [US2] Extend IShaderHandle 实现：SetMacros、GetVariantKey、SelectVariant；VariantKey 基于 MacroSet 哈希或有序表示

- [x] T034 [US2] Implement Precompile: 对 VariantKey 列表逐个编译并缓存；空或无效列表返回 false，不修改内部状态，GetLastError 可设

- [x] T035 [US2] Add tests/test_variants.cpp: DefineKeyword、SetMacros、EnumerateVariants、Precompile、SelectVariant+GetBytecode；验证缓存命中

**Checkpoint**: US2 可独立测试，变体与预编译工作

---

## Phase 5: User Story 3 - Cache Load and Save (Priority: P3)

**Goal**: LoadCache, SaveCache, Invalidate；单线程，无并发保证

**Independent Test**: Compile→SaveCache；Invalidate→Compile；LoadCache→验证 bytecode 恢复

### Implementation

- [x] T036 [P] [US3] Implement te/shader/cache.hpp: IShaderCache interface with LoadCache, SaveCache, Invalidate

- [x] T037 [US3] Implement src/cache_impl.cpp: ShaderCacheImpl；二进制缓存格式（magic、version、per-variant entries）；LoadCache/SaveCache/Invalidate；单线程，不保证线程安全

- [x] T038 [US3] Implement CreateShaderCache, DestroyShaderCache in src/factory.cpp

- [x] T039 [US3] Add tests/test_cache.cpp: Compile→SaveCache→Invalidate→LoadCache；验证 bytecode 恢复

**Checkpoint**: US3 可独立测试，缓存持久化工作

---

## Phase 6: User Story 4 - Hot Reload (Priority: P4)

**Goal**: ReloadShader, OnSourceChanged, NotifyShaderUpdated

**Independent Test**: Load shader→修改源文件→OnSourceChanged 回调→ReloadShader→NotifyShaderUpdated

### Implementation

- [x] T040 [P] [US4] Implement te/shader/hot_reload.hpp: IShaderHotReload with ReloadShader, OnSourceChanged(path, callback, userData), NotifyShaderUpdated

- [x] T041 [US4] Implement src/hot_reload_impl.cpp: 文件监视（polling 或 ReadDirectoryChangesW/inotify/FSEvents）；OnSourceChanged 注册回调；ReloadShader 调用 Compile；NotifyShaderUpdated 调用注册的下游回调

- [x] T042 [US4] Implement CreateShaderHotReload, DestroyShaderHotReload in src/factory.cpp

- [x] T043 [US4] Add tests/test_hot_reload.cpp: OnSourceChanged 回调、ReloadShader、NotifyShaderUpdated；可选：修改测试 shader 文件触发

**Checkpoint**: US4 可独立测试，热重载工作

---

## Phase 7: Polish & Cross-Cutting

**Purpose**: CMake 完善、契约写回、quickstart 验证

- [x] T044 完善 CMakeLists.txt: add_library(te_shader STATIC ...)，target_include_directories(PUBLIC include)，target_link_libraries(te_shader PRIVATE te_rendercore te_rhi te_core glslang::glslang spirv_cross ...)，按平台条件链接 dxc

- [x] T045 cmake 生成后检查：头文件/源文件完整、无循环依赖、依赖链正确；发现问题标注或修复

- [x] T046 将 plan.md「契约更新」中的新增/修改条目写回 specs/_contracts/010-shader-ABI.md（仅增补，不覆盖全文件）

- [x] T047 [P] 运行 quickstart.md 示例：验证 CreateShaderCompiler→LoadSource→Compile→GetBytecode→ReleaseHandle→Destroy 流程

- [x] T048 添加 tenengine_add_module_test 或类似测试入口，确保 tests 可执行

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 1 (Setup)**: 无依赖
- **Phase 2 (Foundational)**: 依赖 Phase 1；**阻塞**所有 User Story
- **Phase 3 (US1)**: 依赖 Phase 2
- **Phase 4 (US2)**: 依赖 Phase 3（依赖 Compile、Handle）
- **Phase 5 (US3)**: 依赖 Phase 3
- **Phase 6 (US4)**: 依赖 Phase 3、5（HotReload 依赖 Compiler、Cache）
- **Phase 7 (Polish)**: 依赖 Phase 3–6

### User Story Dependencies

- **US1**: 无其他 story 依赖；MVP
- **US2**: 依赖 US1（Compiler、Handle）
- **US3**: 依赖 US1（Compiler、Handle）
- **US4**: 依赖 US1、US3（Compiler、Cache）

### Parallel Opportunities

- T022–T024 可并行（不同头文件）
- T036、T040 可并行
- Phase 4 与 Phase 5 部分可并行（Cache 与 Variants 相对独立）

---

## Implementation Strategy

### MVP First (US1 Only)

1. Phase 1: Setup
2. Phase 2: Foundational（第三方完整配置，禁止 stub）
3. Phase 3: US1
4. **STOP and VALIDATE**: test_compile 通过，quickstart 可跑

### Incremental Delivery

1. US1 → MVP
2. US2 → 变体与预编译
3. US3 → 缓存
4. US4 → 热重载

---

## Notes

- 全量 ABI 见 `contracts/010-shader-ABI-full.md`；实现须覆盖全部符号
- 第三方：vulkan-headers 与 008-RHI 共享；使用 `if(NOT TARGET Vulkan::Headers)` 复用
- 测试须调用上游（te_core、te_rhi、te_rendercore）与第三方（glslang、spirv-cross）API，验证依赖链
- cmake 执行前须确认构建根目录
