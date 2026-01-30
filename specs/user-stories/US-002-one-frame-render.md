# US-002：一帧渲染（已迁移至领域）

- **状态**：已迁移至 **rendering** 领域，新编号 **US-rendering-001**。
- **正式文档**：[domains/rendering/US-rendering-001-one-frame-render.md](./domains/rendering/US-rendering-001-one-frame-render.md)
- **领域索引**：[domains/rendering/index.md](./domains/rendering/index.md)

以下为原内容备份，以领域内文档为准。

---

# US-rendering-001：一帧渲染（原 US-002）

- **标题**：一帧渲染（从场景与相机到屏幕图像）
- **编号**：US-002（现 US-rendering-001）

---

## 1. 角色/触发

- **角色**：主循环（或 Editor 视口）
- **触发**：每帧在「更新」之后调用渲染管线，将当前场景、相机与光照等信息绘制到当前渲染目标（屏幕或离屏 RT）。

---

## 2. 端到端流程

1. 主循环在 Tick 回调中拿到本帧 `delta_time` 与当前**场景/相机**（由 Scene/Entity 等提供）。
2. **Pipeline**：接收本帧上下文（场景图、相机、渲染目标、视口）；构建/获取 Pass 图，按 Pass 顺序执行。
3. **Pipeline** 每 Pass：向 **RHI** 提交命令（清屏、绑定 PSO/资源、DrawCall）；RHI 录制到命令列表。
4. **RHI**：将命令列表提交到 GPU（Present 或 提交到队列）；可选：同步（Fence）或下一帧再 Present。
5. 帧结束；下一帧可更新场景后再执行上述流程。

---

## 3. 涉及模块

| 模块 | 职责摘要 |
|------|----------|
| 004-Scene | 场景图、当前主相机、可见节点（本故事仅消费） |
| 005-Entity | 实体/组件、可见渲染器列表（本故事仅消费） |
| 019-PipelineCore | Pass 图、资源生命周期、提交抽象 |
| 020-Pipeline | 管线上下文、渲染目标、DrawCall 编排、调用 RHI |
| 008-RHI | 设备、命令列表、提交、Present |

---

## 4. 每模块职责与 I/O

### 020-Pipeline

- **职责**：提供「一帧渲染」入口；持有或接收场景/相机/视口；按 Pass 调用 RHI 提交 DrawCall，最后 Present。
- **输入**：`FrameContext`（场景根、相机、渲染目标、视口矩形、delta_time）；上游 Pass 图与资源描述（来自 PipelineCore）。
- **输出**：`void renderFrame(FrameContext const& ctx)`；内部调用 RHI 的 beginFrame/endFrame、recordCommands、submit、present。

### 008-RHI

- **职责**：提供设备、命令列表、渲染目标绑定、DrawCall 录制与提交、Present。
- **输入**：来自 Pipeline 的录制请求（清屏、SetRenderTarget、SetPSO、DrawIndexed 等）；Present 请求。
- **输出**：`IDevice`、`ICommandList`、`IRenderTarget`；`beginFrame()`/`endFrame()`；`submit()`/`present()`。

### 019-PipelineCore（本故事中 Pipeline 依赖）

- **职责**：定义 Pass 图、Pass 间资源、提交接口抽象；Pipeline 实现层按此编排。
- **输入**：Pass 描述、资源依赖。
- **输出**：`IPassGraph`、`PassSchedule`；资源句柄与生命周期（本故事派生条目可简化，仅列出 Pipeline 依赖的类型）。

---

## 5. 派生接口（ABI 条目）

### 020-Pipeline

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 020-Pipeline | TenEngine::pipeline | IRenderPipeline | 抽象接口 | 一帧渲染入口 | TenEngine/pipeline/RenderPipeline.h | IRenderPipeline::renderFrame | void renderFrame(FrameContext const& ctx); 由主循环或 Editor 每帧调用 |
| 020-Pipeline | TenEngine::pipeline | — | struct | 帧上下文 | TenEngine/pipeline/FrameContext.h | FrameContext | 含 scene_root, camera, render_target, viewport, delta_time；只读，由调用方填充 |
| 020-Pipeline | TenEngine::pipeline | — | 自由函数 | 创建默认渲染管线 | TenEngine/pipeline/RenderPipeline.h | createRenderPipeline | IRenderPipeline* createRenderPipeline(IDevice* device); 调用方或引擎管理生命周期 |

### 008-RHI

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 008-RHI | TenEngine::rhi | IDevice | 抽象接口 | 图形设备 | TenEngine/rhi/Device.h | IDevice | 创建命令列表、渲染目标、PSO 等；不直接构造，见 createDevice |
| 008-RHI | TenEngine::rhi | — | 自由函数 | 创建设备 | TenEngine/rhi/Device.h | createDevice | IDevice* createDevice(DeviceDesc const& desc); 失败返回 nullptr |
| 008-RHI | TenEngine::rhi | ICommandList | 抽象接口 | 命令列表 | TenEngine/rhi/CommandList.h | ICommandList::begin, end, submit | begin() 开始录制；end() 结束；submit() 提交到队列 |
| 008-RHI | TenEngine::rhi | ICommandList | 抽象接口 | 清屏与绘制 | TenEngine/rhi/CommandList.h | ICommandList::clearRenderTarget, drawIndexed | 由 Pipeline 在录制时调用 |
| 008-RHI | TenEngine::rhi | ISwapChain | 抽象接口 | 交换链 | TenEngine/rhi/SwapChain.h | ISwapChain::present | bool present(); 每帧调用一次，失败可重试 |
| 008-RHI | TenEngine::rhi | — | struct | 设备描述 | TenEngine/rhi/Device.h | DeviceDesc | 后端类型、窗口句柄等；下游按需填充 |

---

*本故事派生出的 ABI 条目将同步到 `specs/_contracts/020-pipeline-ABI.md`、`008-rhi-ABI.md`；019-PipelineCore 若需显式类型可单独补充。*
