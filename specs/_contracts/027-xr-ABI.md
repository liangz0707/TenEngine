# 027-XR 模块 ABI

- **契约**：[027-xr-public-api.md](./027-xr-public-api.md)（能力与类型描述）
- **本文件**：027-XR 对外 ABI 显式表。
- **参考**：Unity XR、UE OpenXR；XR 会话、帧、视口/投影、提交到 XR 交换链；与 Subsystems、Input、Pipeline 对接。
- **命名**：成员方法采用 **PascalCase**；说明列给出**完整函数签名**。

## ABI 表

列定义：**模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明**

### 会话（Session）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 027-XR | te::xr | IXRSession | 抽象接口 | 创建/结束会话 | te/xr/XRSession.h | IXRSession::BeginSession, EndSession | `bool BeginSession(XRSessionDesc const& desc);` `void EndSession();` 与 Subsystems、平台 XR 运行时对接；创建后直至结束会话 |
| 027-XR | te::xr | — | 自由函数/工厂 | 创建 XR 会话 | te/xr/XRSession.h | CreateXRSession | `IXRSession* CreateXRSession();` 失败返回 nullptr；或通过 SubsystemRegistry::GetSubsystem\<IXRSession\>() 获取 |
| 027-XR | te::xr | — | struct | 会话描述 | te/xr/XRSession.h | XRSessionDesc | 运行时、选项；由调用方填充 |

### 帧（Frame）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 027-XR | te::xr | IXRSession | 抽象接口 | 帧开始 | te/xr/XRFrame.h | IXRSession::BeginFrame | `bool BeginFrame();` 返回 false 表示本帧不渲染；每帧 |
| 027-XR | te::xr | IXRSession | 抽象接口 | 帧结束 | te/xr/XRFrame.h | IXRSession::EndFrame | `void EndFrame();` 每帧 |
| 027-XR | te::xr | IXRSession | 抽象接口 | 获取视口/投影 | te/xr/XRFrame.h | IXRSession::GetViewCount, GetViewport, GetProjection | `uint32_t GetViewCount() const;` `void GetViewport(uint32_t viewIndex, Viewport* out) const;` `void GetProjection(uint32_t viewIndex, float nearZ, float farZ, float* outMatrix) const;` 与 Pipeline/RHI 对接 |
| 027-XR | te::xr | — | struct | 视口 | te/xr/XRFrame.h | Viewport | x、y、width、height；每帧有效 |

### 提交（Submit）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 027-XR | te::xr | IXRSession | 抽象接口 | 提交到 XR 交换链 | te/xr/XRFrame.h | IXRSession::Submit | `void Submit(ITexture* const* swapImages, uint32_t count);` 将 Pipeline 产出提交到 XR 交换链；与 pipeline-to-rci 及 RHI 约定一致；每帧 |
| 027-XR | te::xr | IXRSession | 抽象接口 | 获取 XR 交换链 | te/xr/XRFrame.h | IXRSession::GetSwapChain | `ISwapChain* GetSwapChain(uint32_t viewIndex) const;` 与 RHI XR 交换链对接；供 Pipeline 渲染目标绑定 |

### 输入（可选）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 027-XR | te::xr | IXRSession | 抽象接口 | XR 控制器/头显输入 | te/xr/XRInput.h | IXRSession::GetControllerPose, GetHeadPose | `void GetControllerPose(uint32_t hand, Transform* out) const;` `void GetHeadPose(Transform* out) const;` 与 006-Input 模块扩展对接（可选） |

*来源：契约能力 Session、Frame、Submit、Input；参考 Unity XR、UE OpenXR。*
