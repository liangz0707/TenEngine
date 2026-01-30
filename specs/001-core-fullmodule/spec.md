# Feature Specification: 001-Core 完整模块实现

**Feature Branch**: `001-core-fullmodule`  
**Created**: 2026-01-29  
**Status**: Draft  
**Input**: 本 feature 的完整模块规约见 `docs/module-specs/001-core.md`，契约见 `specs/_contracts/001-core-public-api.md`；**本 feature 实现完整模块内容**。

## 规约与契约引用 *(模块切片时必填)*

- **完整模块规约**：`docs/module-specs/001-core.md`（001-Core 基础层根模块：内存、线程、平台、日志、数学、容器、模块加载；不含反射与 ECS）。
- **本模块范围**（本 feature 实现完整模块内容）：
  1. **Memory**：Allocator 接口、DefaultAllocator、Alloc/Free、GetDefaultAllocator；对齐分配、double-free 为 no-op。
  2. **Engine**：Init/Shutdown、InitParams；进程级初始化与关闭。
  3. **Thread**：Thread、TLS、Atomic、Mutex、LockGuard、ConditionVariable、TaskQueue、IThreadPool、GetThreadPool、TaskCallback。
  4. **Platform**：TE_PLATFORM_* 宏、FileRead/FileWrite、DirectoryEnumerate、Time、HighResolutionTimer、GetEnv、PathNormalize。
  5. **Log**：LogLevel、LogSink、Log、LogSetLevelFilter/LogSetStderrThreshold/LogSetSink、Assert、CrashHandlerFn、SetCrashHandler。
  6. **Check**：CheckWarning、CheckError 宏。
  7. **Math**：Scalar、Vector2/3/4、Matrix3/4、Quaternion、AABB、Ray、Lerp、Dot、Cross、Length、Normalize。
  8. **Containers**：Array、Map、String、UniquePtr、SharedPtr。
  9. **ModuleLoad**：ModuleHandle、LoadLibrary、UnloadLibrary、GetSymbol、ModuleInitFn/ModuleShutdownFn、RegisterModuleInit/RegisterModuleShutdown、RunModuleInit/RunModuleShutdown。

实现时只使用 `specs/_contracts/001-core-public-api.md` 中已声明的类型与 API；不实现规约未列出的能力。**本模块无上游依赖**（根模块）；若有上游依赖，实现时只使用各上游契约已声明的类型与 API。

- **ABI 与构建规约**（`.specify/memory/constitution.md` §VI）：本模块须实现其 **ABI 文件**（`specs/_contracts/001-core-ABI.md`）中的**全部**符号与能力；构建须通过引入真实子模块源码满足依赖，**禁止**使用 stub、mock 或与契约不一致的代替实现作为长期方案。契约更新：接口变更须在 ABI 文件中更新完整条目。详见 `specs/_contracts/README.md`。

## User Scenarios & Testing *(mandatory)*

### User Story 1 - 进程启动与 Core 初始化 (Priority: P1)

主工程或上层模块在启动时调用 Core 的 Init，在退出前调用 Shutdown；期间使用内存、线程、平台、日志等能力。

**Why this priority**: 所有下游模块依赖 Core 先完成初始化。

**Independent Test**: 调用 Init 返回 true，再调用 Shutdown 不崩溃；可重复 Init（幂等）。

**Acceptance Scenarios**:

1. **Given** 进程启动，**When** 调用 `Init(nullptr)`，**Then** 返回 true（或可接受参数）。
2. **Given** 已 Init，**When** 再次调用 Init，**Then** 幂等且成功。
3. **Given** 已 Init，**When** 调用 Shutdown，**Then** 无崩溃；之后不再使用 Core 句柄。

---

### User Story 2 - 内存分配与默认分配器 (Priority: P1)

调用方通过全局 Alloc/Free 或 GetDefaultAllocator() 获取的 Allocator 进行分配与释放；size==0 或非法 alignment 返回 nullptr，Free(nullptr) 与 double-free 为 no-op。

**Why this priority**: 所有模块依赖内存能力。

