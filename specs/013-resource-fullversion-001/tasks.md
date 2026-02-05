# Tasks: 013-Resource 最小切片（ResourceId / LoadSync / Release）

**Feature**: 013-resource-fullversion-001 | **Branch**: 013-resource-fullversion-001  
**Input**: plan.md、spec.md、specs/_contracts/013-resource-public-api.md  
**Constraint**: 任务仅暴露契约已声明的 API（ResourceId、LoadHandle、LoadResult、LoadSync、Release）；不实现契约未列出的类型或函数。

## Format: `[ID] [P?] [Story] Description`

- **[P]**: 可并行（不同文件、无未完成依赖）
- **[Story]**: US1 = 通过 ResourceId 标识并同步加载资源（P1），US2 = 释放资源句柄（P2）
- 描述中带具体文件路径

## Path Conventions

- 仓库根：TenEngine-013-resource（当前 worktree）
- 头文件：`include/te_resource/resource.hpp`
- 实现：`src/resource.cpp`
- 单元测试：`tests/unit/resource_test.cpp`
- CMake：`cmake/TenEngineHelpers.cmake`、`cmake/TenEngineModuleDependencies.cmake`、根目录 `CMakeLists.txt`

---

## Phase 1: Setup（共享基础设施）

**Purpose**: 项目初始化与目录结构，与 plan 一致。

- [ ] T001 Create directory structure per plan: `include/te_resource/`, `src/`, `tests/unit/`, and ensure `cmake/` exists at repository root
- [ ] T002 Add root `CMakeLists.txt`: `cmake_minimum_required` 3.x, `project`, `set(CMAKE_CXX_STANDARD 17)`, `set(TENENGINE_CMAKE_DIR ...)`, `include(TenEngineHelpers.cmake)`, `tenengine_resolve_my_dependencies("013-resource" OUT_DEPS MY_DEPS)`, `add_library(te_resource STATIC src/resource.cpp)`, `target_include_directories(te_resource PUBLIC include)`, `target_link_libraries(te_resource PRIVATE ${MY_DEPS})`. **构建说明**：执行 `cmake -B build` 前须已与用户确认**构建方式**（各依赖 001-core、002-object 为源码/DLL）与**根目录**（在 TenEngine-013-resource 下构建）；未澄清时禁止直接执行 cmake。规约见 `docs/build-module-convention.md` §1.1。
- [ ] T003 [P] Ensure `cmake/TenEngineHelpers.cmake` and `cmake/TenEngineModuleDependencies.cmake` exist (copy from upstream or repo); `TENENGINE_013_RESOURCE_DEPS` 已包含 001-core、002-object

---

## Phase 2: Foundational（阻塞性前置）

**Purpose**: 对外 API 声明与可链接桩实现，仅包含契约已声明的类型与函数；所有 User Story 依赖本阶段完成。

- [ ] T004 [P] Declare public API in `include/te_resource/resource.hpp` strictly per `specs/_contracts/013-resource-public-api.md` API 雏形：`namespace te::resource { struct ResourceId { enum class Kind { Path, Guid }; Kind kind; char const* value; }; using LoadHandle = void*; struct LoadResult { bool success; LoadHandle handle; int error_code; }; LoadResult LoadSync(ResourceId const& id); void Release(LoadHandle handle); }` — 不添加契约未声明的符号。
- [ ] T005 Add `src/resource.cpp` with stub implementations: `LoadSync` 返回 `LoadResult{ false, nullptr, 非0 }`，`Release` 空实现，使库可链接；仅使用 001-Core、002-Object 契约已声明的类型与 API（若桩阶段暂不调用上游，则仅保证编译通过）。

---

## Phase 3: User Story 1 - 通过 ResourceId 标识并同步加载资源 (Priority: P1) — MVP

**Goal**: 调用方传入 ResourceId（Path 或 GUID），调用 LoadSync 得到 LoadResult；成功时返回新 LoadHandle，失败时 success==false 且带 error_code，不抛异常。

**Independent Test**: 给定合法 ResourceId 调用 LoadSync 得到有效 LoadHandle；给定无效 ResourceId 得到 LoadResult.success==false 且 error_code 有效，不崩溃。

### Implementation for User Story 1

- [ ] T006 [US1] Implement LoadSync in `src/resource.cpp`：根据 ResourceId.kind 与 value 解析（路径或 GUID），仅使用 001-Core、002-Object 契约已声明的接口；成功时分配并返回新 LoadHandle（每次成功返回新句柄），失败时返回 `LoadResult{ false, nullptr, error_code }`；不抛异常。
- [ ] T007 [US1] Implement internal handle registry or allocation in `src/resource.cpp` so that each successful LoadSync returns a distinct LoadHandle and resources can be tracked for Release (implementation detail, not exposed in API).

