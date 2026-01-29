# Feature Specification: 001-Core Minimal Slice

**Feature Branch**: `001-core-minimal`  
**Created**: 2026-01-29  
**Status**: Draft  
**Input**: 本 feature 的完整模块规约见 `docs/module-specs/001-core.md`，对外 API 契约见 `specs/_contracts/001-core-public-api.md`。spec.md 中只描述本切片的范围（不重复写完整模块规约）。

## 规约与契约引用

- **完整模块规约**：`docs/module-specs/001-core.md`（基础层根模块：内存、线程、平台、日志、数学、容器、模块加载）。
- **对外 API 契约**：`specs/_contracts/001-core-public-api.md`。
- **本切片范围（显式枚举）**：
  1. **内存分配/释放**：分配器抽象最小能力（Alloc/Free、指定大小与对齐）；分配失败与释放语义明确；不包含池化、调试分配与泄漏追踪。
  2. **日志**：分级日志（LogLevel）、输出通道（LogSink）、断言（Assert）；可重定向与过滤；不包含崩溃报告钩子与完整平台抽象。

本 feature 不实现线程、平台、数学、容器、模块加载等规约与契约中未在本切片列出的能力。

## User Scenarios & Testing *(mandatory)*

### User Story 1 - 内存分配与释放 (Priority: P1)

作为引擎或模块作者，需要在运行时向系统申请与释放内存块（指定大小与对齐），以便为数据结构与缓冲提供存储。

**Why this priority**: 内存是根能力，无分配则无法构建容器、缓冲等下游依赖。

**Independent Test**: 调用 Alloc 分配一块内存、写入后调用 Free 释放；可验证无泄漏、对齐正确、分配失败时行为明确。

**Acceptance Scenarios**:

1. **Given** Core 已初始化（或本切片无显式初始化要求），**When** 调用 Alloc(size, alignment)，**Then** 返回可用内存块或明确表示失败（如 nullptr）。
2. **Given** 已分配的内存块，**When** 调用 Free(block)，**Then** 该块被释放且后续不再使用该句柄；重复释放行为有定义（如忽略或报错，由契约约定）。
3. **Given** 分配请求大小为 0 或对齐不合法，**Then** 行为有定义（拒绝或按契约约定）。

---

### User Story 2 - 日志与断言 (Priority: P2)

作为引擎或模块作者，需要输出分级日志并设置日志通道，以及在条件不满足时触发断言，便于调试与问题定位。

**Why this priority**: 日志与断言是开发期与运维期可观测性的最小依赖。

**Independent Test**: 设置 LogLevel/LogSink 后输出不同级别日志，验证仅符合级别的日志被输出；触发 Assert 条件时行为符合契约（如终止或回调）。

**Acceptance Scenarios**:

1. **Given** 已配置 LogSink 与 LogLevel，**When** 发出某级别的日志消息，**Then** 仅当级别满足配置时该消息被写入 LogSink；可重定向到控制台或文件（由契约约定）。
2. **Given** 断言条件为假，**When** 调用 Assert，**Then** 系统按契约约定行为（如终止、调用钩子或返回错误）；行为可重复验证。
3. **Given** 未配置 LogSink 或 LogLevel，**Then** 默认行为有定义（如丢弃或默认控制台）。

---

### Edge Cases

- 分配请求极大或对齐异常：应拒绝或返回失败，不导致未定义行为。
- 对已释放块再次 Free：行为有定义（如忽略、双释放检测）。
- 多线程下 Alloc/Free 与 Log 的并发语义：由契约约定（如线程安全或调用方保证单线程）。

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: 系统必须提供分配与释放接口（Alloc/Free 或契约等价能力）；支持指定大小与对齐；分配失败时返回明确结果（如 nullptr）。
- **FR-002**: 系统必须对 Free 的重复调用或非法句柄有定义行为（由契约约定）。
- **FR-003**: 系统必须提供分级日志与输出通道（LogLevel、LogSink）；支持重定向与过滤。
- **FR-004**: 系统必须提供断言能力（Assert）；条件为假时行为符合契约（如终止或回调）。
- **FR-005**: 本切片行为必须与 `specs/_contracts/001-core-public-api.md` 中能力列表、类型与句柄、调用顺序与约束一致（仅限内存与日志部分）。

### Key Entities

- **分配器 / 内存块句柄**：由 Alloc 返回的逻辑单元；生命周期至显式 Free。
- **LogLevel / LogSink**：日志级别与输出目标；可配置、可重定向。
- **Assert**：条件检查；失败时行为由契约约定。

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: 调用方能成功分配至少一块内存并在释放后无泄漏（通过简单测试或工具验证）。
- **SC-002**: 调用方能按配置的 LogLevel/LogSink 收到预期级别的日志输出。
- **SC-003**: 断言条件为假时，系统在可接受时间内按契约响应（如终止或回调），无未定义行为。
- **SC-004**: 本切片实现与 `docs/module-specs/001-core.md`、`specs/_contracts/001-core-public-api.md` 中对应部分一致；不超出本切片范围。

## Interface Contracts *(multi-agent sync)*

- **本模块实现的契约**：`specs/_contracts/001-core-public-api.md`（本 feature 仅实现其中「内存管理」与「日志」的最小子集，见本切片范围）。
- **本模块依赖的契约**：无（Core 为根模块）。

## Dependencies

- 无上游引擎模块；依赖关系总览见 `specs/_contracts/000-module-dependency-map.md`。
- 规约与契约：完整模块规约见 `docs/module-specs/001-core.md`，对外 API 契约见 `specs/_contracts/001-core-public-api.md`；本 feature 只实现最小子集并引用上述两文件，不重复全文。
