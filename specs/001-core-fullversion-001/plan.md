# Implementation Plan: 001-Core 完整功能集

**Branch**: `001-core-fullversion-001` | **Date**: 2026-01-29 | **Spec**: [spec.md](./spec.md)  
**Input**: Feature specification from `specs/001-core-fullversion-001/spec.md`  
**规约**: [docs/module-specs/001-core.md](../../docs/module-specs/001-core.md) | **契约**: [specs/_contracts/001-core-public-api.md](../_contracts/001-core-public-api.md)

## Summary

本 feature 实现 001-Core 的**完整功能集**（7 个子模块：Memory、Thread、Platform、Log、Math、Containers、ModuleLoad），技术栈 C++17、CMake；仅使用 C++ 标准库与平台 API（Win32/POSIX/dyld），不引入第三方库；仅暴露契约已声明的类型与 API。**内存与线程需进行封装与抽象**，使公共 API 与实现解耦，便于未来切换 Android、iOS 等平台时仅替换平台层实现。可衡量性能/规模目标由本 plan 产出并纳入验收。对外接口以 plan 末尾「契约更新」为准，写回契约「API 雏形」后实施。

## Technical Context

**Language/Version**: C++17  
**Build**: CMake 3.16+  
**Primary Dependencies**: 无第三方库；仅 C++ 标准库（&lt;cstdlib&gt;, &lt;thread&gt;, &lt;mutex&gt;, &lt;atomic&gt;, &lt;filesystem&gt;, &lt;cstdio&gt; 等）与平台 API（Win32 / POSIX / dyld）  
**Storage**: N/A（无持久化；文件 I/O 为 Platform 能力）  
**Testing**: 单元测试与契约测试覆盖 7 个子模块；可选用 Google Test 或手写  
**Target Platform**: Windows / Linux / macOS（当前）；**未来可扩展** Android、iOS 等，Memory 与 Thread 通过封装与抽象支持平台切换（仅替换底层实现，公共 API 不变）  
**Project Type**: 静态库或动态库，供主工程或下游模块链接  
**Performance Goals**: 由本 plan 阶段产出可衡量目标（如 Alloc 延迟、Log 吞吐、并发规模），纳入验收与测试  
**Constraints**: 不引入 jemalloc/spdlog/EASTL 等；仅暴露契约类型与 API；ABI 版本化 MAJOR.MINOR.PATCH  
**Scale/Scope**: 完整 7 子模块；子模块依赖见规约 5.3（Platform→Memory, Thread→Memory, Log→Platform, Containers→Memory, ModuleLoad→Platform+Containers）

## Constitution Check

| Principle | Status | Notes |
|-----------|--------|-------|
| VI. Module Boundaries & Contract-First | PASS | 仅暴露契约类型/API；契约来源 T0-contracts；本 plan 产出「契约更新」写回 API 雏形 |
| V. Versioning & ABI | PASS | 公开 API 按 MAJOR.MINOR.PATCH；破坏性变更递增 MAJOR |
| Technology: C++17, CMake | PASS | 符合 Constitution |
| Code Quality & Testing | PASS | 单元与契约测试；性能目标由 plan 产出并纳入验收 |
| Performance goals in specs/plans | PASS | 可衡量目标由本 plan 产出（spec 已澄清） |

## Project Structure

### Documentation (this feature)

```text
specs/001-core-fullversion-001/
├── plan.md
├── research.md
├── data-model.md
├── quickstart.md
├── contracts/
└── checklists/
```

### Source Code (repository root)

```text
include/te/core/
├── alloc.h           # Memory: Allocator, Alloc/Free, Pool, Debug/Leak
├── thread.h          # Thread, TLS, Atomic, Mutex, ConditionVariable, TaskQueue
├── platform.h        # File, Dir, Time, GetEnv, PathNormalize, 平台宏
├── log.h             # LogLevel, LogSink, Log, Assert, CrashHandler
├── math.h            # Vector2/3/4, Matrix3/4, Quaternion, AABB, Ray, Lerp
├── containers.h      # Array, Map, String, UniquePtr, SharedPtr
└── module_load.h     # LoadLibrary, UnloadLibrary, GetSymbol, Init/Shutdown

src/
├── alloc.cpp
├── thread.cpp
├── platform.cpp
├── log.cpp
├── math.cpp
├── containers.cpp
└── module_load.cpp

tests/
├── unit/
│   ├── test_alloc.cpp
│   ├── test_thread.cpp
│   ├── test_platform.cpp
│   ├── test_log.cpp
│   ├── test_math.cpp
│   ├── test_containers.cpp
│   └── test_module_load.cpp
└── CMakeLists.txt

CMakeLists.txt
```

