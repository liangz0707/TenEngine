# Implementation Plan: 001-Core 完整模块实现

**Branch**: `001-core-fullmodule` | **Date**: 2026-01-29 | **Spec**: [spec.md](./spec.md)  
**Input**: Feature specification from `specs/001-core-fullmodule/spec.md`

**Note**: This template is filled in by the `/speckit.plan` command. See `.specify/templates/commands/plan.md` for the execution workflow.

## Summary

本 feature 实现 001-Core **完整模块内容**，即 **`specs/_contracts/001-core-ABI.md` 描述的全部内容**：规约见 `docs/module-specs/001-core.md`，契约见 `specs/_contracts/001-core-public-api.md`。技术栈 C++17、CMake；仅暴露/使用契约已声明的类型与 API；设计参考 **Unity、Unreal** 的 Core 层模块与 API 构造。无上游依赖；实现 ABI 表中**全部**符号与能力，不遗漏、不截取。

## 实现范围（TenEngine：仅实现 ABI 内容）

> **规约**：本 feature **只实现 ABI 文件**（`specs/_contracts/001-core-ABI.md`）中列出的符号与能力；不得设计或实现 ABI 未声明的对外接口。设计时可参考 **Unity、Unreal** 的模块与 API 构造。对外接口以 ABI 文件为准。**若对现存 ABI 有新增或变更**，则在本 plan 末尾产出一份「契约更新」（命名空间、头文件、符号与完整签名）。见 `specs/_contracts/README.md`、`.specify/memory/constitution.md` §VI。

本 feature 实现的 ABI 范围：**`specs/_contracts/001-core-ABI.md` 描述的全部内容**（全部表行），涵盖：

- **alloc.h**：Alloc, Free, Allocator, DefaultAllocator, GetDefaultAllocator
- **engine.h**：Init, Shutdown, InitParams
- **thread.h**：Thread, TLS\<T\>, Atomic\<T\>, Mutex, LockGuard, ConditionVariable, TaskQueue, IThreadPool, TaskCallback, GetThreadPool
- **platform.h**：TE_PLATFORM_* 宏, FileRead, FileWrite(×2), DirEntry, DirectoryEnumerate, Time, HighResolutionTimer, GetEnv, PathNormalize
- **log.h**：LogLevel, LogSink, Log, LogSetLevelFilter, LogSetStderrThreshold, LogSetSink, Assert, CrashHandlerFn, SetCrashHandler
- **check.h**：CheckWarning, CheckError 宏
- **math.h**：Scalar, Vector2/3/4, Matrix3/4, Quaternion, AABB, Ray, Lerp, Dot, Cross, Length, Normalize
- **containers.h**：Array\<T, Allocator\>, Map\<K,V,...\>, String, UniquePtr\<T\>, SharedPtr\<T\>
- **module_load.h**：ModuleHandle, LoadLibrary, UnloadLibrary, GetSymbol, ModuleInitFn, ModuleShutdownFn, RegisterModuleInit, RegisterModuleShutdown, RunModuleInit, RunModuleShutdown

命名空间：TenEngine::core（实现可用 te::core）；头文件路径 te/core/ = TenEngine/core/。实现须与 ABI 表每一行一一对应，无遗漏。

## Technical Context

**Language/Version**: C++17  
**Build**: CMake 3.16+  
**Primary Dependencies**: 无第三方库；仅 C++ 标准库（`<cstdlib>`, `<thread>`, `<mutex>`, `<atomic>`, `<filesystem>`, `<chrono>`, `<functional>` 等）与平台 API（Win32 / POSIX / dyld）。  
**Storage**: N/A（本模块不定义持久化存储；文件 I/O 仅平台能力）。  
**Testing**: 单元测试（tests/unit/test_alloc, test_engine, test_thread, test_platform, test_log, test_check, test_math, test_containers, test_module_load）；CMake CTest 或直接运行可执行文件。  
**Target Platform**: Windows（MSVC）、Linux（GCC/Clang）、macOS（Clang）；可选 Android/iOS 宏与平台路径。

**Project Type**: 单库（te_core 静态库或动态库），头文件 include/te/core/，实现 src/，测试 tests/unit/。  
**Performance Goals**: 分配/释放、线程/任务、文件 I/O 与数学运算满足下游实时与工具链需求；无具体量化目标在规约中。  
**Constraints**: 仅暴露 ABI 与契约声明的类型与 API；不引入反射与 ECS。  
**Scale/Scope**: 实现 **001-core-ABI.md 描述的全部内容**（约 70+ 符号）；头文件 9 个，源文件对应 9 个（或合并）。

