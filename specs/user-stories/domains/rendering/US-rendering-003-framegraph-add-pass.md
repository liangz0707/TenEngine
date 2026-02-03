# US-rendering-003：渲染通过 FrameGraph 组织，程序员通过 AddPass 等调用将 Pass 纳入渲染

- **标题**：渲染通过 FrameGraph 形式组织；程序员可通过 AddPass 等简单调用将渲染 Pass 纳入渲染；Pass 内可配置要渲染的场景、收集方式（遮挡剔除/视锥剔除/不剔除）、物体类型、渲染类型、自定义渲染输出，并可拿到收集到的渲染物体。
- **编号**：US-rendering-003

---

## 1. 角色/触发

- **角色**：程序员（引擎或游戏侧）
- **触发**：在构建或每帧设置渲染管线时，希望通过 **FrameGraph** 声明式地组织渲染 Pass，通过 **AddPass** 等简单 API 将每个 Pass 加入图；在每个 Pass 上配置场景、剔除方式、物体类型、渲染类型、输出目标，并在执行时拿到该 Pass 收集到的渲染物体列表。

---

## 2. 端到端流程

1. 程序员获取或创建 **IFrameGraph** 实例（由 Pipeline 或 PipelineCore 提供）。
2. 通过 **addPass** 添加一个或多个渲染 Pass；每次 addPass 返回 **IPassBuilder**（或传入 **PassDesc**），用于配置该 Pass。
3. 对每个 Pass 可配置：
   - **要渲染的场景**：绑定到某一场景（或场景根/相机）。
   - **收集方式**：视锥剔除、遮挡剔除、不剔除等（枚举或位掩码）。
   - **场景中物体的类型**：按类型/层级/标签过滤（如仅不透明、仅天空盒）。
   - **渲染类型**：如不透明、透明、自定义等，用于排序或 Shader 选择。
   - **渲染输出**：自定义渲染目标（RT）、深度、多目标等；可配置分辨率、格式。
   - **收集到的渲染物体**：在执行该 Pass 时，通过回调参数或 **PassContext::getCollectedObjects()** 拿到本 Pass 根据上述条件收集到的物体列表，再自行录制 DrawCall 或做其他处理。
4. 配置完成后调用 **compile** 或由 Pipeline 在 **tickPipeline/renderFrame** 时自动编译并执行图；执行顺序由依赖或声明顺序决定。
5. 执行某 Pass 时：管线根据该 Pass 的配置进行**场景收集**（按剔除方式、物体类型、渲染类型过滤），将结果传入 Pass 的 **execute 回调**（或写入 PassContext）；程序员在回调内使用收集到的物体列表录制命令或访问自定义输出。

---

## 3. 涉及模块

| 模块 | 职责摘要 |
|------|----------|
| 019-PipelineCore | FrameGraph 图结构、Pass 描述、AddPass、PassBuilder、剔除/类型/输出配置、Pass 执行回调语义 |
| 020-Pipeline | 持有或创建 FrameGraph、在 renderFrame/tickPipeline 中编译并执行图、驱动场景收集与 Pass 执行、与 RHI 提交衔接 |
| 004-Scene / 005-Entity | 场景与实体数据；Pipeline 按 Pass 配置做视锥/遮挡剔除与类型过滤时消费 |
| 008-RHI | Pass 执行时录制命令、绑定输出、DrawCall |

---

## 4. 每模块职责与 I/O

### 019-PipelineCore

- **职责**：定义 **IFrameGraph**、**addPass**（返回 IPassBuilder 或接受 PassDesc）；定义 **PassDesc** 或 PassBuilder 的配置项：场景、CullMode（视锥剔除/遮挡剔除/不剔除）、物体类型过滤、渲染类型、输出（RenderTarget 描述）；定义 **PassExecuteCallback** 签名（含 PassContext 与收集到的物体列表）；定义 **PassHandle** 或依赖声明方式（可选）。
- **输入**：程序员调用 addPass、setScene、setCullMode、setObjectTypeFilter、setRenderType、setOutput、setExecuteCallback；FrameContext（场景、相机等）在执行时由 Pipeline 注入。
- **输出**：IFrameGraph、IPassBuilder、PassDesc、CullMode 枚举、RenderType 枚举、PassContext、CollectedObjects 类型（或迭代器）；compile() 产出可执行图。

### 020-Pipeline

- **职责**：提供 **getFrameGraph()** 或 **createFrameGraph()**；在 **renderFrame/tickPipeline** 中根据当前 FrameContext **编译** FrameGraph（若为即时模式则每帧 addPass 再编译）；对每个 Pass **执行场景收集**（按 Pass 的剔除方式、物体类型、渲染类型调用 Scene/Entity 或内部 Culler）；将收集结果与 PassContext 传入 Pass 的 **execute 回调**；回调内程序员可调用 **PassContext::getCollectedObjects()**、绑定输出、录制 RHI 命令。
- **输入**：FrameContext；FrameGraph 中已声明的 Pass 列表与配置；Scene/Entity 可见性接口。
- **输出**：getFrameGraph()、createFrameGraph()；执行时每个 Pass 的 execute(PassContext, …) 中可访问 collectedObjects 与自定义输出。

### 004-Scene / 005-Entity

