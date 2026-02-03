# US-004：流水线式多帧渲染（已迁移至领域）

- **状态**：已迁移至 **rendering** 领域，新编号 **US-rendering-002**。
- **正式文档**：[domains/rendering/US-rendering-002-pipelined-multi-frame-render.md](./domains/rendering/US-rendering-002-pipelined-multi-frame-render.md)
- **领域索引**：[domains/rendering/index.md](./domains/rendering/index.md)

以下为原内容备份，以领域内文档为准。

---

# US-rendering-002：流水线式多帧渲染（原 US-004）

- **标题**：渲染支持同时 2–4 帧在途，流水线式阶段（收集物体 / 录制 CommandBuffer / 已提交待 Present）
- **编号**：US-004（现 US-rendering-002）

---

## 1. 角色/触发

- **角色**：主循环（或 Editor 视口）
- **触发**：每帧驱动渲染管线时，希望 CPU 与 GPU 并行：一帧在「收集场景渲染物体」，一帧在「录制 CommandBuffer」，一帧已提交并在等待 Present，从而提高吞吐、降低单帧延迟。

---

## 2. 端到端流程（流水线阶段）

任意时刻，**2–4 个逻辑帧**（由配置决定）处于不同阶段：

| 阶段 | 含义 | 典型工作 |
|------|------|----------|
| **A. 收集场景渲染物体** | 基于当前场景/相机做视锥剔除、排序，产出本帧的渲染列表（DrawCall 输入） | 场景遍历、可见性、LOD、批处理划分 |
| **B. 录制 CommandBuffer** | 根据上一阶段的渲染列表，向 RHI 录制命令（清屏、绑定、DrawCall） | 获取本 slot 的 ICommandList；record；submit |
| **C. 已提交待 Present** | 该帧命令已提交到 GPU，等待交换链可 Present 时 Present | RHI 内部 Fence/Semaphore；Present 时与「当前待 Present 帧」同步 |

主循环每帧调用一次「推进」接口（如 `tickPipeline()` 或 `advanceFrame()`）时：

1. **Pipeline** 决定本帧使用的 **slot 索引**（0 ～ frame_in_flight_count - 1），并可能等待该 slot 上一轮 GPU 已完成（避免覆盖未完成的资源）。
2. **阶段 A**：在该 slot 上执行「收集场景渲染物体」（传入当前场景/相机/视口）；产出渲染列表并绑定到该 slot。
3. **阶段 B**：使用该 slot 对应的 **CommandList** 录制命令；录制结束后 **submit** 该 slot 的命令到 GPU。
4. **阶段 C**：对「已提交且轮到 Present 的那一帧」执行 **Present**；RHI 内部用 Fence/Semaphore 保证 Present 顺序与完成顺序一致。

---

## 3. 涉及模块

| 模块 | 职责摘要 |
|------|----------|
| 019-PipelineCore | 在途帧数（2–4）、帧 slot 语义、同步点抽象（等待 slot 可复用） |
| 020-Pipeline | 每帧推进、按 slot 执行阶段 A/B、调用 RHI 的 submit(slot) 与 Present 同步 |
| 008-RHI | 每 slot 的 CommandList、Fence 每 slot、submit(slot)、waitForSlot(slot)、Present 与「待 Present 帧」同步 |

---

## 4. 每模块职责与 I/O

### 019-PipelineCore

- **职责**：定义「在途帧数」、帧 slot 索引类型；提供「等待某 slot 上的 GPU 工作完成」的抽象，供 Pipeline 在复用 slot 前等待。
- **输入**：配置（frame_in_flight_count = 2～4）；Pipeline 在推进时传入 slot 请求等待。
- **输出**：`kMaxFramesInFlight` 或 `PipelineConfig::frameInFlightCount`；`waitForSlot(slot)` 或由 RHI 暴露的 Fence 等待。

### 020-Pipeline

- **职责**：每帧调用时推进流水线：决定当前 slot；执行阶段 A（收集物体）→ 阶段 B（录制并 submit）；与 RHI 协同完成 Present。在复用 slot 前通过 Core/RHI 等待该 slot 上一轮完成。
- **输入**：本帧的 `FrameContext`（场景、相机、视口等）；Pipeline 配置（在途帧数）；RHI 的 getCommandList(slot)、submit(slot)、present()。
- **输出**：`advanceFrame()` 或 `tickPipeline(ctx)` 返回当前 slot；内部调用 `beginSceneCollect(slot)`、`beginCommandRecord(slot)`、`submitFrame(slot)`；Present 由 Pipeline 或 RHI 在适当时机调用。

