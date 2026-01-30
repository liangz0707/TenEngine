# US-rendering-004：程序逻辑在多线程上执行，主循环/游戏更新/构建管线/多线程收集 RenderItem/逻辑 CommandBuffer/Device 线程提交

- **标题**：程序逻辑可在多线程上执行：主线程执行主循环；线程 A 执行所有游戏更新逻辑；线程 B 触发一次渲染并先构建逻辑渲染流程（pipeline、pass 等逻辑数据）；线程 C 多线程并行收集所有要渲染的内容（场景模型、UI 等），组织成 RenderItem，并行结束后合并；**线程 D** 在**同一线程**内执行：渲染物体的**渲染资源**创建与准备、所有**图形 GPU 操作**与 **GPU 资源创建**、将 RenderItem 转成 CommandBuffer 并录制真实 CommandBuffer、提交。
- **编号**：US-rendering-004

---

## 1. 角色/触发

- **角色**：引擎调度层或程序员
- **触发**：希望**程序逻辑在多线程上执行**：主线程只跑主循环；**游戏更新**在单独线程（线程 A）；**单帧渲染**由某线程（线程 B）触发，并拆成多阶段：构建逻辑管线（B）→ 多线程收集 RenderItem（C）→ 合并；**线程 D** 为**唯一的 GPU/Device 线程**：**所有图形 GPU 操作、GPU 资源创建**（含渲染物体的渲染资源）必须在**线程 D** 执行，并在同一线程内完成 CommandBuffer 录制与提交。

---

## 2. 端到端流程（线程角色）

| 线程 | 职责 | 输入 | 输出 |
|------|------|------|------|
| **主线程** | 主循环：轮询事件、驱动帧节奏、协调各线程 | 窗口事件、帧间隔 | 每帧触发「游戏更新」与「渲染触发」的调度 |
| **线程 A** | 执行所有**游戏更新逻辑**（脚本、物理、动画等） | 本帧 delta_time、场景/实体快照 | 更新后的场景/实体数据；与渲染读端同步点 |
| **线程 B** | **触发一次渲染**；先**构建逻辑渲染流程**（pipeline、pass 等**逻辑数据**，不录 GPU 命令） | FrameContext、FrameGraph 配置 | 本帧的**逻辑管线描述**（LogicalPipeline 或已编译的 Pass 列表），供后续阶段消费 |
| **线程 C** | **多线程并行**收集所有要渲染的内容（场景模型、UI、特效等），组织成 **RenderItem**；并行结束后**合并** | 逻辑管线描述、场景/实体、相机 | 每 Pass 或全局的 **RenderItem 列表**（合并后） |
| **线程 D（GPU/Device 线程）** | **所有图形 GPU 操作、GPU 资源创建**必须在本线程执行。包括：(1) **渲染物体的渲染资源**的创建与准备（纹理、缓冲、PSO 等）；(2) 将 **RenderItem** 转成逻辑/真实 CommandBuffer；(3) 录制真实 **ICommandList**；(4) **提交** CommandBuffer。不在其他线程调用 IDevice/GPU 创建或录制。 | RenderItem 列表、Pass 输出描述、IDevice | 渲染资源就绪、真实 ICommandList 录制完成并 submit；可选 Present |

**约束**：**所有 GPU 资源创建、所有图形 GPU 操作**（含渲染物体所需的渲染资源）**必须在线程 D 执行**；线程 D 即「Device 线程」或「RHI 线程」，与主线程、线程 A/B/C 分离，保证单线程访问 GPU 上下文。

**同步**：主线程或调度层在适当时机同步（如 A 完成后 B 开始、C 合并后 D 开始）；或通过任务图/Job 依赖表达。具体线程与物理线程的对应由实现决定（可复用线程池）。

---

## 3. 涉及模块

| 模块 | 职责摘要 |
|------|----------|
| 003-Application | 主循环；可选：将「游戏更新」与「渲染触发」派发到线程 A / 线程 B（或由上层调度） |
| 019-PipelineCore | 逻辑管线描述、RenderItem 类型、多线程收集与合并、ILogicalCommandBuffer；buildLogicalPipeline（线程 B）、collectRenderItemsParallel（线程 C）、merge；线程 D 内：渲染资源准备、convertToLogicalCommandBuffer 或直接录 CommandBuffer |
| 020-Pipeline | 触发渲染、协调阶段 B/C/D；持有 FrameGraph、调用 buildLogicalPipeline、驱动 collect/merge；在线程 D 内驱动渲染资源创建、CommandBuffer 录制与提交 |
| 008-RHI | **线程 D（GPU/Device 线程）**：所有 IDevice 调用（GPU 资源创建、CommandBuffer 录制、submit）必须在线程 D 执行；渲染物体的渲染资源（纹理、缓冲、PSO 等）创建也在线程 D |

