# Feature Specification: 019-PipelineCore 完整模块实现

**Feature Branch**: `019-pipelinecore-implement-full`  
**Created**: 2026-02-03  
**Status**: Draft  
**Input**: 本 feature 的完整模块规约见 docs/module-specs/019-pipeline-core.md，契约见 specs/_contracts/019-pipelinecore-public-api.md，并根据依赖模块的 ABI 与 API 文件以及 Unity、Unreal 官方文档推断本模块需要的补充内容；**本 feature 实现完整模块内容**。

## Clarifications

### Session 2026-02-03

- Q: 当 PrepareRenderResources 或 ConvertToLogicalCommandBuffer 遇到 RHI 失败（如 CreateBuffer 返回 nullptr）时，模块应如何表现？ → A: 返回错误码或 ResultCode，由调用方（020-Pipeline）决定跳过/重试/中止。
- Q: FrameContext 应由 019 定义还是由调用方提供的不透明类型？ → A: 019 定义最小 FrameContext 接口/struct（如 scene、camera、viewport 等），020 构造并传入。
- Q: 首版实现中 MultiQueue 与 async Compute 应如何处理？ → A: 首版仅支持单队列（Graphics）；MultiQueue 与 async Compute 延后 Phase 2。
- Q: ISceneWorld 应由谁定义？ → A: 019 定义最小 ISceneWorld 接口（如查询可见实体、场景根等），020/004 实现该接口。
- Q: 本模块是否需在开发版提供 Pass 级或 Compile 级 profiling？ → A: 提供可选 profiling 钩子（Pass 开始/结束、Compile 耗时等），由宏/配置控制。

## 规约与契约引用 *(模块切片时必填)*

- **完整模块规约**：`docs/module-specs/019-pipeline-core.md`（管线协议与命令缓冲约定：Pass 图协议 RDG 风格、资源生命周期、命令缓冲格式、与 RHI 提交约定）。
- **本模块范围**（本 feature 实现完整模块内容）：
  1. **PassGraph**：AddPass、DeclareRead、DeclareWrite、TopologicalSort、ExecuteOrder；RDG 风格 Pass 图与 RenderCore Pass 协议一致。
  2. **ResourceLifetime**：AllocateTransient、Alias、RingBuffer、ReleaseAfterPass；资源创建/使用/释放时机、与 RHI 屏障协同。
  3. **CommandFormat**：RecordPass、MapToRHI、InsertBarrier；命令缓冲抽象与 RHI 命令列表映射。
  4. **Submit**：SubmitQueue、SyncPoint、MultiQueue（可选）；提交时机与 RHI 队列、同步点约定。
  5. **FrameGraph / 流水线**：IFrameGraph、IPassBuilder、BuildLogicalPipeline、CollectRenderItemsParallel、MergeRenderItems、ILogicalCommandBuffer、PrepareRenderResources、ConvertToLogicalCommandBuffer；与 ABI 全部符号对齐。
  6. **渲染资源显式控制位置**：CreateRenderItem、CollectCommandBuffer（ConvertToLogicalCommandBuffer）、PrepareRenderMaterial、PrepareRenderMesh；提交到 RHI 见 pipeline-to-rci.md。

实现时只使用**本 feature 依赖的上游契约**（`specs/_contracts/008-rhi-public-api.md`、`specs/_contracts/009-rendercore-public-api.md`）中已声明的类型与 API；不实现本规约未列出的能力。

- **首版范围（Phase 1）**：单队列（Graphics）；MultiQueue、async Compute 调度延后 Phase 2。

- **ABI 与构建规约**（`.specify/memory/constitution.md` §VI）：本模块须实现其 **ABI 文件**（`specs/_contracts/019-pipelinecore-ABI.md`）中的**全部**符号与能力；构建须通过**引入真实子模块源码**（如 CMake `add_subdirectory`）满足依赖，**禁止**使用 stub、mock 或与契约不一致的代替实现作为长期方案。**契约更新**：接口变更须在对应 **ABI 文件**中增补或替换对应的 ABI 条目；下游所需接口须在**上游模块的 ABI 文件**中以 **TODO** 登记。详见 `specs/_contracts/README.md`「契约更新流程」。

- **第三方依赖**：第三方库引入说明在契约 `specs/_contracts/019-pipelinecore-public-api.md` 中声明；本 spec 引用该契约即可。Plan 从 public-api 读取并自动填入「第三方依赖」小节。详见 `docs/third_party-integration-workflow.md`。

### 依赖的上游 API 与补充内容

**上游契约**：本模块依赖的上游 API 以各上游契约为准；实现时**只使用**以下契约中已声明的类型与 API：

