# 契约：027-XR 模块对外 API

## 适用模块

- **实现方**：**027-XR**（XR 会话、帧与提交；依赖 Core、Subsystems、Input、Pipeline）
- **对应规格**：`docs/module-specs/027-xr.md`
- **依赖**：001-Core、007-Subsystems、006-Input、020-Pipeline（见 `000-module-dependency-map.md`）

## 消费者（T0 下游）

- **无**（XR 为 L4 消费端；向应用或运行时提供 XR 会话、帧、提交到 XR 交换链；或作为 Subsystems 子系统挂接）

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。
- 当前契约版本：（由实现或计划阶段填写）

## 类型与句柄（跨边界）

本模块向**应用/运行时**提供以下类型（引擎内无其他模块依赖本模块；Pipeline 消费 RHI 与 SwapChain，XR 通过 Pipeline 或 RHI 提交到 XR 交换链）：

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| XRSessionHandle | XR 会话句柄；与 Subsystems 子系统、平台 XR 运行时对接 | 创建后直至结束会话 |
| XRFrameHandle | 帧句柄；视口、投影、与 Pipeline 提交、RHI XR 交换链对接 | 每帧 |
| 提交接口 | 将 Pipeline 产出提交到 XR 交换链；与 pipeline-to-rci 及 RHI 约定一致 | 每帧 |

与 007-Subsystems 对接（XR 可作为子系统挂接）；与 006-Input 对接（XR 输入）；与 020-Pipeline 对接（渲染产出提交）。**ABI 显式表**：[027-xr-ABI.md](./027-xr-ABI.md)。

## 能力列表（提供方保证）

1. **Session**：IXRSession::BeginSession、EndSession；CreateXRSession、XRSessionDesc；与 Subsystems、平台 XR 运行时对接。
2. **Frame**：IXRSession::BeginFrame、EndFrame、GetViewCount、GetViewport、GetProjection；Viewport；与 Pipeline/RHI 对接。
3. **Submit**：IXRSession::Submit、GetSwapChain；提交到 XR 交换链；与 RHI、pipeline-to-rci 约定一致。
4. **Input（可选）**：IXRSession::GetControllerPose、GetHeadPose；XR 控制器/头显输入、与 Input 模块扩展对接。

## 调用顺序与约束

- 须在 Core、Subsystems、Input、Pipeline 初始化之后使用；提交格式与 RHI/Pipeline 约定一致。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 每模块一契约：027-XR 对应本契约；无下游；依赖表与 007/008/020 规格一致 |
| 2026-01-28 | 根据 027-xr-ABI 反向更新：IXRSession、CreateXRSession、BeginSession、EndSession、BeginFrame、EndFrame、GetViewport、GetProjection、Submit、GetSwapChain、GetControllerPose、GetHeadPose；能力与类型与 ABI 表一致 |