# Tasks: 001-core-fullmodule

**Input**: Design documents from `specs/001-core-fullmodule/`  
**Prerequisites**: plan.md, spec.md, research.md, data-model.md, quickstart.md  
**Contract**: Only expose APIs declared in [specs/_contracts/001-core-public-api.md](../_contracts/001-core-public-api.md) and [001-core-ABI.md](../_contracts/001-core-ABI.md). **Symbols and signatures follow the ABI file.** No additional public API.

**Organization**: Tasks are grouped by user story (spec.md). Each task implements or tests only contract/ABI-declared types and API.

## Format: `[ID] [P?] [Story?] Description`

- **[P]**: Can run in parallel (different files, no dependencies on incomplete tasks)
- **[Story]**: Which user story (US1–US7) this task belongs to
- Include exact file paths; only contract/ABI-declared symbols in public headers

## Path Conventions (per plan.md)

- Headers: `include/te/core/*.h`
- Source: `src/*.cpp`
- Tests: `tests/unit/test_*.cpp`

## Build/CMake (TenEngine)

若任务包含「配置/构建工程」或执行 `cmake -B build`，执行前须已澄清 **构建方式**（各依赖 源码/DLL/不引入）与 **根目录**（构建所在模块路径）；未澄清时**禁止**直接执行 cmake，须先向用户询问。规约见 `docs/engine-build-module-convention.md` §3.1。

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Project structure and CMake per plan.md; no contract API yet.

- [x] T001 Verify or create directory structure per plan.md: `include/te/core/`, `src/`, `tests/unit/` at repository root
- [x] T002 Verify or create root CMakeLists.txt: project(te_core), C++17, add_library(te_core ...), add_subdirectory(tests)
- [x] T003 Verify or create tests/CMakeLists.txt: add_executable(test_*), target_link_libraries(test_* PRIVATE te_core) for each test per plan

---

## Phase 2: Memory – alloc.h (US2 内存分配与默认分配器)

**Purpose**: Implement ABI 表 alloc.h 全部符号；仅暴露契约/ABI 声明的 API，签名以 001-core-ABI.md 为准。

- [x] T004 [P] [US2] Declare in include/te/core/alloc.h: Alloc, Free, Allocator, DefaultAllocator, GetDefaultAllocator（签名与 001-core-ABI.md 一致，无额外符号）
- [x] T005 [US2] Implement in src/alloc.cpp: Alloc, Free, DefaultAllocator, GetDefaultAllocator；double-free 为 no-op
- [x] T006 [US2] Add or extend tests/unit/test_alloc.cpp: Alloc/Free 语义，GetDefaultAllocator 非空且 Alloc/Free 与全局一致，Free(nullptr) 与 double-free no-op

---

## Phase 3: Engine – engine.h (US1 进程启动与 Core 初始化)

**Purpose**: Implement ABI 表 engine.h 全部符号；仅暴露契约/ABI 声明的 API。

- [x] T007 [P] [US1] Declare in include/te/core/engine.h: InitParams, Init, Shutdown（签名与 001-core-ABI.md 一致）
- [x] T008 [US1] Implement in src/engine.cpp: Init（幂等）, Shutdown；可选 InitParams 处理
- [x] T009 [US1] Add tests/unit/test_engine.cpp: Init/Shutdown 序列，Init 幂等，Shutdown 后不再使用 Core 句柄

---

## Phase 4: Platform – platform.h (US3 平台与文件/时间)

**Purpose**: Implement ABI 表 platform.h 全部符号；仅暴露契约/ABI 声明的 API。

- [ ] T010 [P] [US3] Declare in include/te/core/platform.h: TE_PLATFORM_* 宏, FileRead, FileWrite(×2), DirEntry, DirectoryEnumerate, Time, HighResolutionTimer, GetEnv, PathNormalize（签名与 001-core-ABI.md 一致）
- [ ] T011 [US3] Implement in src/platform.cpp: 上述 API 的实现
- [ ] T012 [US3] Add or extend tests/unit/test_platform.cpp: 平台宏、FileWrite/FileRead、Time/HighResolutionTimer、PathNormalize 等契约行为

---

## Phase 5: Thread – thread.h (US4 线程与线程池)

**Purpose**: Implement ABI 表 thread.h 全部符号；仅暴露契约/ABI 声明的 API。

- [x] T013 [P] [US4] Declare in include/te/core/thread.h: Thread, TLS\<T\>, Atomic\<T\>, Mutex, LockGuard, ConditionVariable, TaskQueue, IThreadPool, TaskCallback, GetThreadPool（签名与 001-core-ABI.md 一致）
- [x] T014 [US4] Implement in src/thread.cpp: 上述类型与函数；GetThreadPool 返回非空 IThreadPool*
- [x] T015 [US4] Add or extend tests/unit/test_thread.cpp: GetThreadPool 非空，SubmitTask 在工作线程执行 callback

