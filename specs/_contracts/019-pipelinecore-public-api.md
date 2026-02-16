# 契约：019-PipelineCore 模块对外 API

## 适用模块

- **实现方**：019-PipelineCore（L3；命令缓冲格式、Pass 图协议（RDG 风格）、与 RHI 提交约定）
- **对应规格**：`docs/module-specs/019-pipeline-core.md`
- **依赖**：008-RHI、009-RenderCore

## 消费者

- 020-Pipeline、021-Effects

## 能力列表

### 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| PassGraphBuilder | Pass 图构建器；AddPass、DeclareRead、DeclareWrite、TopologicalSort、ExecuteOrder | 单次图构建周期 |
| PassContext | Pass 执行上下文；GetCollectedObjects、GetRenderItemList(slot)、GetLightItemList；SetRenderItemList、SetLightItemList；与 RHI 命令列表映射 | 单次 Pass 执行 |
| ResourceHandle | 管线内资源句柄；AllocateTransient、Alias、RingBuffer、ReleaseAfterPass；kResourceHandleIdBackBuffer 表示 BackBuffer | 与 Pass 图资源生命周期一致 |
| PassKind / PassContentSource | Pass 类型（Scene/Light/PostProcess/Effect/Custom）与内容来源（FromModelComponent/FromLightComponent/FromPassDefined/Custom）；用于收集与执行分发 | 与 Pass 配置绑定 |
| PassAttachmentDesc / PassCollectConfig | 单 Attachment 描述（handle、format、loadOp/storeOp、sourcePass）；Pass 收集配置含 passKind、contentSource、colorAttachments、depthStencilAttachment | 与 Pass 绑定 |
| IRenderItemList / ILightItemList / ICameraItemList / IReflectionProbeItemList / IDecalItemList | 渲染项/灯光项/相机项/反射探针项/贴花项列表；Create/Destroy 工厂；020 收集后填入 PassContext | 单帧或单次收集 |
| SubmitContext | 提交上下文；SubmitQueue、SyncPoint、与 RHI 队列对接 | 单次提交周期 |
| CreateRenderItem / PrepareRenderMaterial / PrepareRenderMesh | 创建逻辑渲染资源、准备材质/网格；CollectCommandBuffer 产出逻辑命令缓冲 | 单帧或单次收集 |

命令缓冲最终通过 RHI 提交；格式与提交约定见 `pipeline-to-rci.md`。

### 能力（提供方保证）

| 序号 | 能力 | 说明 |
|------|------|------|
| 1 | PassGraph | AddPass(name)、AddPass(name, PassKind)；IScenePassBuilder、ILightPassBuilder、IPostProcessPassBuilder、IEffectPassBuilder 派生；DeclareRead、DeclareWrite、AddColorAttachment、SetDepthStencilAttachment；TopologicalSort、ExecuteOrder；RDG 风格与 RenderCore Pass 协议一致 |
| 2 | ResourceLifetime | AllocateTransient、Alias、RingBuffer、ReleaseAfterPass；与 RHI 屏障协同 |
| 3 | CommandFormat | RecordPass、MapToRHI、InsertBarrier；命令缓冲抽象与 RHI 命令列表映射 |
| 4 | Submit | SubmitQueue、SyncPoint、MultiQueue（可选）；与 RHI 队列、同步点约定 |

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。

## 约束

- 须在 RHI、RenderCore 初始化之后使用。Pass 资源声明须与 RenderCore PassProtocol 一致。资源屏障与生命周期不得违反 RHI 要求。

## TODO 列表

（以下任务来自原 ABI 数据相关 TODO。）

- [x] **数据**：RenderItem 含 mesh、material、sortKey；扩展 transform、bounds（RenderItemBounds）；IRenderItemList。
- [x] **接口**：PrepareRenderMaterial/PrepareRenderMesh 在线程 D 触发 011/012 EnsureDeviceResources；ConvertToLogicalCommandBuffer 按 (material, mesh, submeshIndex) 排序并合并 instanced draw；IFrameGraph::GetPassCount、ExecutePass；PassContext::SetCollectedObjects 由 020 按 Pass 填充。
- [ ] **接口**：CollectRenderItemsParallel 按 Pass 从 ISceneWorld 取数并填充 out（当前仅清空）；009 Bind/008 SetUniformBuffer 由 020 在每条 Draw 前按材质绑定。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 019-PipelineCore 契约 |
| 2026-02-05 | 统一目录；能力列表用表格；去除 ABI 引用 |
| 2026-02-10 | TODO 更新：PrepareRenderMaterial/Mesh、Convert 合批、ExecutePass、PassContext 已实现；CollectRenderItemsParallel 与 009 Bind 分工说明 |
| 2026-02-11 | FrameGraph 扩展：PassKind、PassContentSource、PassAttachmentDesc、派生 PassBuilder；PassContext 多 slot RenderItemList、LightItemList；ILightItemList、ICameraItemList、IReflectionProbeItemList、IDecalItemList 及 Create/Destroy；ILogicalPipeline::GetPassConfig(index, PassCollectConfig*) |
| 2026-02-17 | 新增提交相关函数：ExecuteLogicalCommandBufferOnDeviceThread（执行逻辑命令缓冲到 RHI）、SubmitLogicalCommandBuffer（提交到 RHI 队列）、PresentSwapChain（Present 交换链） |
