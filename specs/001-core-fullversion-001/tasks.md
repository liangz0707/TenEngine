# Tasks: 001-core-fullversion-001

**Input**: Design documents from `specs/001-core-fullversion-001/`  
**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/  
**Contract**: Only expose APIs declared in [specs/_contracts/001-core-public-api.md](../_contracts/001-core-public-api.md). No additional public API.

**Organization**: Tasks grouped by user story (US1=Memory+Thread, US2=Platform+Log, US3=Math+Containers+ModuleLoad). Implementation order respects submodule dependencies: Memory → Platform/Thread → Log → Containers → ModuleLoad.

## Format: `[ID] [P?] [Story?] Description`

- **[P]**: Can run in parallel (different files, no dependencies on incomplete tasks)
- **[Story]**: US1, US2, US3 per spec.md
- Include exact file paths; only contract-declared types/API in public headers

## Path Conventions (per plan.md)

- Headers: `include/te/core/*.h`
- Source: `src/*.cpp` (platform-specific e.g. `alloc_win32.cpp` / `alloc_posix.cpp` as needed)
- Tests: `tests/unit/test_*.cpp`

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Project initialization and CMake structure per plan.md

- [x] T001 Create directory structure: `include/te/core/`, `src/`, `tests/unit/` at repository root per plan.md
- [x] T002 Create root CMakeLists.txt: C++17, library target, include dirs, and tests/CMakeLists.txt for unit tests
- [x] T003 [P] Add include/te/core/ placeholder headers (empty or include guards only): alloc.h, thread.h, platform.h, log.h, math.h, containers.h, module_load.h

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Memory submodule MUST be complete before Platform, Thread, Containers, and ModuleLoad (per plan dependency: Platform→Memory, Thread→Memory, Containers→Memory).

**⚠️ CRITICAL**: No user story implementation (beyond Memory) can begin until Memory contract API is available.

- [x] T004 Declare in include/te/core/alloc.h: Allocator interface (Alloc(size_t, size_t), Free(void*)), DefaultAllocator, global Alloc/Free; only contract-declared API
- [x] T005 Implement in src/alloc.cpp (or platform-specific alloc_win32.cpp/alloc_posix.cpp): Alloc(size_t size, size_t alignment), Free(void* ptr); size==0 or invalid alignment→nullptr, Free(nullptr)/double-free no-op
- [x] T006 Implement DefaultAllocator and optional AlignedAlloc; optional PoolAllocator/DebugAllocator/LeakTracker per contract capability 1 in src/alloc.cpp
- [x] T007 Add tests/unit/test_alloc.cpp: unit tests for Alloc/Free semantics, alignment, nullptr and double-free behavior per contract

**Checkpoint**: Memory API stable; Platform, Thread, Log, Containers, ModuleLoad can proceed per user stories

---

## Phase 3: User Story 1 - 内存与线程基础 (Priority: P1)

**Goal**: Downstream or main project uses Core for heap/aligned allocation and thread creation/synchronization; behavior conforms to contract capabilities 1 and 2.

**Independent Test**: Call Alloc/Free, create thread, use Mutex sync; verify contract semantics and lifecycle.

### Implementation for User Story 1

- [x] T008 [P] [US1] Declare in include/te/core/thread.h: Thread (create/join/detach), TLS, Atomic<T>, Mutex, ConditionVariable, TaskQueue skeleton; only contract-declared API
- [x] T009 [US1] Implement in src/thread.cpp (or thread_win32.cpp/thread_pthread.cpp): Thread create/destroy, join/detach; platform abstraction per plan
- [x] T010 [US1] Implement in src/thread.cpp: TLS, Atomic<T>, Mutex, ConditionVariable; semantics per contract capability 2
- [x] T011 [US1] Implement TaskQueue skeleton in src/thread.cpp: submit task, execution and sync semantics per contract
- [x] T012 [US1] Add tests/unit/test_thread.cpp: thread creation, Mutex/ConditionVariable, TaskQueue usage per contract

**Checkpoint**: US1 complete; Memory + Thread API available and testable independently

---

## Phase 4: User Story 2 - 平台与日志 (Priority: P2)

**Goal**: Callers use file I/O, time, env/path, leveled log and assert; OS-agnostic, redirectable and filterable.

**Independent Test**: File read/write, directory enumerate, time/timer, Log level and channel, Assert; verify contract capabilities 3 and 4.

### Implementation for User Story 2

- [x] T013 [P] [US2] Declare in include/te/core/platform.h: FileRead/Write, DirectoryEnumerate, Time/HighResolutionTimer, GetEnv, PathNormalize, platform macros (Windows/Linux/macOS); only contract-declared API
- [x] T014 [US2] Implement in src/platform.cpp (or platform_win32.cpp/platform_posix.cpp): FileRead/Write, DirectoryEnumerate, Time, HighResolutionTimer, GetEnv, PathNormalize; platform abstraction per plan
- [x] T015 [US2] Add tests/unit/test_platform.cpp: file I/O, dir enumerate, time/timer, GetEnv, PathNormalize per contract
- [x] T016 [P] [US2] Declare in include/te/core/log.h: LogLevel (Debug/Info/Warn/Error), LogSink, Log(level, message), Assert, CrashHandler; only contract-declared API
- [x] T017 [US2] Implement in src/log.cpp: Log(LogLevel, char const*), thread-safe, single-message atomic; LogSink redirect/filter; Assert and CrashHandler per contract capability 4
- [x] T018 [US2] Add tests/unit/test_log.cpp: level filtering, stderr threshold, Assert/CrashHandler behavior per contract