---

## Phase 6: Log & Check – log.h, check.h (US5 日志与 Check)

**Purpose**: Implement ABI 表 log.h、check.h 全部符号；仅暴露契约/ABI 声明的 API。

- [x] T016 [P] [US5] Declare in include/te/core/log.h: LogLevel, LogSink, Log, LogSetLevelFilter, LogSetStderrThreshold, LogSetSink, Assert, CrashHandlerFn, SetCrashHandler（签名与 001-core-ABI.md 一致）
- [x] T017 [P] [US5] Declare in include/te/core/check.h: CheckWarning, CheckError 宏（与 001-core-ABI.md 一致）
- [x] T018 [US5] Implement in src/log.cpp: Log、LogSetLevelFilter、LogSetStderrThreshold、LogSetSink、Assert、SetCrashHandler
- [x] T019 [US5] Add or extend tests/unit/test_log.cpp、tests/unit/test_check.cpp: Log 各级别输出，CheckWarning/CheckError 条件为假时行为

---

## Phase 7: Math – math.h (US6 数学与容器 之数学)

**Purpose**: Implement ABI 表 math.h 全部符号；仅暴露契约/ABI 声明的 API。

- [ ] T020 [P] [US6] Declare in include/te/core/math.h: Scalar, Vector2/3/4, Matrix3/4, Quaternion, AABB, Ray, Lerp, Dot, Cross, Length, Normalize（签名与 001-core-ABI.md 一致）
- [ ] T021 [US6] Implement in src/math.cpp（或仅头文件实现）: 上述类型与函数
- [ ] T022 [US6] Add or extend tests/unit/test_math.cpp: Lerp、Dot、Length、Normalize 等契约行为

---

## Phase 8: Containers – containers.h (US6 数学与容器 之容器)

**Purpose**: Implement ABI 表 containers.h 全部符号；仅暴露契约/ABI 声明的 API。

- [ ] T023 [P] [US6] Declare in include/te/core/containers.h: Array\<T, Allocator\>, Map\<K,V,...\>, String, UniquePtr\<T\>, SharedPtr\<T\>（与 001-core-ABI.md 一致，std 等价或类型别名）
- [ ] T024 [US6] Implement in src/containers.cpp（或仅头文件）：若需实现则仅暴露契约类型
- [ ] T025 [US6] Add or extend tests/unit/test_containers.cpp: Array/Map/String/UniquePtr/SharedPtr 契约行为

---

## Phase 9: ModuleLoad – module_load.h (US7 模块加载)

**Purpose**: Implement ABI 表 module_load.h 全部符号；仅暴露契约/ABI 声明的 API。

- [ ] T026 [P] [US7] Declare in include/te/core/module_load.h: ModuleHandle, LoadLibrary, UnloadLibrary, GetSymbol, ModuleInitFn, ModuleShutdownFn, RegisterModuleInit, RegisterModuleShutdown, RunModuleInit, RunModuleShutdown（签名与 001-core-ABI.md 一致）
- [ ] T027 [US7] Implement in src/module_load.cpp: 上述 API
- [ ] T028 [US7] Add or extend tests/unit/test_module_load.cpp: LoadLibrary/GetSymbol、RunModuleInit/RunModuleShutdown 顺序

---

## Phase 10: Polish & Contract Alignment

**Purpose**: Ensure public API matches contract/ABI only; run all tests.

- [ ] T029 Ensure include/te/core/*.h expose only types and APIs listed in specs/_contracts/001-core-ABI.md and 001-core-public-api.md; remove or hide any extra public API
- [ ] T030 Update README or docs with build/run and init–shutdown order per quickstart.md and contract「调用顺序与约束」
- [ ] T031 Run quickstart validation: 构建（执行前须已澄清构建方式与根目录），运行全部单元测试（test_alloc, test_engine, test_thread, test_platform, test_log, test_check, test_math, test_containers, test_module_load）

---

## Dependencies & Execution Order

- Phase 1 (Setup): No dependencies.
- Phase 2 (Memory): Depends on Phase 1；其他模块可能依赖 Allocator。
- Phase 3–9: 依赖 Phase 1；各 Phase 之间可按 US 并行（如 Platform 与 Thread 可并行）。
- Phase 10: 依赖 Phase 2–9 完成。

## Contract / ABI

- All declarations and definitions MUST match [specs/_contracts/001-core-ABI.md](../_contracts/001-core-ABI.md) and [001-core-public-api.md](../_contracts/001-core-public-api.md). **Symbols and signatures follow the ABI file.** No additional public API.