### Tests for User Story 1

- [ ] T008 [P] [US1] Add unit test in `tests/unit/resource_test.cpp`: LoadSync with valid ResourceId yields LoadResult.success==true and non-null handle
- [ ] T009 [P] [US1] Add unit test in `tests/unit/resource_test.cpp`: LoadSync with invalid or null ResourceId yields LoadResult.success==false and non-zero error_code, no exception
- [ ] T010 [US1] Add `tenengine_add_module_test(NAME te_resource_test MODULE_TARGET te_resource SOURCES tests/unit/resource_test.cpp ENABLE_CTEST)` in root `CMakeLists.txt` if not present

**Checkpoint**: User Story 1 可独立验收：LoadSync 成功/失败语义与契约一致。

---

## Phase 4: User Story 2 - 释放资源句柄 (Priority: P2)

**Goal**: 调用方对 LoadHandle 调用 Release 后句柄失效、资源可回收；对同一句柄多次 Release 为幂等（无操作/成功），不崩溃。

**Independent Test**: LoadSync 得到 handle 后调用 Release，句柄失效；对同一 handle 再次 Release 不崩溃、不报错。

### Implementation for User Story 2

- [ ] T011 [US2] Implement Release in `src/resource.cpp`: 释放传入的 LoadHandle 对应资源，与内部句柄表协调；对已释放或无效句柄再次调用为幂等（无操作）；不抛异常。

### Tests for User Story 2

- [ ] T012 [P] [US2] Add unit test in `tests/unit/resource_test.cpp`: after LoadSync success, Release(handle) invalidates handle; subsequent use of handle is undefined but double-Release is idempotent (no crash)
- [ ] T013 [US2] Add unit test in `tests/unit/resource_test.cpp`: same ResourceId passed to LoadSync multiple times returns distinct handles; each handle must be Released exactly once (or double-Release idempotent)

**Checkpoint**: User Stories 1 与 2 均可独立验收；LoadSync + Release 生命周期与契约一致。

---

## Phase 5: Polish & Cross-Cutting

**Purpose**: 与契约一致性检查、文档与收尾。

- [ ] T014 [P] Validate `quickstart.md` usage: include path `te_resource/resource.hpp`, LoadSync/Release call sequence and error handling match implementation
- [ ] T015 Verify no extra public symbols: `include/te_resource/resource.hpp` and exported library surface expose only ResourceId, LoadHandle, LoadResult, LoadSync, Release per `specs/_contracts/013-resource-public-api.md`

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 1 (Setup)**: 无依赖，可立即开始
- **Phase 2 (Foundational)**: 依赖 Phase 1；阻塞所有 User Story
- **Phase 3 (US1)**: 依赖 Phase 2；可独立于 US2 实现与测试
- **Phase 4 (US2)**: 依赖 Phase 2；依赖 US1 的 LoadSync 与 LoadHandle 以测试 Release
- **Phase 5 (Polish)**: 依赖 Phase 3、4 完成

### User Story Dependencies

- **US1 (P1)**: Phase 2 完成后即可开始；无其他 Story 依赖
- **US2 (P2)**: Phase 2 完成后即可开始；测试需 US1 的 LoadSync/Handle

### Parallel Opportunities

- T003 与 T001/T002 可并行（不同目录/文件）
- T004 与 T005 可并行（头文件 vs 源文件）
- T008、T009 可并行；T012、T013 可并行
- T014、T015 可并行

---

## Implementation Strategy

### MVP First (User Story 1)

1. Phase 1 → Phase 2 → Phase 3（T006–T010）
2. 验收：LoadSync 成功/失败、结果类型与契约一致
3. 再推进 Phase 4（Release）与 Phase 5

### Contract Compliance

- 实现中仅使用 `specs/_contracts/001-core-public-api.md`、`specs/_contracts/002-object-public-api.md` 已声明的类型与 API。
- 本模块对外仅暴露 `specs/_contracts/013-resource-public-api.md` API 雏形中的 ResourceId、LoadHandle、LoadResult、LoadSync、Release。

---

## Notes

- 构建/执行 cmake 前必须已确认**构建方式**与**根目录**（见 T002 与 `docs/build-module-convention.md` §1.1）。
- 每项任务完成后可单独提交；按 Phase checkpoint 验收后再进入下一 Phase。
