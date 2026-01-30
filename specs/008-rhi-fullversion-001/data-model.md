# Data Model: 008-RHI 完整功能

**Feature**: 008-rhi-fullversion-001 | **Date**: 2026-01-29

## Entities

### Device 与后端

- **Backend**：枚举 `Vulkan`, `D3D12`, `Metal`。
- **IDevice**：图形设备抽象；创建队列、资源、PSO、命令列表；CreateDevice → DestroyDevice；RHI 管理。
- **IQueue**：队列句柄；GetQueue(type, index)；非拥有，生命周期与 IDevice 一致。
- **DeviceFeatures**：可读、可验证；最小集由 plan 定义（如 maxTextureDimension2D/3D）。

### 命令列表

- **ICommandList**：命令缓冲；单次录制周期内有效；IDevice 分配，Begin/End 录制，Submit 提交到 IQueue。
- **Submit**：将 ICommandList 提交到 IQueue 执行；Pipeline 产出的抽象命令缓冲通过等价接口交给 RHI（见 pipeline-to-rci）。

### 资源与视图

- **IBuffer / ITexture / ISampler**：创建后直至显式 Destroy；IDevice::Create*。
- **资源视图句柄**（ViewHandle / 描述符）：SRV/UAV/RTV/DSV 等概念；CreateView；与资源生命周期一致。
- **BufferDesc / TextureDesc / SamplerDesc / ViewDesc**：描述符；字段由实现定义。

### PSO

- **IPSO**：管线状态对象（图形/计算）；与 Shader 字节码绑定；可缓存；CreateGraphicsPSO / CreateComputePSO；DestroyPSO。
- **GraphicsPSODesc / ComputePSODesc**：含 Shader 字节码或模块引用；与 RenderCore/Shader 对接。

### 同步

- **IFence / ISemaphore**：多队列、跨帧同步；CreateFence / CreateSemaphore；Wait / Signal；生命周期按实现约定。
- **ResourceBarrier**：在 ICommandList 上声明；语义与契约能力 2、5 一致。

## Validation Rules

- CreateDevice / Create* 失败返回 nullptr；无效参数或资源不足有明确报告。
- GetQueue 无效类型或越界返回 nullptr。
- Submit 前须 End(ICommandList)；命令缓冲引用资源须有效且状态正确。
- 资源销毁顺序不得违反底层 API（如先释放依赖该资源的命令或 PSO）。

## State Transitions

- **IDevice**：CreateDevice → 使用（GetQueue、Create*、Submit 等）→ DestroyDevice → 失效。
- **ICommandList**：CreateCommandList → Begin → 录制（Draw/Dispatch/Copy/ResourceBarrier）→ End → Submit → 失效或归还。
- **IBuffer/ITexture/ISampler/IPSO**：Create* → 使用 → Destroy* → 失效。
