# Feature Specification: 008-RHI 完整模块内容

**Feature Branch**: `008-rhi-fullmodule-004`  
**Created**: 2026-01-31  
**Status**: Draft  
**Input**: 本 feature 的完整模块规约见上述规约，契约见 specs/_contracts/008-rhi-public-api.md；**本 feature 实现完整模块内容**。

## 规约与契约引用 *(模块切片时必填)*

- **完整模块规约**：`docs/module-specs/008-rhi.md`（008-RHI 图形 API 抽象：命令列表、资源、PSO、多后端、同步等）。
- **对外 API 契约**：`specs/_contracts/008-rhi-public-api.md`。
- **本模块范围**（本 feature 实现完整模块内容）：
  1. **Device**：CreateDevice、GetQueue、GetFeatures、SelectBackend。
  2. **CommandList**：Begin/End、Draw、Dispatch、Copy、ResourceBarrier（细粒度：按资源+状态过渡）、Submit。
  3. **Resources**：CreateBuffer、CreateTexture、CreateSampler、CreateView、Destroy。
  4. **PSO**：CreateGraphicsPSO、CreateComputePSO、SetShader、Cache、DestroyPSO；接受后端原生 Shader 字节码（SPIR-V/DXIL/MSL），与 010-Shader 对接。
  5. **Synchronization**：CreateFence、CreateSemaphore、Wait、Signal、DestroyFence、DestroySemaphore、ResourceBarrier。
  6. **SwapChain/Present**：CreateSwapChain、Present、GetCurrentBackBuffer、Resize；与 020-Pipeline、027-XR 的边界以契约为准。
  7. **后端与调试**：实现 **Vulkan**、**D3D12**、**Metal** 三后端；集成 Vulkan Validation Layer、D3D12 Debug Layer 等，通过构建/运行时选项启用。

实现时只使用**本 feature 依赖的上游契约**（`specs/_contracts/001-core-public-api.md`）中已声明的类型与 API；不实现本规约未列出的能力。

- **ABI 与构建规约**（`.specify/memory/constitution.md` §VI）：本模块须实现其 **ABI 文件**（`specs/_contracts/008-rhi-ABI.md`）中的**全部**符号与能力；构建须通过**引入真实子模块源码**（如 CMake `add_subdirectory`）满足依赖，**禁止**使用 stub、mock 或与契约不一致的代替实现作为长期方案。接口变更须在对应 ABI 文件中更新；下游所需接口须在上游模块的 ABI 文件中以 TODO 登记。详见 `specs/_contracts/README.md`「契约更新流程」。

- **第三方依赖**：第三方库引入说明在契约 `specs/_contracts/008-rhi-public-api.md` 中声明（「第三方依赖」小节）；本 spec 引用该契约即可。Plan 从 public-api 读取并填入「第三方依赖」；Task 将生成版本选择、自动下载、配置、安装、编译测试、部署、配置实现等任务。

## User Scenarios & Testing *(mandatory)*

### User Story 1 - 创建设备、队列与后端选择 (Priority: P1)

上层（Application、Pipeline 等）需要初始化 RHI 时，能够创建图形设备、选择 Vulkan/D3D12/Metal 后端、获取队列，并得到可用于创建资源与录制命令的设备与队列句柄。

**Why this priority**: 设备与队列是一切 RHI 能力的前提。

**Independent Test**: 调用 CreateDevice、GetQueue、GetFeatures、SelectBackend；验证设备创建、队列获取与可读可验证的特性查询符合契约。

**Acceptance Scenarios**:

1. **Given** 支持的后端与环境，**When** 调用创建设备与获取队列接口，**Then** 返回有效 IDevice、队列句柄及可读、可验证的特性信息。
2. **Given** 不支持的后端或不可用环境，**When** 请求创建设备，**Then** 明确失败（错误码/返回值），不自动回退；回退与重试由上层负责，不崩溃。

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

