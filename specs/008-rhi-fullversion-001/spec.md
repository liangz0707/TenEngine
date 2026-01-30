# Feature Specification: 008-RHI 完整功能

**Feature Branch**: `008-rhi-fullversion-001`  
**Created**: 2026-01-29  
**Status**: Draft  
**Input**: 本 feature 的完整模块规约见上述规约，对外 API 契约见上述契约；本 feature 实现完整功能。

## 规约与契约引用

- **完整模块规约**：`docs/module-specs/008-rhi.md`（008-RHI 图形 API 抽象：命令列表、资源、PSO、多后端、同步等）。
- **对外 API 契约**：`specs/_contracts/008-rhi-public-api.md`。
- **本模块范围**：覆盖规约与契约所述全部子模块与能力，即 Device、CommandList、Resources、PSO、Synchronization；不包含其他模块。

实现时只使用 `specs/_contracts/001-core-public-api.md`（或本 feature 依赖的上游契约）中已声明的类型与 API；不实现本规约未列出的能力。

## User Scenarios & Testing *(mandatory)*

### User Story 1 - 创建设备、队列与后端选择 (Priority: P1)

上层（Application、Pipeline 等）需要初始化 RHI 时，能够创建图形设备、选择 Vulkan/D3D12/Metal 后端、获取队列，并得到可用于创建资源与录制命令的设备与队列句柄。

**Why this priority**: 设备与队列是一切 RHI 能力的前提。

**Independent Test**: 调用 CreateDevice、GetQueue、GetFeatures、SelectBackend；验证设备创建、队列获取与特性查询符合契约。

**Acceptance Scenarios**:

1. **Given** 支持的后端与环境，**When** 调用创建设备与获取队列接口，**Then** 返回有效 IDevice、队列句柄及可读特性信息。
2. **Given** 不支持的后端或不可用环境，**When** 请求创建设备，**Then** 有明确失败报告，不崩溃。

---

### User Story 2 - 录制并提交命令 (Priority: P1)

上层在获得设备与队列后，能够分配命令缓冲、录制 Draw/Dispatch/Copy、插入资源屏障，并提交到队列执行。

**Why this priority**: 命令列表是渲染与计算提交的核心路径。

**Independent Test**: Begin/End 命令缓冲、录制 Draw/Dispatch/Copy、ResourceBarrier、Submit；验证提交语义与契约一致。

**Acceptance Scenarios**:

1. **Given** 有效 IDevice 与队列，**When** 录制绘制/计算/拷贝命令并提交，**Then** 命令按约定顺序执行，无未定义行为。
2. **Given** 无效资源或状态，**When** 录制或提交，**Then** 有明确失败报告或约定语义。

---

### User Story 3 - 创建与管理资源 (Priority: P1)

上层能够创建 Buffer、Texture、Sampler 及视图（SRV/UAV/RTV/DSV 等概念），管理内存与生命周期，并在不再使用时显式销毁。

**Why this priority**: 资源是绘制与计算的基础数据载体。

**Independent Test**: CreateBuffer、CreateTexture、CreateSampler、CreateView、Destroy；验证创建成功、视图绑定与销毁顺序符合契约。

**Acceptance Scenarios**:

1. **Given** 有效 IDevice，**When** 创建缓冲、纹理、采样器与视图，**Then** 返回有效句柄，生命周期与契约一致。
2. **Given** 无效参数或资源不足，**When** 创建资源，**Then** 有明确失败报告。

---

### User Story 4 - 创建与绑定 PSO (Priority: P1)

上层能够创建图形/计算管线状态对象，与 Shader 字节码或模块绑定，可选缓存与编译，并与 RenderCore/Shader 模块对接。

**Why this priority**: PSO 决定渲染与计算管线配置，与 Shader 紧密耦合。

**Independent Test**: CreateGraphicsPSO、CreateComputePSO、SetShader、Cache；验证 PSO 创建、绑定与缓存行为符合契约。

**Acceptance Scenarios**:

1. **Given** 有效 Shader 字节码与配置，**When** 创建图形/计算 PSO 并绑定，**Then** 返回有效 IPSO，可用于命令录制。
2. **Given** 无效或缺失 Shader/配置，**When** 创建 PSO，**Then** 有明确失败报告。