---

## 4. 每模块职责与 I/O

### 019-PipelineCore

- **职责**：提供 **buildLogicalPipeline**（线程 B）：根据 FrameGraph 与 FrameContext 产出**逻辑管线描述**（Pass 列表、每 Pass 的收集配置与输出）；提供 **collectRenderItemsParallel**（线程 C）：按逻辑管线描述，**多线程**遍历场景/UI 等，产出 **RenderItem** 列表，再 **mergeRenderItems** 合并；提供 **RenderItem** 结构（模型、UI、材质、排序 key 等）。**线程 D** 内：提供 **prepareRenderResources**（为 RenderItem 创建/准备 GPU 渲染资源，如纹理、缓冲、PSO）及 **convertToLogicalCommandBuffer** 或将 RenderItem 直接录成 CommandBuffer；上述逻辑均在**线程 D** 执行，与 RHI 的 GPU 操作同一线程。
- **输入**：FrameContext、IFrameGraph（或已编译图）；场景/实体/UI 数据（只读）；线程 D 内使用 IDevice 创建资源。
- **输出**：ILogicalPipeline、RenderItem、IRenderItemList、collectRenderItemsParallel、mergeRenderItems；线程 D 内：prepareRenderResources、ILogicalCommandBuffer、convertToLogicalCommandBuffer。

### 020-Pipeline

- **职责**：**触发一次渲染**时（可由线程 B 或主线程调用）：调用 **buildLogicalPipeline**（线程 B）；调度 **collectRenderItemsParallel**（线程 C）并 **mergeRenderItems**；将**线程 D** 作为 **GPU/Device 线程**调度：在**线程 D** 内依次执行**渲染资源创建**（为 RenderItem 准备 GPU 资源）、**convertToLogicalCommandBuffer**（或直接录制）、将逻辑/真实 CommandBuffer 交给 RHI 在**同一线程 D** 录制并 submit；协调同步点或任务依赖。
- **输入**：FrameContext；FrameGraph；各阶段产出。
- **输出**：triggerRender(ctx) 或 tickPipeline 内部分阶段；在线程 D 内调用 submitLogicalCommandBuffer 或等效，且所有 GPU 相关调用均在线程 D。

### 008-RHI

- **职责**：**线程 D = GPU/Device 线程**。**所有图形 GPU 操作、GPU 资源创建**（含 IDevice、ICommandList、纹理/缓冲/PSO 创建、CommandBuffer 录制、submit）**必须在线程 D 执行**；渲染物体的渲染资源（纹理、缓冲、PSO 等）创建也必须在线程 D。提供 **executeLogicalCommandBuffer** 等在**线程 D** 上将逻辑 CommandBuffer 录成真实 ICommandList 并 submit；禁止在其他线程调用 IDevice 的创建或录制接口。
- **输入**：ILogicalCommandBuffer 或 RenderItem 相关数据（由 Pipeline 在线程 D 传入）；IDevice、ICommandList。
- **输出**：IDevice::executeLogicalCommandBuffer(ILogicalCommandBuffer) 等；约定：**所有 IDevice 接口的调用线程 = 线程 D**。

---

## 5. 派生接口（ABI 条目）

以下按 `docs/engine-abi-interface-generation-spec.md` 书写。