- RHI 接口多线程行为由实现定义并文档化；本 feature 不要求默认并发安全。
- 设备丢失、驱动升级或后端不可用时的错误路径与恢复策略；后端不可用时仅明确失败，不自动回退。
- 资源销毁顺序违反底层 API 时的行为与报告。
- 平台无可用图形 API 时的错误路径与用户可理解报告。
- ResourceBarrier 使用细粒度接口；上层须正确声明资源状态 OldState→NewState。
- Validation/Debug Layer 可通过构建或运行时选项启用；启用时用于辅助发现错误，不影响发布构建。

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: 系统必须提供创建设备、获取队列、特性检测、后端选择（**Vulkan**、**D3D12**、**Metal** 全部三个后端）的能力；多后端统一接口。CreateDevice 不依赖窗口/上下文；后端不可用时仅明确失败，不自动回退。
- **FR-002**: 系统必须提供命令列表的录制与提交：Begin/End、Draw、Dispatch、Copy、**ResourceBarrier（细粒度：按资源+状态过渡）**、Submit，语义明确。
- **FR-003**: 系统必须提供 Buffer、Texture、Sampler、视图的创建与销毁；内存管理与生命周期明确，失败时有明确报告。
- **FR-004**: 系统必须提供图形/计算 PSO 的创建、与**后端原生 Shader 字节码**（SPIR-V/DXIL/MSL）绑定、可选缓存与编译；与 RenderCore/Shader 对接。
- **FR-005**: 系统必须提供 Fence、Semaphore、资源屏障及多队列同步；提交顺序与等待语义明确。
- **FR-006**: 设备丢失或运行时错误可上报；支持回退或重建，不导致引擎崩溃。
- **FR-007**: RHI 接口在多线程访问下的行为由实现定义并文档化；本 feature 不要求默认并发安全。
- **FR-008**: 实现仅依赖 001-Core 契约（`specs/_contracts/001-core-public-api.md`）已声明的类型与 API；不暴露契约未声明的类型或接口。
- **FR-009**: 本 feature 包含 **SwapChain、Present**（及可选 XR 对接点）；与 020-Pipeline、027-XR 的边界以契约为准。
- **FR-010**: 本 feature 集成 **Vulkan Validation Layer、D3D12 Debug Layer** 等；通过构建/运行时选项启用，发布构建可关闭。

### Key Entities

- **IDevice**：图形设备抽象；创建队列、资源、PSO；生命周期直至 DestroyDevice。
- **IQueue**：队列句柄；由 GetQueue 返回，非拥有。
- **ICommandList**：命令缓冲；单次录制周期内有效。
- **IBuffer / ITexture / ISampler**：GPU 资源；创建后直至显式 Destroy。
- **IPSO**：管线状态对象；与后端原生 Shader 字节码绑定，可缓存。
- **IFence / ISemaphore**：同步对象；多队列、跨帧同步。
- **ViewHandle**：资源视图句柄，用于绑定到 PSO。

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**：调用 CreateDevice、GetQueue、GetFeatures 后，能获得有效句柄与可读特性信息。
- **SC-002**：录制 Draw/Dispatch/Copy 并 Submit 后，命令按约定语义执行，无未定义行为。
- **SC-003**：CreateBuffer/CreateTexture/CreateSampler 成功返回有效句柄；Destroy 后资源正确释放。
- **SC-004**：CreateGraphicsPSO/CreateComputePSO 成功返回有效 IPSO；SetShader、Cache 行为符合契约。
- **SC-005**：Fence/Semaphore 与 ResourceBarrier 在多队列场景下正确同步，无竞态。

## Interface Contracts *(multi-agent sync)*

- **本模块实现的契约**：`specs/_contracts/008-rhi-public-api.md`（完整能力列表与类型与句柄）。
- **本模块依赖的契约**：见下方 Dependencies。
- **ABI/构建**：须实现 `specs/_contracts/008-rhi-ABI.md` 中的全部符号；构建须引入真实子模块代码，禁止长期使用 stub 或代替方案。接口变更须在 ABI 文件中更新完整条目；下游所需接口须在上游 ABI 中以 TODO 登记（constitution §VI、`specs/_contracts/README.md`）。

## Dependencies

- **001-Core**：`specs/_contracts/001-core-public-api.md`（内存、平台、日志等）。**依赖的上游 API 见该契约**；实现时只使用契约已声明的类型与 API。
- 依赖关系总览：`specs/_contracts/000-module-dependency-map.md`。
