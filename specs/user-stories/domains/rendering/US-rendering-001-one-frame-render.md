# US-rendering-001：一帧渲染

- **标题**：一帧渲染（从场景与相机到屏幕图像）
- **编号**：US-rendering-001（原 US-002）

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

（同原 US-002，略。）

---

## 5. 派生接口（ABI 条目）

（见原 US-002 文档；已同步到 `020-pipeline-ABI.md`、`008-rhi-ABI.md`。）

---

## 6. 参考（可选）

- Unreal：RHI → RenderCore → Renderer；RDG（Pass 图、资源生命周期）。
- Unity：SRP（HDRP/URP）、Camera.Render、CommandBuffer。

*ABI 条目已同步到 `specs/_contracts/020-pipeline-ABI.md`、`008-rhi-ABI.md`。*