---

### User Story 5 - 同步与多队列 (Priority: P1)

上层能够创建 Fence、Semaphore，使用资源屏障，并在多队列、跨帧场景下正确同步提交顺序与等待语义。

**Why this priority**: 同步是正确性与可预测性的保障。

**Independent Test**: CreateFence、Wait、Signal、ResourceBarrier；验证多队列提交顺序与等待语义符合契约。

**Acceptance Scenarios**:

1. **Given** 多队列与跨帧提交，**When** 使用 Fence/Semaphore 与屏障同步，**Then** 执行顺序与依赖关系符合约定，无竞态。
2. **Given** 设备丢失或运行时错误，**When** 上报并尝试恢复，**Then** 有明确报告，支持回退或重建，不导致引擎崩溃。

---

### Edge Cases

- 多线程下 RHI 接口的并发行为（由契约/实现定义并文档化）。
- 设备丢失、驱动升级或后端不可用时的错误路径与恢复策略。
- 资源销毁顺序违反底层 API 时的行为与报告。
- 平台无可用图形 API 时的错误路径与用户可理解报告。

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: 系统必须提供创建设备、获取队列、特性检测、后端选择（Vulkan/D3D12/Metal）的能力；多后端统一接口（契约能力 1）。
- **FR-002**: 系统必须提供命令列表的录制与提交：Begin/End、Draw、Dispatch、Copy、ResourceBarrier、Submit，语义明确（契约能力 2）。
- **FR-003**: 系统必须提供 Buffer、Texture、Sampler、视图的创建与销毁；内存管理与生命周期明确，失败时有明确报告（契约能力 3）。
- **FR-004**: 系统必须提供图形/计算 PSO 的创建、与 Shader 字节码/模块绑定、可选缓存与编译；与 RenderCore/Shader 对接（契约能力 4）。
- **FR-005**: 系统必须提供 Fence、Semaphore、资源屏障及多队列同步；提交顺序与等待语义明确（契约能力 5）。
- **FR-006**: 设备丢失或运行时错误可上报；支持回退或重建，不导致引擎崩溃（契约能力 6）。
- **FR-007**: RHI 接口在多线程访问下的行为由实现定义并文档化（契约能力 7）。
- **FR-008**: 实现仅依赖 001-Core 契约已声明的类型与 API；不暴露契约未声明的类型或接口。

### Key Entities

- **IDevice**：图形设备抽象；创建队列、资源、PSO；由 RHI 管理，创建后直至销毁（见契约）。
- **ICommandList**：命令缓冲；Draw/Dispatch/Copy、ResourceBarrier、Submit；单次录制周期内有效。
- **IBuffer / ITexture / ISampler**：缓冲、纹理、采样器；创建后直至显式销毁。
- **资源视图句柄**：描述符或视图 ID，用于绑定到 PSO；与资源生命周期一致。
- **IPSO**：管线状态对象（图形/计算）；与 Shader 字节码绑定；可缓存，创建后直至显式销毁。
- **Fence / Semaphore**：同步对象；多队列、跨帧同步；按实现约定。

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: 在支持的环境中，用户能通过指定后端成功创建设备、获取队列与资源，并完成命令录制与提交。
- **SC-002**: 资源创建、PSO 创建、命令提交与同步的调用结果与契约描述一致，无未定义行为。
- **SC-003**: 失败路径（不支持后端、不可用环境、资源不足、无效配置）有明确、可区分的报告，不导致引擎崩溃。
- **SC-004**: 多后端（Vulkan/D3D12/Metal）统一接口可被测试验证，行为与规约一致。

## Interface Contracts *(multi-agent sync)*

- **本模块实现的契约**：`specs/_contracts/008-rhi-public-api.md`（完整能力列表与类型与句柄）。
- **本模块依赖的契约**：见下方 Dependencies。

## Dependencies

- **001-Core**：`specs/_contracts/001-core-public-api.md`（内存、平台、日志等；实现时仅使用契约中已声明的类型与 API）。
- 依赖关系总览：`specs/_contracts/000-module-dependency-map.md`。