| 上游模块 | 契约文件 | 主要类型与能力 |
|----------|----------|----------------|
| 008-RHI | specs/_contracts/008-rhi-public-api.md、008-rhi-ABI.md | IDevice、IQueue、ICommandList、IBuffer、ITexture、ISampler、IPSO、IFence、ISemaphore；CreateBuffer/CreateTexture、ResourceBarrier、Submit、Begin/End、Draw/DrawIndexed/Dispatch；LoadOp/StoreOp、RenderPassDesc、BeginRenderPass/EndRenderPass |
| 009-RenderCore | specs/_contracts/009-rendercore-public-api.md、009-rendercore-ABI.md | PassResourceDecl、DeclareRead、DeclareWrite、PassHandle、ResourceHandle、ResourceLifetime；VertexFormat、IndexFormat、TextureDesc、BufferDesc；IUniformLayout、UniformBufferHandle；与 Pass 协议对接 |

**边界契约**：`specs/_contracts/pipeline-to-rci.md` 描述 020-Pipeline 产出抽象命令缓冲、008-RHI 消费与提交的约定；本模块产出的逻辑命令缓冲须符合该约定。

**Unity / Unreal 参考（补充推断）**：
- **Unreal RDG**：Setup 与 Execute 阶段；Pass 通过参数结构体声明对 RDG 资源的读写从而推导依赖与生命周期；异步 Compute 调度、瞬态资源分配、Split-Barrier 状态迁移、并行录制、未使用 Pass/资源剔除（参考 `docs/research/engine-reference-unity-unreal-modules.md` §1.3）。
- **Unity SRP RenderGraph**：AddRasterRenderPass、RenderGraphContext；Pass 内可配置 culling、render queue、target；声明式资源依赖。
- **本模块补充**：Pass 资源声明（DeclareRead/DeclareWrite）与 RenderCore PassProtocol 一致；资源生命周期与 RHI 屏障协同；命令缓冲最终通过 RHI Submit 提交，格式见 pipeline-to-rci。

## User Scenarios & Testing *(mandatory)*

### User Story 1 - FrameGraph 构建与 Pass 执行 (Priority: P1)

程序员通过 IFrameGraph::AddPass 添加渲染 Pass，配置场景、剔除方式、物体类型、渲染类型、输出目标，设置执行回调；调用 Compile 后管线按依赖顺序执行各 Pass，执行回调内通过 PassContext::GetCollectedObjects 拿到收集到的物体并录制 RHI 命令。

**Why this priority**: FrameGraph 是本模块核心能力，是 020-Pipeline 与 021-Effects 消费的基础。

**Independent Test**: 创建 FrameGraph、AddPass 若干、Compile，验证执行顺序与 PassContext 数据；可 Mock IRenderObjectList 验证回调被正确调用。

**Acceptance Scenarios**:

1. **Given** 已创建的 IFrameGraph，**When** 调用 AddPass("Opaque") 并配置 SetScene、SetCullMode、SetExecuteCallback，**Then** 返回有效 IPassBuilder 且配置被保存。
2. **Given** 已配置的 FrameGraph，**When** 调用 Compile()，**Then** 返回 true，且后续执行时 Pass 按拓扑顺序执行。
3. **Given** Pass 执行回调被调用，**When** 在回调内调用 ctx.GetCollectedObjects()，**Then** 返回本 Pass 收集到的物体列表（或空列表）。

---

### User Story 2 - 逻辑管线与多线程收集 RenderItem (Priority: P1)

线程 B 调用 BuildLogicalPipeline 产出逻辑管线；线程 C 调用 CollectRenderItemsParallel 多线程收集 RenderItem，再 MergeRenderItems 合并；线程 D 调用 PrepareRenderResources 准备 GPU 资源，ConvertToLogicalCommandBuffer 产出逻辑命令缓冲。

**Why this priority**: 多线程管线阶段（US-rendering-004）是流水线式渲染的基础，线程 B/C/D 分工须正确实现。

**Independent Test**: 提供 Mock FrameContext 与场景数据，验证 BuildLogicalPipeline、CollectRenderItemsParallel、MergeRenderItems 产出正确；在线程 D 验证 PrepareRenderResources 与 ConvertToLogicalCommandBuffer 仅使用 RHI/RenderCore 已声明接口。

**Acceptance Scenarios**:

