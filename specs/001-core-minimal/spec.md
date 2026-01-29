# Feature Specification: 001-Core 最小切片（Alloc/Free、Log）

**Feature Branch**: `001-core-minimal`  
**Created**: 2026-01-29  
**Status**: Draft  

## 规约与契约引用

- **完整模块规约**：[docs/module-specs/001-core.md](../../docs/module-specs/001-core.md) — 001-Core 的完整功能、子模块、依赖与资源类型；本 spec 不重复其内容。
- **对外 API 契约**：[specs/_contracts/001-core-public-api.md](../_contracts/001-core-public-api.md) — 本模块对外类型与接口；实现时只暴露契约中已声明的类型与 API。

## 本切片范围

本 feature 仅实现 001-Core 的**最小可用子集**，范围限定为：

1. **内存**：分配与释放（Alloc/Free）— 支持指定大小与对齐的堆分配、显式释放；语义与生命周期见契约。
2. **日志**：分级日志（Log）— **本切片至少支持 Debug / Info / Warn / Error 四级**；通道即 stdout / stderr，可配置「某级别及以上→stderr、其余→stdout」。**不包含 Assert、CrashHandler**；不实现文件、重定向等，待 Platform 切片后再扩展。

其余子模块（Thread、Platform、Math、Containers、ModuleLoad 等）**不在本切片内**，后续切片实现。

## Clarifications

### Session 2026-01-29

- Q: 本切片中，Log 的「输出」应如何界定？ → A: 本切片 Log 仅输出到 stdout/stderr；不实现文件、重定向等，待 Platform 切片后再扩展。
- Q: Alloc(size=0 或非法 alignment)、Free(nullptr)、double-free 的约定？ → A: Alloc(0 或非法 alignment) 返回 nullptr；Free(nullptr) 与 double-free 均为 no-op（安全忽略）。
- Q: 本切片至少支持哪些 Log 级别？通道（stdout/stderr）与级别的对应？ → A: 至少支持 Debug / Info / Warn / Error 四级；通道即 stdout / stderr，可配置「某级别及以上→stderr、其余→stdout」。
- Q: 本切片 Log 是否含 Assert、CrashHandler？ → A: 本切片 Log 不包含 Assert、CrashHandler；仅分级、通道、过滤。Assert/CrashHandler 后续切片实现。
- Q: 本切片 Log 写入是否要求线程安全？ → A: 本切片 Log 写入要求线程安全；多线程并发写不丢、不乱序（单条消息原子）。

## User Scenarios & Testing *(mandatory)*

### User Story 1 - 堆内存分配与释放 (Priority: P1)

上游模块或主工程通过 Core 分配一块指定大小与对齐的内存，使用完毕后显式释放；分配失败或释放语义符合契约约定。

**Why this priority**: 内存抽象是所有下游模块的基础依赖。

**Independent Test**: 调用 Alloc 获得有效指针、按契约使用后调用 Free 释放；可独立验证无泄漏与重复释放行为。

**Acceptance Scenarios**:

1. **Given** 已初始化 Core，**When** 调用 Alloc(size, alignment)，**Then** 返回满足对齐的非空指针或契约定义的失败表示。
2. **Given** 由 Alloc 获得的合法指针，**When** 调用 Free(ptr)，**Then** 该块被释放，后续不再使用该指针。

---

### User Story 2 - 分级日志输出 (Priority: P2)

调用方按 **Debug / Info / Warn / Error** 写入日志；本切片输出仅 stdout/stderr，可配置「某级别及以上→stderr、其余→stdout」及级别过滤，行为符合契约与模块规约。

**Why this priority**: 日志为调试、运维与排查问题所必需。

**Independent Test**: 按四级写入日志，验证输出到达 stdout/stderr 且级别映射、过滤正确。

**Acceptance Scenarios**:

1. **Given** 已配置级别与 stdout/stderr 映射，**When** 按某级别写入日志，**Then** 消息到达对应通道（stdout 或 stderr）。
2. **Given** 级别过滤阈值，**When** 写入低于阈值的级别，**Then** 可被过滤（不输出）。

---

### Edge Cases

- **Alloc**：size=0 或 alignment 不合法（如非 2 的幂）时返回 nullptr。
- **Free**：Free(nullptr) 与 double-free 均为 no-op（安全忽略）；对非 Alloc 得来的指针 Free 的行为留待 plan/契约明确，本切片测试不覆盖。
- **Log**：本切片 Log 写入**要求线程安全**；多线程并发写不丢、不乱序（单条消息原子）。未初始化或已关闭时的行为留待 plan/契约明确。

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: 本切片 MUST 提供 Alloc(size, alignment) 与 Free(ptr)，语义、失败与生命周期见 [001-core-public-api.md](../_contracts/001-core-public-api.md)。Alloc(0 或非法 alignment) 返回 nullptr；Free(nullptr) 与 double-free 均为 no-op。
- **FR-002**: 本切片 MUST 提供分级 Log API；至少支持 Debug / Info / Warn / Error 四级，通道即 stdout/stderr，可配置「某级别及以上→stderr、其余→stdout」及过滤。Log 写入要求线程安全（多线程并发写不丢、不乱序）。不包含 Assert、CrashHandler；不实现文件或重定向。
- **FR-003**: 实现 MUST 只暴露 [001-core-public-api.md](../_contracts/001-core-public-api.md) 中已声明的类型与接口；不新增契约外的公开 API。
- **FR-004**: 行为与子模块边界 MUST 与 [001-core.md](../../docs/module-specs/001-core.md) 中 Memory、Log 描述一致，不重复实现完整规约中的其他子模块。

### Key Entities

- **内存块句柄/指针**：由 Alloc 分配、由 Free 释放；生命周期与所有权见契约。
- **日志级别与通道**：本切片至少 Debug / Info / Warn / Error；通道为 stdout / stderr，可配置级别→通道映射及过滤。见规约 Log 子模块与契约能力列表。

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: 调用方可仅依赖契约中的 Alloc/Free、Log 声明完成分配、释放与分级日志输出，无需依赖规约外的内部接口。
- **SC-002**: 通过针对 Alloc/Free、Log 的独立测试，覆盖契约规定的成功路径与边界行为（含分配失败、释放语义、级别过滤）。
- **SC-003**: 本切片的对外 API 与 [001-core-public-api.md](../_contracts/001-core-public-api.md) 的「API 雏形」保持一致；若有 plan 产出，须写回契约后再 tasks/implement。

## Interface Contracts *(multi-agent sync)*

- **本模块实现的契约**： [specs/_contracts/001-core-public-api.md](../_contracts/001-core-public-api.md)（本切片仅实现其中 Alloc/Free、Log 相关部分）。
- **本模块依赖的契约**：无。001-Core 为根模块，不依赖其他引擎模块。

## Dependencies

- **上游模块**：无。
- **假设与约束**：实现与主工程约定初始化/卸载顺序；只使用 [001-core-public-api.md](../_contracts/001-core-public-api.md) 及 [001-core.md](../../docs/module-specs/001-core.md) 中描述的接口与行为，不引入契约未声明的类型或 API。本切片 Log 不依赖 Platform；文件、重定向等待 Platform 切片后扩展。
