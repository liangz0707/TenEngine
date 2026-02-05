# US-rendering-002：流水线式多帧渲染（2–4 帧在途）

- **标题**：渲染支持同时 2–4 帧在途，流水线式阶段（收集物体 / 录制 CommandBuffer / 已提交待 Present）
- **编号**：US-rendering-002（原 US-004）

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

主循环每帧调用一次「推进」接口（如 `tickPipeline(ctx)`）时：Pipeline 决定当前 slot → 阶段 A → 阶段 B（录制并 submit）→ 若轮到则 present；RHI 提供 getCommandListForSlot、submitCommandList、waitForSlot、present（与待 Present 帧同步）。

---

## 3. 涉及模块

| 模块 | 职责摘要 |
|------|----------|
| 019-PipelineCore | 在途帧数（2–4）、帧 slot 语义、同步点抽象 |
| 020-Pipeline | 每帧推进、按 slot 执行阶段 A/B、调用 RHI submit(slot) 与 Present |
| 008-RHI | 每 slot 的 CommandList、Fence、submit(slot)、waitForSlot(slot)、Present 同步 |

---

## 4. 每模块职责与 I/O

（同原 US-004，略。）

---

## 5. 派生接口（ABI 条目）

（见原 US-004 文档；已同步到 `019-pipelinecore-ABI.md`、`020-pipeline-ABI.md`、`008-rhi-ABI.md`。）

---

## 6. 参考（可选）

- Unreal：RDG 多帧异步、Fence/Semaphore、SwapChain Present 同步。
- Unity：SRP 多相机、CommandBuffer 异步执行。

*ABI 条目已同步到 `specs/_contracts/019-pipelinecore-ABI.md`、`020-pipeline-ABI.md`、`008-rhi-ABI.md`。*
