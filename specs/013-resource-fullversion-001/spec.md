# Feature Specification: 013-Resource 最小切片（ResourceId / LoadSync / Release）

**Feature Branch**: `013-resource-fullversion-001`  
**Created**: 2026-01-29  
**Status**: Draft  
**Input**: 本 feature 的完整模块规约见下方规约引用，对外 API 契约见下方契约引用；本 feature 仅实现其中最小子集，见下方显式枚举。

<!-- 当本 feature 为「某模块规约的切片」时，必须包含下方「规约与契约引用」节，与 /speckit.specify 输出一致。 -->
## 规约与契约引用 *(模块切片时必填)*

- **完整模块规约**：`docs/module-specs/013-resource.md`（Resource 模块：资源导入、同步/异步加载、卸载、流式与可寻址；依赖 Core、Object）。
- **对外 API 契约**：`specs/_contracts/013-resource-public-api.md`。
- **本切片范围（显式枚举）**：
  1. **ResourceId**：资源唯一标识；可寻址路径、GUID，与 Object 引用解析对接。
  2. **LoadSync**：同步加载 API（LoadSync）；加载请求、依赖解析仅限本切片所需最小集合。
  3. **Release**：释放/卸载（Release）；与各模块资源句柄协调、引用计数或显式释放。

实现时只使用 `specs/_contracts/001-core-public-api.md`、`specs/_contracts/002-object-public-api.md` 中已声明的类型与 API；不实现本规约未列出的能力（不实现 LoadAsync、Streaming、Addressing 全量、Import 等）。

## Clarifications

### Session 2026-01-29

- Q: 对同一 LoadHandle 多次调用 Release 时，本切片采用幂等还是明确错误？ → A: 幂等（第二次及以后视为成功/无操作）。
- Q: 无效或不可解析的 ResourceId 时，调用方如何得知 LoadSync 失败？ → A: 返回结果类型：成功时带 LoadHandle，失败时带错误码或原因，不抛异常。
- Q: SC-001 中「约定时间」是否在本切片量化？ → A: 本切片不量化，留到 plan 阶段再定。
- Q: 对同一 ResourceId 多次 LoadSync（未 Release）时期望语义？ → A: 每次 LoadSync 返回新的 LoadHandle（显式引用；多次调用得到多个句柄）。
- Q: 本切片 ResourceId 合法输入形式？ → A: 可寻址路径与 GUID 均支持。

## User Scenarios & Testing *(mandatory)*

### User Story 1 - 通过 ResourceId 标识并同步加载资源 (Priority: P1)

调用方持有资源可寻址路径或 GUID，需要同步获取可用的资源句柄以便使用；系统在 Core、Object 已初始化前提下，根据 ResourceId 解析并同步加载资源，返回 LoadHandle。

**Why this priority**: 同步加载是下游（如 Pipeline、Audio）使用资源的最小可用路径。

**Independent Test**: 给定合法 ResourceId，调用 LoadSync，在无 I/O 异常下得到有效 LoadHandle；可单独验收不依赖异步或流式。

**Acceptance Scenarios**:

1. **Given** Core 与 Object 已初始化、资源已注册/可解析，**When** 调用方传入合法 ResourceId 并调用 LoadSync，**Then** 返回有效 LoadHandle，且资源可被本进程使用。
2. **Given** 传入无效或不可解析的 ResourceId，**When** 调用 LoadSync，**Then** 返回结果类型表示失败（带错误码或原因），不抛异常、不崩溃。

---

### User Story 2 - 释放资源句柄 (Priority: P2)

调用方在不再需要资源时，通过 Release 释放 LoadHandle，系统更新引用计数或执行显式卸载，并与各模块资源句柄协调，避免悬空引用。

**Why this priority**: 释放是生命周期闭环与资源可控回收的前提。

**Independent Test**: 对同一 LoadHandle 先 LoadSync 再 Release，可验证引用/计数或资源可被回收；可独立于异步与流式测试。

**Acceptance Scenarios**:

1. **Given** 已通过 LoadSync 获得 LoadHandle，**When** 调用方调用 Release 传入该句柄，**Then** 句柄失效，资源可被回收且不产生悬空引用。
2. **Given** 对已释放或无效句柄再次调用 Release，**When** 调用 Release，**Then** 系统行为为幂等（第二次及以后无操作/成功），不崩溃。

---

### Edge Cases

- 对同一 ResourceId 多次 LoadSync 未 Release：每次返回新的 LoadHandle（显式引用）；多个句柄须各自 Release。
- LoadSync 期间依赖的上游（Core 文件/内存、Object 解析）不可用：系统以约定方式失败并清理，不泄漏句柄。
- Release 顺序与下游模块句柄释放顺序：满足契约中「各模块资源句柄的释放顺序须与 Resource 卸载策略协调」的约束。

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: 系统 MUST 提供 ResourceId 的表示与解析能力，与 `specs/_contracts/013-resource-public-api.md` 中 ResourceId 语义一致；本切片支持可寻址路径与 GUID 两种输入形式。
- **FR-002**: 系统 MUST 提供 LoadSync：给定 ResourceId，在依赖满足时同步加载并返回结果类型（成功时带 LoadHandle，失败时带错误码或原因）；失败不抛异常。对同一 ResourceId 多次调用 LoadSync 每次返回新的 LoadHandle（显式引用）。
- **FR-003**: 系统 MUST 提供 Release：给定 LoadHandle，释放资源并协调引用/卸载，满足契约中的调用顺序与约束；对同一句柄多次调用 Release 为幂等（第二次及以后视为成功/无操作）。
- **FR-004**: 实现 MUST 仅使用 001-Core、002-Object 契约中已声明的类型与 API，不引入未声明依赖。
- **FR-005**: 本切片 MUST 不实现 LoadAsync、Streaming、Addressing 全量、Import、GC、UnloadPolicy 等规约中未列入「本切片范围」的能力。

### Key Entities

- **ResourceId**：资源唯一标识；本切片支持可寻址路径与 GUID 两种形式，与 Object 引用解析对接（见契约类型表）。
- **LoadHandle**：同步加载得到的句柄；生命周期为请求发出至完成或取消（见契约类型表）。
- **Release**：对 LoadHandle 的释放操作；与契约中 Unload 能力中的 Release 一致，仅限本切片。

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: 调用方在给定合法 ResourceId 且上游可用时，能通过 LoadSync 得到有效 LoadHandle；具体时限/性能目标在 plan 阶段定义，本 spec 不量化。
- **SC-002**: 调用方对获得的 LoadHandle 调用 Release 后，资源可回收且无悬空引用（可通过测试或检查清单验证）。
- **SC-003**: 本切片实现可通过仅覆盖 ResourceId、LoadSync、Release 的验收场景通过，不依赖异步加载或流式接口。

## Interface Contracts *(multi-agent sync)*

- **本模块实现的契约**（若有）: 本 feature 实现 `specs/_contracts/013-resource-public-api.md` 中与本切片相关的部分（ResourceId、LoadSync、Release）。
- **本模块依赖的契约**: 见下方 Dependencies。

## Dependencies

- **001-Core**: `specs/_contracts/001-core-public-api.md`（文件、内存、平台 I/O 等）。
- **002-Object**: `specs/_contracts/002-object-public-api.md`（序列化、反射、GUID/引用解析）。
- 依赖关系总览：`specs/_contracts/000-module-dependency-map.md`。
