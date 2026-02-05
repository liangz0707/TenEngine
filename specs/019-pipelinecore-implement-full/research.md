# Research: 019-PipelineCore 完整模块实现

**Feature**: 019-pipelinecore-implement-full | **Date**: 2026-02-03

## 1. 技术选型与设计决策

### 1.1 错误处理策略

- **Decision**: PrepareRenderResources、ConvertToLogicalCommandBuffer 遇 RHI 失败时返回 `ResultCode`，由调用方（020-Pipeline）决定跳过/重试/中止。
- **Rationale**: 符合引擎分层，019 不越权做业务决策；与 009-RenderCore 的 ResultCode 模式一致。
- **Alternatives considered**: 静默跳过（违反可观测性）；立即中止（过于严格，剥夺调用方选择）。

### 1.2 FrameContext 与 ISceneWorld 定义归属

- **Decision**: 019 定义 FrameContext 最小 struct、ISceneWorld 最小接口；020 构造/实现并传入。
- **Rationale**: 019 不依赖 004-Scene，保持依赖图清晰；依赖倒置，020/004 实现 019 定义的接口。
- **Alternatives considered**: 019 仅前向声明 opaque（实现简单但契约不明确）；放在 009 共享（009 与 Scene 无关，不宜混入）。

### 1.3 MultiQueue / Async Compute 阶段

- **Decision**: 首版仅支持单队列（Graphics）；MultiQueue、async Compute 延后 Phase 2。
- **Rationale**: 先保证 Pass 图、命令缓冲、资源生命周期正确；Unreal RDG 的 async Compute 复杂度高，可后续迭代。
- **Alternatives considered**: 首版实现 MultiQueue（增加首版复杂度）；由 Plan 再定（spec 已澄清，无需推迟）。

### 1.4 Profiling 可观测性

- **Decision**: 开发版提供可选 profiling 钩子（Pass 开始/结束、Compile 耗时），由宏或配置控制。
- **Rationale**: 满足 Constitution §IV Performance & Observability；与 Unreal RDG Insights、Unity Profiler 理念一致。
- **Alternatives considered**: 不提供（违反 Constitution）；强制开启（影响发布版性能）。

### 1.5 IMaterialHandle / IMeshHandle

- **Decision**: 019 对 IMaterialHandle、IMeshHandle 做前向声明；020 在调用 PrepareRenderMaterial/PrepareRenderMesh 时传入 011-Material、012-Mesh 产出的句柄。019 不依赖 011/012；PrepareRenderMaterial、PrepareRenderMesh 可通过回调或扩展点委托 020 注入的实现。
- **Rationale**: 019 仅依赖 008、009；实际材质/网格准备由 020（依赖 011/012）在「线程 D」内调用 019 的 API 时，020 可先通过 011/012 完成准备，再让 019 做帧图侧的组织；或 019 提供 `SetMaterialPrepareCallback` 等扩展，020 注册 011/012 的实现。
- **Simplified approach for Phase 1**: PrepareRenderMaterial、PrepareRenderMesh 接收 opaque 指针；019 内部可 no-op 或调用 020 注册的回调；具体实现由 020 在集成时注入。ABI 保持 `IMaterialHandle const*`、`IMeshHandle const*` 为前向声明类型。

### 1.6 RDG 风格与 Unity/Unreal 参考

- **Unreal RDG** (Render Dependency Graph): Setup 与 Execute 两阶段；Pass 通过参数声明读写资源，自动推导依赖与生命周期；瞬态资源、Split-Barrier、并行录制、未使用剔除。参考：<https://dev.epicgames.com/documentation/en-us/unreal-engine/render-dependency-graph-in-unreal-engine>
- **Unity SRP RenderGraph**: AddRasterRenderPass、RenderGraphContext；Pass 内可配置 culling、render queue、target；声明式资源依赖。
- **本模块采纳**: Pass 资源声明与 009-RenderCore PassProtocol（DeclareRead/DeclareWrite、PassHandle、ResourceHandle）对接；资源生命周期与 RHI ResourceBarrier 协同；命令缓冲格式符合 pipeline-to-rci.md。

## 2. 上游契约要点

| 契约 | 关键类型/API |
|------|--------------|
| 008-rhi-public-api | IDevice、IQueue、ICommandList、IBuffer、ITexture、CreateBuffer/CreateTexture、ResourceBarrier、Submit、Begin/End、Draw/DrawIndexed、BeginRenderPass/EndRenderPass |
| 009-rendercore-public-api | PassHandle、ResourceHandle、ResourceLifetime、DeclareRead/DeclareWrite、PassResourceDecl、VertexFormat、TextureDesc、BufferDesc、IUniformLayout、ResultCode |
| pipeline-to-rci | 抽象命令缓冲格式、提交时机、资源状态约定 |

## 3. 命名空间与 Target

- **命名空间**: 参考上游 008-RHI（te::rhi）、009-RenderCore（te::rendercore），019 使用 **te::pipelinecore**，与上游风格一致。
- **头文件路径**: **te/pipelinecore/**（与 te/rhi/、te/rendercore/ 一致）。
- **CMake Target**: **te_pipelinecore**（与 te_rhi、te_rendercore 风格一致）。