- **职责**：提供场景/实体查询与可见性接口，供 Pipeline 在 Pass 执行前按 CullMode、物体类型、渲染类型做收集（视锥剔除、遮挡剔除等）；本故事中仅消费，不新增 ABI，由 Pipeline 内部调用。

---

## 5. 派生接口（ABI 条目）

以下按 `docs/engine-abi-interface-generation-spec.md` 书写。

### 019-PipelineCore

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 019-PipelineCore | TenEngine::pipelinecore | IFrameGraph | 抽象接口 | FrameGraph 入口 | TenEngine/pipelinecore/FrameGraph.h | IFrameGraph::addPass | IPassBuilder* addPass(char const* name); 将 Pass 加入图，返回 Builder 供配置 |
| 019-PipelineCore | TenEngine::pipelinecore | IPassBuilder | 抽象接口 | Pass 配置 | TenEngine/pipelinecore/FrameGraph.h | IPassBuilder::setScene, setCullMode, setObjectTypeFilter, setRenderType, setOutput, setExecuteCallback | 配置场景、剔除方式、物体类型、渲染类型、输出、执行回调；build 后不可再改 |
| 019-PipelineCore | TenEngine::pipelinecore | — | 枚举 | 收集/剔除方式 | TenEngine/pipelinecore/FrameGraph.h | CullMode | enum class CullMode { None, FrustumCull, OcclusionCull, FrustumAndOcclusion }; |
| 019-PipelineCore | TenEngine::pipelinecore | — | 枚举 | 渲染类型 | TenEngine/pipelinecore/FrameGraph.h | RenderType | enum class RenderType { Opaque, Transparent, Overlay, Custom }; 可按实现扩展 |
| 019-PipelineCore | TenEngine::pipelinecore | — | struct | Pass 输出描述 | TenEngine/pipelinecore/FrameGraph.h | PassOutputDesc | 渲染目标、深度、多 RT、分辨率、格式等；程序员配置自定义输出 |
| 019-PipelineCore | TenEngine::pipelinecore | PassContext | 抽象/struct | Pass 执行时上下文 | TenEngine/pipelinecore/FrameGraph.h | PassContext::getCollectedObjects | IRenderObjectList const* getCollectedObjects() const; 执行回调内调用，拿到本 Pass 收集到的物体列表 |
| 019-PipelineCore | TenEngine::pipelinecore | — | 回调类型 | Pass 执行回调 | TenEngine/pipelinecore/FrameGraph.h | PassExecuteCallback | void (*)(PassContext& ctx, ICommandList* cmd); 或 void (*)(PassContext& ctx); 在 ctx 中取 collectedObjects 与输出 |
| 019-PipelineCore | TenEngine::pipelinecore | IFrameGraph | 抽象接口 | 编译图 | TenEngine/pipelinecore/FrameGraph.h | IFrameGraph::compile | bool compile(); 编译为可执行图；依赖与顺序由实现解析 |
| 019-PipelineCore | TenEngine::pipelinecore | — | 接口/类型 | 收集到的物体列表 | TenEngine/pipelinecore/FrameGraph.h | IRenderObjectList | 只读列表/迭代器，供 Pass 回调遍历绘制；由 Pipeline 在收集阶段填充 |

### 020-Pipeline

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 020-Pipeline | TenEngine::pipeline | IRenderPipeline | 抽象接口 | 获取/设置 FrameGraph | TenEngine/pipeline/RenderPipeline.h | IRenderPipeline::getFrameGraph, setFrameGraph | IFrameGraph* getFrameGraph(); void setFrameGraph(IFrameGraph* graph); 每帧执行时编译并执行 graph 中的 Pass |
| 020-Pipeline | TenEngine::pipeline | — | 自由函数 | 创建 FrameGraph | TenEngine/pipeline/FrameGraph.h | createFrameGraph | IFrameGraph* createFrameGraph(); 或由 Pipeline 内部创建；调用方或 Pipeline 管理生命周期 |
| 020-Pipeline | TenEngine::pipeline | IRenderPipeline | 抽象接口 | 执行图 | TenEngine/pipeline/RenderPipeline.h | 内部行为 | renderFrame/tickPipeline 内：compile FrameGraph；对每 Pass 做场景收集（按 CullMode/类型/渲染类型），再调用 Pass 的 execute(PassContext)，PassContext 中提供 getCollectedObjects() 与输出绑定 |

（PassContext、IRenderObjectList、IPassBuilder 可由 019 定义，020 在执行时注入实现；或 020 引用 019 的类型并在执行阶段填充。）

---

## 6. 参考（可选）

- **Unreal**：[Render Dependency Graph (RDG)](https://dev.epicgames.com/documentation/en-us/unreal-engine/render-dependency-graph-in-unreal-engine)：AddPass、Pass 参数声明读写资源、编译后执行。
- **Unity**：SRP RenderGraph、RenderGraphContext、AddRasterRenderPass 等；Pass 内可配置 culling、render queue、target。
- **Frostbite / GDC**：FrameGraph 即帧图、Pass 声明式、自动资源生命周期与同步。

---

*本故事派生出的 ABI 条目将同步到 `specs/_contracts/019-pipelinecore-ABI.md`、`020-pipeline-ABI.md`。*
