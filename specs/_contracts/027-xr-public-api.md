# 契约：027-XR 模块对外 API

## 适用模块

- **实现方**：027-XR（L4；AR/VR 子系统、头显、手柄；无下游）
- **对应规格**：`docs/module-specs/027-xr.md`
- **依赖**：001-Core、007-Subsystems、006-Input、020-Pipeline

## 消费者

- 无（L4 消费端；向应用或运行时提供 XR 会话、帧、提交到 XR 交换链；或作为 Subsystems 子系统挂接）

## 能力列表

### 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| XRSessionHandle | XR 会话句柄；与 Subsystems 子系统、平台 XR 运行时对接 | 创建后直至结束会话 |
| XRFrameHandle | 帧句柄；视口、投影、与 Pipeline 提交、RHI XR 交换链对接 | 每帧 |
| 提交接口 | 将 Pipeline 产出提交到 XR 交换链；与 pipeline-to-rci 及 RHI 约定一致 | 每帧 |

### 能力（提供方保证）

| 序号 | 能力 | 说明 |
|------|------|------|
| 1 | 会话 | XRSessionHandle、与 Subsystems、平台 XR 运行时对接 |
| 2 | 帧 | XRFrameHandle、视口、投影、与 Pipeline 提交对接 |
| 3 | 提交 | 将 Pipeline 产出提交到 XR 交换链；与 RHI 约定一致 |
| 4 | 输入 | 与 006-Input 对接（XR 输入） |

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。

## 约束

- 须在 Core、Subsystems、Input、Pipeline 初始化之后使用。可与 007-Subsystems 对接（XR 作为子系统挂接）。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 027-XR 契约 |
| 2026-02-05 | 统一目录；能力列表用表格 |