**Checkpoint**: US2 complete; Platform + Log API available and testable independently

---

## Phase 5: User Story 3 - 数学、容器与模块加载 (Priority: P3)

**Goal**: Callers use math types, containers, smart pointers, and dynamic library load/unload/symbol; no GPU dependency; containers work with custom allocator.

**Independent Test**: Vector/matrix/quaternion, Array/Map/String, UniquePtr/SharedPtr, LoadLibrary/GetSymbol; verify contract capabilities 5, 6, 7.

### Implementation for User Story 3

- [x] T019 [P] [US3] Declare in include/te/core/math.h: Vector2/3/4, Matrix3/4, Quaternion, AABB, Ray; Lerp and common math functions; only contract-declared API, no GPU
- [x] T020 [US3] Implement in src/math.cpp: Vector2/3/4, Matrix3/4, Quaternion, AABB, Ray, Lerp per contract capability 5
- [x] T021 [US3] Add tests/unit/test_math.cpp: vector/matrix/quaternion ops, AABB, Ray, Lerp per contract
- [x] T022 [P] [US3] Declare in include/te/core/containers.h: Array, Map, String, UniquePtr, SharedPtr; allocator support; only contract-declared API, no reflection/ECS
- [x] T023 [US3] Implement in src/containers.cpp: Array, Map, String, UniquePtr, SharedPtr; optional allocator parameter per contract capability 6
- [x] T024 [US3] Add tests/unit/test_containers.cpp: Array/Map/String and smart pointers with allocator per contract
- [x] T025 [US3] Declare in include/te/core/module_load.h: LoadLibrary, UnloadLibrary, GetSymbol, ModuleInit/Shutdown callbacks; only contract-declared API
- [x] T026 [US3] Implement in src/module_load.cpp: LoadLibrary/UnloadLibrary/GetSymbol (dlopen/LoadLibrary/dyld); module dependency order and Init/Shutdown callbacks per contract capability 7
- [x] T027 [US3] Add tests/unit/test_module_load.cpp: load/unload/symbol and init/shutdown order per contract

**Checkpoint**: US3 complete; Math, Containers, ModuleLoad API available and testable independently

---

## Phase 6: Polish & Cross-Cutting Concerns

**Purpose**: Documentation, ABI versioning, and contract alignment

- [x] T028 [P] Update README or docs with build/run instructions per quickstart.md
- [x] T029 Ensure all public headers under include/te/core/ contain only types and APIs listed in specs/_contracts/001-core-public-api.md; remove any extra public API
- [x] T030 Add ABI version (MAJOR.MINOR.PATCH) and versioning note in public API per contract; document init/shutdown order per contract「调用顺序与约束」
- [x] T031 Run quickstart.md validation: configure, build, run unit tests for all 7 submodules

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 1 (Setup)**: No dependencies — start immediately
- **Phase 2 (Foundational)**: Depends on Phase 1 — Memory MUST be done before Platform/Thread/Containers/ModuleLoad
- **Phase 3 (US1)**: Depends on Phase 2 — Thread implementation uses Memory
- **Phase 4 (US2)**: Depends on Phase 2 — Platform uses Memory; Log uses Platform
- **Phase 5 (US3)**: Depends on Phase 2 and Phase 4 for ModuleLoad (Platform+Containers); Math and Containers depend on Phase 2 (Memory)
- **Phase 6 (Polish)**: Depends on Phases 3–5 complete

### Submodule Dependency (from plan)

- Memory → first (Phase 2)
- Platform, Thread → after Memory (Phases 4, 3)
- Log → after Platform (Phase 4)
- Containers → after Memory (Phase 5)
- ModuleLoad → after Platform + Containers (Phase 5: Containers then ModuleLoad)

### Parallel Opportunities

- T003: All placeholder headers [P]
- T008, T013, T016: Declare thread, platform, log headers [P] within their phases
- T019, T022: Declare math, containers headers [P] in Phase 5
- T028: Docs [P]

---

## Implementation Strategy

### MVP First (US1 only)

1. Complete Phase 1 (Setup)
2. Complete Phase 2 (Memory)
3. Complete Phase 3 (Thread)
4. **STOP and VALIDATE**: Run test_alloc + test_thread; verify contract API only

### Incremental Delivery

1. Phase 1 + 2 → Memory ready
2. Phase 3 (US1) → Memory + Thread → validate
3. Phase 4 (US2) → Platform + Log → validate
4. Phase 5 (US3) → Math + Containers + ModuleLoad → validate
5. Phase 6 → Polish and contract compliance check

### Contract Compliance

- Every public symbol in `include/te/core/*.h` MUST appear in specs/_contracts/001-core-public-api.md (API 雏形 or 能力列表).
- Do not add new public types or functions beyond the contract.

---

## Notes

- [P] = different files, no dependency on other incomplete tasks in same phase
- [US1/US2/US3] = task belongs to that user story for traceability
- Commit after each task or logical group; use English for commit messages per .cursor/rules
- Run tests after each phase checkpoint