## 依赖引入方式（TenEngine 构建规约：必须澄清）

> **规约**：见 `docs/engine-build-module-convention.md`。对本 feature 的**每个直接依赖**必须明确写出引入方式之一；**未写明时默认使用源码引入**。

| 依赖模块（如 001-core） | 引入方式 | 说明 |
|-------------------------|----------|------|
| 无上游模块 | **不引入外部库** | 001-Core 为根模块；仅依赖 C++17 标准库与平台 API，不链接其他引擎模块。 |

**说明**：本模块不依赖其他 TenEngine 模块；构建时无需 add_subdirectory 或 FetchContent 引入上游。若未来下游模块依赖本模块，由下游在其 worktree 中以源码或预编译库方式引入 001-Core。

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

- **§I–V**：模块化、现代图形 API、数据驱动、性能可观测、版本化 — 本 feature 为 Core 基础设施，不涉及渲染器或图形 API；满足模块边界与版本化（契约/ABI 为准）。
- **§VI Module Boundaries & Contract-First**：
  - ABI 与模块边界以 `specs/_contracts/001-core-public-api.md` 与 `001-core-ABI.md` 为准；仅暴露契约已声明的类型与接口。 **PASS**
  - 实现仅使用契约中声明的类型与接口；无上游故无上游契约依赖。 **PASS**
  - 实现 ABI 文件中的**全部**符号与能力（001-core-ABI.md 描述的全部内容）；禁止长期 stub/占位。 **PASS**
  - 构建通过真实源码（本库自身），不以外部 stub 代替本模块。 **PASS**
  - 契约更新：本 feature 无对 ABI 的新增或变更；若有则在本 plan 末尾产出「契约更新」。 **PASS**
- **Technology Stack**：C++17、CMake；符合宪法。 **PASS**
- **Code Quality & Testing**：单元测试覆盖契约保证；无豁免。 **PASS**

**Result**: 全部通过，无豁免项。

## Project Structure

### Documentation (this feature)

```text
specs/001-core-fullmodule/
├── plan.md              # This file (/speckit.plan command output)
├── research.md          # Phase 0 output (Unity/UE Core 参考与 C++17 决策)
├── data-model.md        # Phase 1 output (实体与类型摘要)
├── quickstart.md        # Phase 1 output (构建与 Init–Shutdown 顺序)
├── contracts/           # 可选；本 feature 契约已在 specs/_contracts/，可不复制
└── tasks.md             # Phase 2 output (/speckit.tasks command - NOT created by /speckit.plan)
```

### Source Code (repository root)

```text
include/te/core/
├── alloc.h
├── engine.h
├── thread.h
├── platform.h
├── log.h
├── check.h
├── math.h
├── containers.h
└── module_load.h

src/
├── alloc.cpp
├── engine.cpp
├── thread.cpp
├── platform.cpp
├── log.cpp
├── math.cpp
├── containers.cpp
└── module_load.cpp

tests/
├── CMakeLists.txt
└── unit/
    ├── test_alloc.cpp
    ├── test_engine.cpp
    ├── test_thread.cpp
    ├── test_platform.cpp
    ├── test_log.cpp
    ├── test_check.cpp
    ├── test_math.cpp
    ├── test_containers.cpp
    └── test_module_load.cpp

CMakeLists.txt           # 根：project(te_core), add_library(te_core ...), add_subdirectory(tests)
```

**Structure Decision**: 单库 + 单测目录；头文件与 ABI 表一一对应；实现须覆盖 **001-core-ABI.md 描述的全部内容**，便于维护与契约对齐。

## Complexity Tracking

> **Fill ONLY if Constitution Check has violations that must be justified**

无违规；本表留空。

---

## 契约更新（Contract Update）

本 feature **无对现存 ABI 的新增或变更**。对外接口以以下文件为唯一依据：

- **ABI 表**：`specs/_contracts/001-core-ABI.md`（命名空间、头文件、符号与完整签名）
- **能力与类型描述**：`specs/_contracts/001-core-public-api.md`

若本 feature 或后续迭代**新增或修改**了任何 ABI 条目，则须在本 plan 末尾或单独一节中产出一份「契约更新」，列出：

- **命名空间**：TenEngine::core（te::core）
- **头文件**：te/core/*.h（与 ABI 表一致）
- **符号与完整签名**：按 ABI 表格式列出新增/变更的每一行

并同步更新 `specs/_contracts/001-core-ABI.md` 与 `specs/_contracts/001-core-public-api.md`。

**本次结论**：无需变更 ABI 或 public-api；实现严格按 **001-core-ABI.md 描述的全部内容** 完成即可。