### 008-RHI

- **职责**：为每个 slot（0 ～ frame_in_flight - 1）提供独立的 CommandList（或池）；每 slot 一个 Fence；`getCommandList(slot)`、`submit(slot)`、`waitForSlot(slot)`；Present 时与「当前待 Present 的帧」同步（等待其 Fence 再 Present）。
- **输入**：Pipeline 的 getCommandList(slot)、submit(slot)、present 请求；设备创建时或 SwapChain 创建时指定 frame_in_flight_count。
- **输出**：`getCommandListForSlot(slot)`；`submitCommandList(slot)`；`waitForSlot(slot)`；`present()` 内部与待 Present 帧同步。

---

## 5. 派生接口（ABI 条目）

以下按 `docs/engine-abi-interface-generation-spec.md` 书写。

### 019-PipelineCore

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 019-PipelineCore | TenEngine::pipelinecore | — | 常量 | 最大在途帧数 | TenEngine/pipelinecore/Config.h | kMaxFramesInFlight | 建议 2～4；实现可配置 |
| 019-PipelineCore | TenEngine::pipelinecore | — | struct | 管线配置 | TenEngine/pipelinecore/Config.h | PipelineConfig | 含 frameInFlightCount（2～4）；创建 Pipeline/SwapChain 时传入 |
| 019-PipelineCore | TenEngine::pipelinecore | — | 类型别名 | 帧 slot 索引 | TenEngine/pipelinecore/Config.h | FrameSlotId | uint32_t，范围 [0, frameInFlightCount) |

### 020-Pipeline

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 020-Pipeline | TenEngine::pipeline | IRenderPipeline | 抽象接口 | 推进流水线一帧（多阶段） | TenEngine/pipeline/RenderPipeline.h | IRenderPipeline::tickPipeline | void tickPipeline(FrameContext const& ctx); 内部按 slot 执行：等待 slot 可复用 → 阶段 A 收集物体 → 阶段 B 录制并 submit → 若轮到则 present |
| 020-Pipeline | TenEngine::pipeline | IRenderPipeline | 抽象接口 | 获取当前帧 slot | TenEngine/pipeline/RenderPipeline.h | IRenderPipeline::getCurrentSlot | FrameSlotId getCurrentSlot() const; 本帧使用的 slot，用于访问该 slot 的渲染列表或资源 |
| 020-Pipeline | TenEngine::pipeline | — | struct | 创建管线时的配置 | TenEngine/pipeline/RenderPipeline.h | RenderPipelineDesc | 含 frameInFlightCount、IDevice*、ISwapChain* 等；与 PipelineConfig 一致 |

### 008-RHI

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 008-RHI | TenEngine::rhi | IDevice | 抽象接口 | 按 slot 获取命令列表 | TenEngine/rhi/Device.h | IDevice::getCommandListForSlot | ICommandList* getCommandListForSlot(FrameSlotId slot); 每 slot 一个 CommandList，复用前需 waitForSlot(slot) |
| 008-RHI | TenEngine::rhi | IDevice | 抽象接口 | 提交指定 slot 的命令列表 | TenEngine/rhi/Device.h | IDevice::submitCommandList | void submitCommandList(FrameSlotId slot); 提交后该 slot 的 Fence 会 signal；Present 前会等待对应帧 Fence |
| 008-RHI | TenEngine::rhi | IDevice | 抽象接口 | 等待某 slot 上 GPU 完成 | TenEngine/rhi/Device.h | IDevice::waitForSlot | void waitForSlot(FrameSlotId slot); 阻塞直到该 slot 上一轮提交的 work 完成，便于安全复用资源 |
| 008-RHI | TenEngine::rhi | ISwapChain | 抽象接口 | 带同步的 Present | TenEngine/rhi/SwapChain.h | ISwapChain::present | bool present(); 内部与「当前待 Present 帧」的 Fence 同步后再 Present；每帧调用一次 |
| 008-RHI | TenEngine::rhi | — | struct | 设备描述（含在途帧数） | TenEngine/rhi/Device.h | DeviceDesc | 增加 frameInFlightCount（2～4）；SwapChain 创建时也可指定 |

---

*本故事派生出的 ABI 条目将同步到 `specs/_contracts/019-pipelinecore-ABI.md`、`020-pipeline-ABI.md`、`008-rhi-ABI.md`。与 US-002 的关系：US-002 描述单帧语义，US-004 在此基础上扩展为 2–4 帧流水线及 slot/Fence 同步。*