**Independent Test**: Alloc(64,8) 返回非空、写后 Free 不崩溃；GetDefaultAllocator() 非空且 Alloc/Free 语义与全局一致。

**Acceptance Scenarios**:

1. **Given** 无前置条件，**When** Alloc(64, 8)，**Then** 返回非 nullptr；写后 Free 不崩溃。
2. **Given** 无前置条件，**When** GetDefaultAllocator()，**Then** 返回非空指针；通过该指针 Alloc/Free 行为与全局一致。
3. **Given** 已 Free 的指针，**When** 再次 Free，**Then** 不崩溃（no-op）。

---

### User Story 3 - 平台与文件/时间 (Priority: P2)

调用方使用平台宏、FileRead/FileWrite、DirectoryEnumerate、Time、HighResolutionTimer、GetEnv、PathNormalize。

**Why this priority**: 日志、资源加载等依赖平台能力。

**Independent Test**: 至少一个 TE_PLATFORM_* 为 1；FileWrite 后 FileRead 得到相同内容；Time/HighResolutionTimer 返回非负且单调。

**Acceptance Scenarios**:

1. **Given** 编译为某平台，**When** 检查 TE_PLATFORM_WINDOWS/LINUX/MACOS，**Then** 当前平台对应宏为 1。
2. **Given** 路径与数据，**When** FileWrite 再 FileRead，**Then** 内容一致（或失败有明确语义）。
3. **Given** 无前置条件，**When** 调用 Time() 与 HighResolutionTimer()，**Then** 返回非负；连续两次 HighResolutionTimer 单调不减。

---

### User Story 4 - 线程与线程池 (Priority: P2)

调用方创建 Thread、使用 Mutex/LockGuard/ConditionVariable、TaskQueue，或通过 GetThreadPool() 提交 TaskCallback。

**Why this priority**: 多线程与异步任务依赖线程能力。

**Independent Test**: GetThreadPool() 非空；SubmitTask 使回调在工作线程执行。

**Acceptance Scenarios**:

1. **Given** 无前置条件，**When** GetThreadPool()，**Then** 返回非空 IThreadPool*。
2. **Given** 线程池，**When** SubmitTask(callback, user_data)，**Then** callback 在某一工作线程被调用。

---

### User Story 5 - 日志与 Check (Priority: P2)

调用方使用 Log、LogSetLevelFilter/LogSetStderrThreshold/LogSetSink、Assert、SetCrashHandler；使用 CheckWarning/CheckError 宏在条件为假时记录或中止。

**Why this priority**: 调试与健壮性。

**Independent Test**: Log 各级别可输出；CheckWarning(false) 产生 Warning 日志；CheckError(false) 产生 Error 并可配置中止/返回。

**Acceptance Scenarios**:

1. **Given** 已 Init（若需要），**When** Log(Info, "msg")，**Then** 消息可被默认 Sink 输出。
2. **Given** 条件为假，**When** CheckWarning(condition, "message")，**Then** 记录 Warning。
3. **Given** 条件为假，**When** CheckError(condition, "message")，**Then** 记录 Error；按契约可中止或返回。

---

### User Story 6 - 数学与容器 (Priority: P2)

调用方使用 Vector2/3/4、Matrix3/4、Quaternion、AABB、Ray、Lerp、Dot、Cross、Length、Normalize；使用 Array、Map、String、UniquePtr、SharedPtr。

**Why this priority**: 下游模块（场景、实体、渲染等）依赖数学与容器。

**Independent Test**: 数学运算结果与预期一致；容器类型可构造、插入、访问。

**Acceptance Scenarios**:

1. **Given** 向量与标量，**When** 使用 Lerp、Dot、Length、Normalize，**Then** 结果符合数学定义。
2. **Given** 无前置条件，**When** 使用 Array/Map/String/UniquePtr/SharedPtr，**Then** 行为与契约（或 std 等价）一致。

---

### User Story 7 - 模块加载 (Priority: P3)

调用方使用 LoadLibrary、UnloadLibrary、GetSymbol；RegisterModuleInit/RegisterModuleShutdown、RunModuleInit/RunModuleShutdown。

**Why this priority**: 插件与动态加载依赖。