**Structure Decision**: 单库、按子模块分头文件与实现；`include/te/core/` 对应契约公共 API。**Memory 与 Thread 采用平台抽象层**：公共头文件仅声明稳定接口，实现层按平台分支（如 `alloc_win32.cpp` / `alloc_posix.cpp` / 未来 `alloc_android.cpp`、`thread_win32.cpp` / `thread_pthread.cpp` / 未来 `thread_android.cpp`），便于未来切换 Android、iOS 时仅增平台实现而不改 API。实现顺序需满足子模块依赖（Platform/Memory 先行，Log 依赖 Platform，ModuleLoad 依赖 Platform+Containers）。

## Phase 0: Research Summary

见 [research.md](./research.md)。结论：全部使用 C++17 标准库与平台 API 封装；**Memory 与 Thread 做封装与抽象**，通过平台抽象层（当前 Win/Linux/macOS，未来可扩展 Android/iOS）使公共 API 稳定、仅底层实现可切换；Memory 用 aligned_alloc/_aligned_malloc + 可选池与调试层；Thread 用 std::thread、std::mutex 等（或未来平台原生 API 封装）；Platform 用 std::filesystem + Win32/POSIX/dyld；Log 用 std::mutex 保证线程安全；Math/Containers 自实现或 std 扩展；ModuleLoad 用 dlopen/LoadLibrary/dyld。性能目标在 plan 中给出量化指标。

## Phase 1: Data Model & Contracts

- **Data model**: 见 [data-model.md](./data-model.md)。实体覆盖契约「类型与句柄」：分配器/内存块、任务/作业、平台句柄、数学类型、容器类型、原子类型、模块句柄。
- **Contracts**: 本 feature 完整 API 见下方「契约更新」；占位见 [contracts/](./contracts/)。

## Quickstart

见 [quickstart.md](./quickstart.md)。要点：CMake 配置、构建、运行各子模块单元测试、示例调用与初始化/卸载顺序。

## Complexity Tracking

> 无 Constitution 违规需豁免。

---

## 契约更新（API 雏形）

以下内容可直接粘贴到 `specs/_contracts/001-core-public-api.md` 的「**API 雏形（简化声明）**」小节，作为本 feature（001-core-fullversion-001）对外暴露的完整类型与函数声明。与既有「本切片（001-core-minimal）产出」可合并或替换为本次完整雏形。

```markdown
### 本 feature（001-core-fullversion-001）完整 API 雏形

#### 1. 内存 (Memory)

- **Allocator** 接口（抽象基类）：`void* Alloc(size_t size, size_t alignment);`、`void Free(void* ptr);`
- `void* Alloc(size_t size, size_t alignment);` — 默认堆分配；失败返回 nullptr；size==0 或非法 alignment 返回 nullptr。
- `void Free(void* ptr);` — Free(nullptr) 与 double-free 为 no-op。
- **DefaultAllocator**、**AlignedAlloc**、**PoolAllocator**（可选）、**DebugAllocator/LeakTracker**（可选）：语义见契约能力 1。

#### 2. 线程 (Thread)

- **Thread**：创建/销毁、join/detach。
- **TLS**：线程局部存储。
- **Atomic&lt;T&gt;**：原子类型封装。
- **Mutex**、**ConditionVariable**：同步原语。
- **TaskQueue** 骨架：提交任务、执行与同步语义明确（见契约能力 2）。

#### 3. 平台 (Platform)

- **FileRead/Write**：文件读写接口。
- **DirectoryEnumerate**：目录枚举。
- **Time/HighResolutionTimer**：时间与高精度计时。
- **GetEnv**、**PathNormalize**：环境变量、路径规范化。
- 平台宏与检测（Windows/Linux/macOS）。

#### 4. 日志 (Log)

- **LogLevel** 枚举：Debug, Info, Warn, Error（及可扩展）。
- **LogSink**：输出通道抽象；可重定向与过滤。
- `void Log(LogLevel level, char const* message);` — 线程安全；单条消息原子。
- **Assert**、**CrashHandler**：断言与崩溃报告钩子。

#### 5. 数学 (Math)

- **Vector2/3/4**、**Matrix3/4**、**Quaternion**、**AABB**、**Ray**；**Lerp** 等插值与常用数学函数；无 GPU 依赖（见契约能力 5）。

#### 6. 容器 (Containers)

- **Array**、**Map**、**String**、**UniquePtr**、**SharedPtr**；可与自定义分配器配合；无反射、无 ECS（见契约能力 6）。

#### 7. 模块加载 (ModuleLoad)

- **LoadLibrary**、**UnloadLibrary**、**GetSymbol**：动态库加载/卸载/符号解析。
- 模块依赖顺序、**ModuleInit/Shutdown** 回调；与构建/插件系统配合（见契约能力 7）。

#### 调用顺序与约束

- 主工程须先完成 Core 初始化，再调用各子能力；卸载前释放所有由 Core 分配的资源并停止使用句柄。具体初始化/卸载顺序与 ABI 由实现与主工程约定。
```

**说明**：上述为简化声明与意图；具体函数签名、参数与返回值由实现与契约定稿时细化。本雏形与契约「能力列表」「类型与句柄」一致，写回契约后供下游与 tasks 使用。