1. **Given** IFrameGraph 与 FrameContext，**When** 调用 BuildLogicalPipeline(graph, ctx)，**Then** 返回 ILogicalPipeline 且含 Pass 列表与每 Pass 收集配置。
2. **Given** ILogicalPipeline 与 FrameContext，**When** 调用 CollectRenderItemsParallel 后 MergeRenderItems，**Then** 产出合并后的 IRenderItemList。
3. **Given** IRenderItemList 与 IDevice，**When** 在线程 D 调用 PrepareRenderResources(items, device)，**Then** 为 RenderItem 创建/准备 GPU 资源（仅使用 IDevice 已声明 API）。
4. **Given** IRenderItemList 与 ILogicalPipeline，**When** 在线程 D 调用 ConvertToLogicalCommandBuffer，**Then** 返回 ILogicalCommandBuffer，且格式符合 pipeline-to-rci 约定。

---

### User Story 3 - 流水线在途帧与 Slot 配置 (Priority: P2)

管线支持 2～4 帧在途；PipelineConfig 含 frameInFlightCount；FrameSlotId 用于访问每 slot 的资源；在复用 slot 前须等待该 slot 上 GPU 工作完成。

**Why this priority**: 流水线式多帧渲染（US-rendering-002）需要 slot 语义与同步点。

**Independent Test**: 验证 kMaxFramesInFlight、PipelineConfig、FrameSlotId 类型与常量存在且符合 ABI；与 RHI 的 getCommandListForSlot、waitForSlot 协同测试。

**Acceptance Scenarios**:

1. **Given** 模块已加载，**When** 引用 kMaxFramesInFlight 与 PipelineConfig，**Then** 类型与值符合 019-pipelinecore-ABI.md。
2. **Given** PipelineConfig.frameInFlightCount = 3，**When** 使用 FrameSlotId，**Then** 有效范围为 [0, 3)。

---

### User Story 4 - RDG 风格资源声明与生命周期 (Priority: P2)

Pass 通过 DeclareRead、DeclareWrite 声明对资源的读写；ResourceLifetime 支持 Transient、Alias、RingBuffer、ReleaseAfterPass；资源屏障与 RHI ResourceBarrier 协同。

**Why this priority**: RDG 风格资源管理是 Pass 图协议的核心，与 RenderCore PassProtocol 一致。

**Independent Test**: 验证 DeclareRead/DeclareWrite 与 RenderCore PassResourceDecl、ResourceHandle 对接；验证 AllocateTransient、Alias、ReleaseAfterPass 语义与 RHI 屏障插入正确。

**Acceptance Scenarios**:

1. **Given** PassHandle 与 ResourceHandle，**When** 调用 DeclareRead(pass, resource)，**Then** 依赖图中记录读边，Compile 后执行顺序正确。
2. **Given** 瞬态资源声明，**When** 执行 Pass 图，**Then** 资源在 Pass 使用后按 ReleaseAfterPass 释放，且屏障符合 RHI 要求。

---

### Edge Cases

- Pass 图含环时 Compile 应失败或返回 false。
- 空 FrameGraph 调用 Compile 应返回 true 且执行为空。
- 线程 D 外调用 PrepareRenderResources、ConvertToLogicalCommandBuffer 应文档化为未定义行为或返回错误。
- RHI 设备丢失或资源创建失败时，PrepareRenderResources、ConvertToLogicalCommandBuffer 须返回错误码或 ResultCode，由调用方（020-Pipeline）决定跳过/重试/中止；保持内部资源状态一致。

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**：模块 MUST 实现 `specs/_contracts/019-pipelinecore-ABI.md` 中全部符号与类型（IFrameGraph、IPassBuilder、CullMode、RenderType、PassOutputDesc、PassContext、PassExecuteCallback、IRenderObjectList、ILogicalPipeline、BuildLogicalPipeline、RenderItem、IRenderItemList、CollectRenderItemsParallel、MergeRenderItems、ILogicalCommandBuffer、PrepareRenderResources、PrepareRenderMaterial、PrepareRenderMesh、ConvertToLogicalCommandBuffer、kMaxFramesInFlight、PipelineConfig、FrameSlotId、CreateRenderItem）。
- **FR-002**：Pass 图 MUST 支持 AddPass、DeclareRead、DeclareWrite、TopologicalSort、ExecuteOrder；与 RenderCore PassResourceDecl、PassHandle、ResourceHandle 对接。
- **FR-003**：ResourceLifetime MUST 支持 AllocateTransient、Alias、RingBuffer、ReleaseAfterPass；资源屏障与 RHI ResourceBarrier 协同。
- **FR-004**：命令缓冲抽象 MUST 支持 RecordPass、MapToRHI、InsertBarrier；产出的 ILogicalCommandBuffer 格式符合 `pipeline-to-rci.md`。
- **FR-005**：Submit 相关约定 MUST 支持 SubmitQueue、SyncPoint；**首版仅支持单队列（Graphics）**，MultiQueue 与 async Compute 延后 Phase 2。
- **FR-006**：PrepareRenderResources、PrepareRenderMaterial、PrepareRenderMesh、ConvertToLogicalCommandBuffer MUST 仅在线程 D（GPU/Device 线程）调用；文档化线程约束。
- **FR-007**：实现 MUST 仅使用 008-rhi-public-api、009-rendercore-public-api 中已声明的类型与 API；不依赖内部或未文档化接口。
- **FR-008**：构建 MUST 通过引入真实 008-RHI、009-RenderCore 子模块源码满足依赖；禁止长期使用 stub 或 mock。
- **FR-009**：PrepareRenderResources、ConvertToLogicalCommandBuffer 遇 RHI 失败（如 CreateBuffer 返回 nullptr）时 MUST 返回错误码或 ResultCode，由调用方决定后续策略；不得静默忽略或未定义行为。
- **FR-010**：FrameContext MUST 由本模块定义最小接口/struct（scene、camera、viewport 等）；020-Pipeline 负责构造并传入 BuildLogicalPipeline、CollectRenderItemsParallel。
- **FR-011**：ISceneWorld MUST 由本模块定义最小接口（查询可见实体、场景根等）；020/004 实现该接口，供 IPassBuilder::SetScene 绑定；019 不依赖 004-Scene。
- **FR-012**：开发版 MUST 提供可选 profiling 钩子（Pass 开始/结束、Compile 耗时等），由宏或配置控制；满足 Constitution 可观测性要求。