**Independent Test**: LoadLibrary 有效路径返回非空句柄；GetSymbol 可解析符号；RunModuleInit/RunModuleShutdown 按顺序调用已注册回调。

**Acceptance Scenarios**:

1. **Given** 有效动态库路径，**When** LoadLibrary(path)，**Then** 返回非 nullptr（或契约规定的失败语义）。
2. **Given** 有效句柄与符号名，**When** GetSymbol(handle, name)，**Then** 返回符号地址或 nullptr。
3. **Given** 已注册 ModuleInit/ModuleShutdown，**When** RunModuleInit 再 RunModuleShutdown，**Then** 按注册顺序执行。

---

### Edge Cases

- Alloc(0, 8)、Alloc(16, 3) 等非法参数返回 nullptr。
- Free(nullptr)、double-free 不崩溃。
- DirectoryEnumerate 失败返回空 vector；FileRead 失败返回 empty optional。
- Init 失败返回 false；Shutdown 在未 Init 或多次调用时的行为按契约（no-op 或单次有效）。

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: 模块 MUST 实现 `specs/_contracts/001-core-ABI.md` 中列出的全部符号与签名。
- **FR-002**: 模块 MUST 提供进程级 Init/Shutdown，且 Init 可幂等、Shutdown 在 Init 之后调用一次。
- **FR-003**: 内存 Alloc/Free 与 GetDefaultAllocator 的语义 MUST 符合契约（nullptr、double-free no-op、对齐）。
- **FR-004**: 平台宏、FileRead/FileWrite、DirectoryEnumerate、Time、HighResolutionTimer、GetEnv、PathNormalize MUST 符合契约。
- **FR-005**: Thread、TLS、Atomic、Mutex、LockGuard、ConditionVariable、TaskQueue、IThreadPool、GetThreadPool、TaskCallback MUST 符合契约。
- **FR-006**: Log、LogSetLevelFilter/LogSetStderrThreshold/LogSetSink、Assert、SetCrashHandler 与 CheckWarning/CheckError MUST 符合契约。
- **FR-007**: Math 类型与函数、Containers 类型 MUST 符合契约。
- **FR-008**: ModuleLoad 相关 API MUST 符合契约；RunModuleInit/RunModuleShutdown 按注册顺序/逆序调用。

### Key Entities

- **Allocator / 内存块**：由 Alloc/Free 或 Allocator 管理；生命周期至显式释放。
- **Engine 状态**：Init 后可用、Shutdown 后不再使用 Core 句柄。
- **线程与任务**：Thread、TaskQueue、IThreadPool、TaskCallback；任务在工作线程执行。
- **平台句柄与类型**：文件路径、DirEntry、时间值、环境变量、路径字符串。
- **模块句柄**：LoadLibrary 返回的 ModuleHandle；GetSymbol 解析的符号地址。

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**：CMake 配置与构建通过，无链接错误。
- **SC-002**：所有契约规定的单元测试（或等价验证）通过。
- **SC-003**：公开头文件与符号仅包含 `specs/_contracts/001-core-ABI.md` 与 `001-core-public-api.md` 中声明的类型与 API。
- **SC-004**：Init → 使用各子能力 → Shutdown 流程无崩溃，且行为符合规约与契约。

## Interface Contracts *(multi-agent sync)*

- **本模块实现的契约**：`specs/_contracts/001-core-public-api.md`（及 ABI 表 `001-core-ABI.md`）。
- **本模块依赖的契约**：无（001-Core 为根模块）。
- **ABI/构建**：须实现 ABI 中全部符号；构建不依赖其他引擎模块；禁止长期使用 stub 或代替方案。接口变更须在 ABI 文件中更新完整条目（constitution §VI、`specs/_contracts/README.md`）。

## Dependencies

- **上游模块**：无。001-Core 为根模块。
- **外部依赖**：C++17 标准库（如 `<cstdlib>`、`<thread>`、`<mutex>`、`<atomic>`、`<filesystem>`、`<chrono>`、`<functional>` 等）与平台 API（Win32 / POSIX / dyld）；实现时仅使用契约已声明的对外类型与 API，不额外暴露未在契约中声明的依赖。