### 019-PipelineCore

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 019-PipelineCore | TenEngine::pipelinecore | ILogicalPipeline | 抽象/struct | 逻辑管线描述 | TenEngine/pipelinecore/LogicalPipeline.h | ILogicalPipeline | 由 buildLogicalPipeline 产出；含 Pass 列表、每 Pass 收集配置与输出；供线程 C 消费 |
| 019-PipelineCore | TenEngine::pipelinecore | — | 自由函数/接口 | 构建逻辑管线（线程 B） | TenEngine/pipelinecore/LogicalPipeline.h | buildLogicalPipeline | ILogicalPipeline* buildLogicalPipeline(IFrameGraph const* graph, FrameContext const& ctx); 仅产出逻辑数据，不录 GPU 命令 |
| 019-PipelineCore | TenEngine::pipelinecore | — | struct | 单条可渲染项 | TenEngine/pipelinecore/RenderItem.h | RenderItem | 场景模型、UI、材质、排序 key 等；收集阶段产出 |
| 019-PipelineCore | TenEngine::pipelinecore | — | 接口/容器 | RenderItem 列表 | TenEngine/pipelinecore/RenderItem.h | IRenderItemList | 合并后的 RenderItem 列表；线程 C 多线程收集后 merge 得到 |
| 019-PipelineCore | TenEngine::pipelinecore | — | 自由函数/接口 | 多线程收集 RenderItem（线程 C） | TenEngine/pipelinecore/CollectPass.h | collectRenderItemsParallel | void collectRenderItemsParallel(ILogicalPipeline const* pipeline, FrameContext const& ctx, IRenderItemList* out_per_pass_or_merged); 多线程并行，内部 merge 或由调用方 merge |
| 019-PipelineCore | TenEngine::pipelinecore | — | 自由函数/接口 | 合并 RenderItem 列表 | TenEngine/pipelinecore/CollectPass.h | mergeRenderItems | void mergeRenderItems(IRenderItemList const* partial_lists, size_t count, IRenderItemList* merged); 线程 C 并行结束后合并 |
| 019-PipelineCore | TenEngine::pipelinecore | ILogicalCommandBuffer | 抽象接口 | 逻辑 CommandBuffer | TenEngine/pipelinecore/LogicalCommandBuffer.h | ILogicalCommandBuffer | CPU 侧逻辑命令序列（Draw、Bind 等），非 GPU 命令；由 convertToLogicalCommandBuffer 产出 |
| 019-PipelineCore | TenEngine::pipelinecore | — | 自由函数/接口 | 线程 D：渲染资源准备 | TenEngine/pipelinecore/RenderItem.h | prepareRenderResources | void prepareRenderResources(IRenderItemList const* items, IDevice* device); **必须在线程 D 调用**；为 RenderItem 创建/准备 GPU 渲染资源（纹理、缓冲、PSO 等） |
| 019-PipelineCore | TenEngine::pipelinecore | — | 自由函数/接口 | RenderItem 转逻辑 CB（线程 D） | TenEngine/pipelinecore/LogicalCommandBuffer.h | convertToLogicalCommandBuffer | ILogicalCommandBuffer* convertToLogicalCommandBuffer(IRenderItemList const* items, ILogicalPipeline const* pipeline); **必须在线程 D 调用**；与 GPU 操作同线程 |

### 020-Pipeline

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 020-Pipeline | TenEngine::pipeline | IRenderPipeline | 抽象接口 | 触发一次渲染（多阶段调度） | TenEngine/pipeline/RenderPipeline.h | IRenderPipeline::triggerRender | void triggerRender(FrameContext const& ctx); 内部：buildLogicalPipeline(B) → collectRenderItemsParallel + merge(C) → **在线程 D**：prepareRenderResources、convertToLogicalCommandBuffer、submitLogicalCommandBuffer（所有 GPU 操作在线程 D） |
| 020-Pipeline | TenEngine::pipeline | IRenderPipeline | 抽象接口 | 提交逻辑 CommandBuffer（须在线程 D 调用） | TenEngine/pipeline/RenderPipeline.h | IRenderPipeline::submitLogicalCommandBuffer | void submitLogicalCommandBuffer(ILogicalCommandBuffer* logical_cb); **必须在线程 D 调用**；将 logical_cb 交给 RHI 在同一线程 D 录制并 submit |

### 008-RHI

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 008-RHI | TenEngine::rhi | IDevice | 抽象接口 | 在线程 D 执行逻辑 CommandBuffer 并提交 | TenEngine/rhi/Device.h | IDevice::executeLogicalCommandBuffer | void executeLogicalCommandBuffer(ILogicalCommandBuffer* logical_cb); **必须在线程 D（GPU/Device 线程）调用**；所有图形 GPU 操作、GPU 资源创建须在线程 D；将 logical_cb 转成真实 ICommandList 并 submit |
| 008-RHI | TenEngine::rhi | IDevice | 约定 | GPU 线程约束 | TenEngine/rhi/Device.h | — | **所有** IDevice 的 GPU 资源创建、CommandBuffer 录制与提交**必须在线程 D 执行**；渲染物体的渲染资源创建也须在线程 D |

---

## 6. 参考（可选）

- **Unreal**：RHI 线程、Render 线程、Game 线程分离；Mesh Draw Command 生成（多线程）→ 逻辑命令 → RHI 线程录制并提交。
- **Unity**：SRP 多线程渲染、Job 化 Culling 与 DrawCommand 生成；主线程/渲染线程/GPU 流水线。
- **通用**：逻辑命令与真实命令分离、Device 单线程录制与提交，避免多线程争用 GPU 上下文。

---

*本故事派生出的 ABI 条目将同步到 `specs/_contracts/019-pipelinecore-ABI.md`、`020-pipeline-ABI.md`、`008-rhi-ABI.md`。与 US-rendering-001/002/003 互补：001 为单帧语义，002 为多帧流水线，003 为 FrameGraph AddPass，004 为多线程阶段划分；**线程 D = 唯一 GPU/Device 线程**：所有图形 GPU 操作、GPU 资源创建（含渲染物体的渲染资源）须在线程 D 执行。*