### Key Entities

- **FrameContext**：本模块定义的最小接口/struct；含 scene、camera、viewport 等 BuildLogicalPipeline、CollectRenderItemsParallel 所需字段；020-Pipeline 构造并传入。
- **ISceneWorld**：本模块定义的最小接口；提供查询可见实体、场景根等；020/004 实现该接口，供 IPassBuilder::SetScene 绑定。
- **PassGraphBuilder / IFrameGraph**：Pass 图构建器；AddPass、Compile、拓扑排序与执行顺序。
- **PassContext**：Pass 执行上下文；GetCollectedObjects、与 RHI ICommandList 映射。
- **ResourceHandle**：管线内资源句柄；AllocateTransient、Alias、RingBuffer、ReleaseAfterPass。
- **SubmitContext**：提交上下文；SubmitQueue、SyncPoint、与 RHI 队列对接。
- **RenderItem / IRenderItemList**：可渲染项及列表；CollectRenderItemsParallel、MergeRenderItems 产出。
- **ILogicalCommandBuffer**：逻辑命令缓冲；ConvertToLogicalCommandBuffer 产出，符合 pipeline-to-rci。
- **ILogicalPipeline**：逻辑管线描述；BuildLogicalPipeline 产出，含 Pass 列表与收集配置。

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**：下游 020-Pipeline、021-Effects 可基于本模块 ABI 构建 FrameGraph、执行 Pass、产出逻辑命令缓冲并提交到 RHI，无需修改本模块内部实现。
- **SC-002**：所有 ABI 符号通过编译与链接；无缺失或 stub 占位（除上游 TODO 外）。
- **SC-003**：Pass 图编译与执行顺序与 DeclareRead/DeclareWrite 声明一致；资源生命周期无泄漏或提前释放。
- **SC-004**：线程 D 约束下的 PrepareRenderResources、ConvertToLogicalCommandBuffer 与 RHI Submit 协同正确；多帧流水线（2～4 帧在途）可正常工作。
- **SC-005**：开发版启用 profiling 时，可获取 Pass 执行耗时、Compile 耗时等指标，供性能分析使用。

## Interface Contracts *(multi-agent sync)*

- **本模块实现的契约**：`specs/_contracts/019-pipelinecore-public-api.md`
- **本模块依赖的契约**：`specs/_contracts/008-rhi-public-api.md`、`specs/_contracts/009-rendercore-public-api.md`；边界契约 `specs/_contracts/pipeline-to-rci.md`
- **ABI/构建**：须实现 ABI 中全部符号；构建须引入真实子模块代码，禁止长期使用 stub 或代替方案。接口变更须在 ABI 文件中更新完整条目；下游所需接口须在上游 ABI 中以 TODO 登记（constitution §VI、`specs/_contracts/README.md`）。

## Dependencies

- **008-RHI**：specs/_contracts/008-rhi-public-api.md、specs/_contracts/008-rhi-ABI.md
- **009-RenderCore**：specs/_contracts/009-rendercore-public-api.md、specs/_contracts/009-rendercore-ABI.md
- **边界**：specs/_contracts/pipeline-to-rci.md（命令缓冲格式与提交约定）
- **依赖总览**：specs/_contracts/000-module-dependency-map.md
