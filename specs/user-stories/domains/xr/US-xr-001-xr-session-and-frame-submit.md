# US-xr-001：XR 会话与帧提交（Session 创建、帧循环、提交到 XR 交换链）

- **标题**：应用可创建 **XR 会话**（如 OpenXR）、在每帧**获取 XR 帧状态**（视口、投影、眼偏移）、将**渲染结果提交到 XR 交换链**；与 Pipeline 对接渲染、与 Subsystems 对接会话生命周期。
- **编号**：US-xr-001

---

## 1. 角色/触发

- **角色**：游戏/应用、Pipeline
- **触发**：需要在 **XR 设备**（VR/AR）上运行；每帧需获取 XR 视口与投影、渲染左右眼（或单眼）、将图像**提交到 XR 交换链**（Present 到头显）。

---

## 2. 端到端流程

1. 调用方在初始化后创建 **XR Session**（或通过 Subsystems 注册 XR 子系统）；Session 与平台 XR 运行时（如 OpenXR）对接，获取**显示/视图**与**交换链**。
2. 每帧 **beginFrame()** 获取本帧 XR 状态（视口、FOV、眼偏移、追踪姿态）；Pipeline 使用该状态**渲染左/右眼**（或单眼），产出到 XR 交换链绑定的 RT。
3. **endFrame()** 或 **submitFrame()** 将本帧图像提交到 XR 运行时；XR 运行时负责同步与 Present 到头显。
4. Session 结束时 **endSession()**；与 Application 生命周期或 Subsystems 协同。

---

## 3. 涉及模块

| 模块 | 职责摘要 |
|------|----------|
| 027-XR | createSession、endSession、beginFrame、endFrame/submitFrame；视口、投影、眼偏移、交换链；与 Pipeline/RHI 对接 |
| 020-Pipeline | 使用 XR 视口/投影渲染、输出到 XR 交换链 RT；XRSubmit 或等价 |
| 007-Subsystems | XR 作为子系统注册、生命周期 |
| 008-RHI | XR 交换链、多视口、与平台 XR API 对接 |

---

## 4. 每模块职责与 I/O

### 027-XR

- **职责**：提供 **createSession**、**endSession**、**beginFrame**、**endFrame/submitFrame**；本帧视口、投影、眼偏移、交换链句柄；与 Pipeline 对接渲染目标、与 RHI 对接 XR 交换链。
- **输入**：平台 XR 运行时、每帧状态请求。
- **输出**：XRFrameHandle、视口/投影、交换链；供 Pipeline 渲染并提交。

---

## 5. 派生 ABI（与契约对齐）

- **027-xr-ABI**：createSession、beginFrame、endFrame、submitFrame、XRFrameHandle、视口/投影。详见 `specs/_contracts/027-xr-ABI.md`。

---

## 6. 验收要点

- 可创建 XR 会话、每帧获取视口/投影、将渲染结果提交到 XR 交换链并在头显显示。
