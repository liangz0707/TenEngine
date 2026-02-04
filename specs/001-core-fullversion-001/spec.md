# Feature Specification: 001-Core 完整功能集

**Feature Branch**: `001-core-fullversion-001`  
**Created**: 2026-01-29  
**Status**: Draft  

## 规约与契约引用

- **完整模块规约**：[docs/module-specs/001-core.md](../../docs/module-specs/001-core.md) — 001-Core 的完整功能、子模块、依赖与资源类型；本 spec 不重复其内容。
- **对外 API 契约**：[specs/_contracts/001-core-public-api.md](../_contracts/001-core-public-api.md) — 本模块对外类型与接口；实现时只暴露契约中已声明的类型与 API。

## 本切片范围

本 feature 实现 001-Core 的**完整功能集**，即规约与契约中定义的全部能力，范围覆盖以下子模块：

1. **Memory**：分配器抽象、默认堆分配、对齐分配、内存池、调试分配与泄漏追踪。
2. **Thread**：线程创建与管理、TLS、原子类型、Mutex/ConditionVariable、任务队列骨架。
3. **Platform**：文件 I/O、目录枚举、时间与高精度计时、环境变量、路径规范化、平台检测（Windows/Linux/macOS 等）。
4. **Log**：分级日志、输出通道、断言、崩溃报告钩子。
5. **Math**：标量/向量/矩阵/四元数、AABB、射线、插值及常用数学函数（与渲染无关的纯数学）。
6. **Containers**：动态数组、哈希表、字符串、智能指针（无反射、无 ECS）。
7. **ModuleLoad**：动态库加载/卸载/符号解析、模块依赖顺序、初始化与关闭回调。

实现时只使用 [001-core-public-api.md](../_contracts/001-core-public-api.md) 及上游无依赖（根模块）；不实现规约与契约未列出的能力。

## Clarifications

### Session 2026-01-29

- Q: 本 feature 是否允许引入规约中的可选外部库（如 jemalloc、spdlog、EASTL）？ → A: 不允许；本 feature 仅使用 C++ 标准库与平台 API（Win32/POSIX/dyld），不引入第三方库；可选库留待后续迭代。
- Q: 本 feature 的可衡量性能/规模目标应在 spec 中补充还是留待 plan？ → A: 不在本 spec 中写具体数值；可衡量性能/规模目标由 plan 阶段产出，并纳入验收与测试。

## User Scenarios & Testing *(mandatory)*

### User Story 1 - 内存与线程基础 (Priority: P1)

下游模块或主工程通过 Core 进行堆/对齐分配与释放、创建线程与同步；行为符合契约能力列表 1、2。

**Why this priority**: 内存与线程是所有上层模块的基础。

**Independent Test**: 调用 Alloc/Free、线程创建与 Mutex 同步，验证契约规定的语义与生命周期。

**Acceptance Scenarios**:

1. **Given** 已初始化 Core，**When** 使用 Memory 与 Thread 能力，**Then** 行为符合契约与规约。
2. **Given** 多线程场景，**When** 使用同步原语，**Then** 无数据竞争、语义明确。

---

### User Story 2 - 平台与日志 (Priority: P2)

调用方使用文件 I/O、时间、环境与路径、分级日志与断言；与具体 OS 解耦、可重定向与过滤。

**Why this priority**: 平台抽象与日志为跨平台与运维所必需。

**Independent Test**: 文件读写、目录枚举、时间/计时、Log 级别与通道、Assert；验证契约能力 3、4。

**Acceptance Scenarios**:

1. **Given** 目标平台（Win/Linux/macOS），**When** 调用 Platform 与 Log API，**Then** 行为一致、可测。
2. **Given** 日志级别与通道配置，**When** 写入日志，**Then** 按契约可重定向与过滤。

---

### User Story 3 - 数学、容器与模块加载 (Priority: P3)

调用方使用数学类型、容器、智能指针及动态库加载/卸载/符号解析；无 GPU 依赖、可与分配器配合。

**Why this priority**: 数学与容器为引擎通用；模块加载为插件与构建集成所需。

**Independent Test**: 向量/矩阵/四元数、Array/Map/String、UniquePtr/SharedPtr、LoadLibrary/GetSymbol；验证契约能力 5、6、7。

**Acceptance Scenarios**:

1. **Given** 数学与容器 API，**When** 按契约使用，**Then** 无反射/ECS、可指定分配器。
2. **Given** 动态库路径，**When** Load/Unload/GetSymbol 及初始化回调，**Then** 依赖顺序与生命周期符合契约。

---

### Edge Cases

- 分配失败、重复释放、跨平台差异（文件路径、时间精度）按契约与规约定义处理。
- 未初始化或已关闭时调用各子能力的边界行为；多线程与模块加载顺序约束见契约「调用顺序与约束」。

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: 本 feature MUST 实现规约 [001-core.md](../../docs/module-specs/001-core.md) 中全部 7 个子模块（Memory、Thread、Platform、Log、Math、Containers、ModuleLoad），行为与 [001-core-public-api.md](../_contracts/001-core-public-api.md) 能力列表一致。
- **FR-002**: 实现 MUST 只暴露契约中已声明的类型与接口；不新增契约外的公开 API。
- **FR-003**: 主工程或上层模块须先完成 Core 初始化再调用各子能力；卸载前释放资源并停止使用句柄，符合契约「调用顺序与约束」。
- **FR-004**: 公开 API 版本化（MAJOR.MINOR.PATCH）、ABI 与契约一致；破坏性变更递增 MAJOR 并附迁移说明。

### Key Entities

- 规约与契约中定义的类型与句柄：分配器/内存块、任务/作业、平台句柄、数学类型、容器类型、原子类型、模块句柄；生命周期与语义见契约。

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: 调用方可仅依赖 [001-core-public-api.md](../_contracts/001-core-public-api.md) 完成全部 7 类能力的调用，无需依赖未声明接口。
- **SC-002**: 通过针对各子模块的单元与集成测试，覆盖契约规定的成功路径与边界行为。
- **SC-003**: 对外 API 与契约「API 雏形」及后续定稿一致；实现与主工程约定的初始化/卸载顺序由功能测试验证。
- **SC-004**: 可衡量的性能与规模目标由 plan 阶段产出，并纳入验收与测试（本 spec 不预先规定具体数值）。

## Interface Contracts *(multi-agent sync)*

- **本模块实现的契约**：[specs/_contracts/001-core-public-api.md](../_contracts/001-core-public-api.md)（本 feature 实现完整能力集）。
- **本模块依赖的契约**：无。001-Core 为根模块。

## Dependencies

- **上游模块**：无。
- **假设与约束**：实现与主工程约定初始化/卸载顺序；只使用 [001-core-public-api.md](../_contracts/001-core-public-api.md) 及 [001-core.md](../../docs/module-specs/001-core.md) 中描述的类型与行为；技术栈 C++17、CMake。**本 feature 不引入第三方或规约中的可选外部库**（如 jemalloc、spdlog、EASTL），仅使用 C++ 标准库与平台 API（Win32/POSIX/dyld）；可选库留待后续迭代。
