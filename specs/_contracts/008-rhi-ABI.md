# 008-RHI 模块 ABI

- **契约**：[008-rhi-public-api.md](./008-rhi-public-api.md)（能力与类型描述）
- **本文件**：008-RHI 对外 ABI 显式表。
- **平台与接口**：引擎支持 **Android、iOS** 等平台（见 001-Core）；RHI 支持 **Vulkan**、**Metal（MTL）**、**D3D12/DXIL** 等图形接口，及 **GLSL**、**HLSL/DXIL**、**MSL** 等 Shader 接口。**可以通过宏来判断执行哪一段代码**（如 TE_RHI_VULKAN、TE_RHI_METAL、TE_RHI_D3D12），编译时选择后端实现路径。
- **渲染资源显式控制位置**：**准备/创建/更新 GPU 资源**（**CreateDeviceResource**、**UpdateDeviceResource**）由本模块提供；**提交到实际 GPU Command**（**SubmitCommandBuffer**）见 executeLogicalCommandBuffer / submitCommandList。创建逻辑渲染资源、CollectCommandBuffer、PrepareRenderMaterial/Mesh 见 019-PipelineCore/020-Pipeline。

## ABI 表

列定义：**模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明**

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 008-RHI | TenEngine::rhi | IDevice | 抽象接口 | 图形设备 | TenEngine/rhi/Device.h | IDevice | 创建命令列表、渲染目标、PSO 等；不直接构造，见 CreateDevice |
| 008-RHI | TenEngine::rhi | — | 自由函数 | 创建设备 | TenEngine/rhi/Device.h | CreateDevice | `IDevice* CreateDevice(DeviceDesc const& desc);` 失败返回 nullptr |
| 008-RHI | TenEngine::rhi | ICommandList | 抽象接口 | 命令列表 | TenEngine/rhi/CommandList.h | ICommandList::Begin, End, Submit | `void Begin();` `void End();` `void Submit();` 开始录制、结束、提交到队列 |
| 008-RHI | TenEngine::rhi | ICommandList | 抽象接口 | 清屏与绘制 | TenEngine/rhi/CommandList.h | ICommandList::ClearRenderTarget, DrawIndexed | `void ClearRenderTarget(...);` `void DrawIndexed(uint32_t index_count, uint32_t instance_count, ...);` 由 Pipeline 在录制时调用 |
| 008-RHI | TenEngine::rhi | ISwapChain | 抽象接口 | 交换链 | TenEngine/rhi/SwapChain.h | ISwapChain::Present | `bool Present();` 每帧调用一次，失败可重试 |
| 008-RHI | TenEngine::rhi | — | struct | 设备描述 | TenEngine/rhi/Device.h | DeviceDesc | 后端类型（Vulkan/Metal/D3D12）、窗口句柄、frameInFlightCount（2～4）；下游按需填充 |
| 008-RHI | TenEngine::rhi | — | 枚举/宏 | 图形后端 | TenEngine/rhi/Backend.h | BackendType / TE_RHI_* | `enum class BackendType { Vulkan, Metal, D3D12 };` 或编译期宏 TE_RHI_VULKAN、TE_RHI_METAL、TE_RHI_D3D12 |
| 008-RHI | TenEngine::rhi | IDevice | 抽象接口 | 按 slot 获取命令列表 | TenEngine/rhi/Device.h | IDevice::GetCommandListForSlot | `ICommandList* GetCommandListForSlot(FrameSlotId slot);` 每 slot 一个 CommandList，复用前需 WaitForSlot(slot) |
| 008-RHI | TenEngine::rhi | IDevice | 抽象接口 | 提交指定 slot 的命令列表 | TenEngine/rhi/Device.h | IDevice::SubmitCommandList | `void SubmitCommandList(FrameSlotId slot);` 提交后该 slot 的 Fence signal；Present 前等待对应帧 Fence |
| 008-RHI | TenEngine::rhi | IDevice | 抽象接口 | 等待某 slot 上 GPU 完成 | TenEngine/rhi/Device.h | IDevice::WaitForSlot | `void WaitForSlot(FrameSlotId slot);` 阻塞直到该 slot 上一轮提交的 work 完成 |
| 008-RHI | TenEngine::rhi | ISwapChain | 抽象接口 | 带同步的 Present | TenEngine/rhi/SwapChain.h | ISwapChain::Present | `bool Present();` 内部与「当前待 Present 帧」的 Fence 同步后再 Present；每帧调用一次 |
| 008-RHI | TenEngine::rhi | IDevice | 抽象接口 | 在线程 D 执行逻辑 CommandBuffer 并提交 | TenEngine/rhi/Device.h | IDevice::ExecuteLogicalCommandBuffer | `void ExecuteLogicalCommandBuffer(ILogicalCommandBuffer* logical_cb);` **必须在线程 D 调用**；将 logical_cb 转成真实 ICommandList 并 submit |
| 008-RHI | TenEngine::rhi | IDevice | 约定 | GPU 线程约束 | TenEngine/rhi/Device.h | — | 所有 IDevice 的 GPU 资源创建、CommandBuffer 录制与提交必须在线程 D 执行 |
| 008-RHI | TenEngine::rhi | IDevice | 抽象接口 | **创建 GPU 资源**（显式控制位置） | TenEngine/rhi/Device.h | IDevice::CreateDeviceResource | `IDeviceResource* CreateDeviceResource(DeviceResourceType type, void const* desc);` type：Buffer/Texture/Sampler/PSO/DescriptorSetLayout/DescriptorSet；desc 指向对应描述结构；失败返回 nullptr；必须在线程 D 调用 |
| 008-RHI | TenEngine::rhi | — | 枚举 | 设备资源类型 | TenEngine/rhi/DeviceResource.h | DeviceResourceType | `enum class DeviceResourceType { Buffer, Texture, Sampler, PSO, DescriptorSetLayout, DescriptorSet };` |
| 008-RHI | TenEngine::rhi | — | struct | 缓冲创建描述 | TenEngine/rhi/DeviceResource.h | BufferDesc | size、usage（Vertex/Index/Uniform/Staging）、stride 等；CreateDeviceResource(type=Buffer, desc=BufferDesc*) 时使用 |
| 008-RHI | TenEngine::rhi | — | struct | 纹理创建描述 | TenEngine/rhi/DeviceResource.h | TextureDesc | width、height、depth、format、mipLevels、usage（SRV/RTV/DSV）等；CreateDeviceResource(type=Texture, …) 时使用 |
| 008-RHI | TenEngine::rhi | — | struct | 采样器创建描述 | TenEngine/rhi/DeviceResource.h | SamplerDesc | filter、addressMode、anisotropy 等；CreateDeviceResource(type=Sampler, …) 时使用 |
| 008-RHI | TenEngine::rhi | — | struct | PSO 创建描述 | TenEngine/rhi/DeviceResource.h | PSODesc | 图形/计算、Shader 字节码、顶点布局、混合/深度/光栅状态等；CreateDeviceResource(type=PSO, …) 时使用 |
| 008-RHI | TenEngine::rhi | — | struct | **描述符集布局**创建描述 | TenEngine/rhi/DeviceResource.h | **DescriptorSetLayoutDesc** | 槽位（binding）列表：每槽位类型（UniformBuffer/Texture/Sampler/CombinedImageSampler 等）、数量、stage；CreateDeviceResource(type=DescriptorSetLayout, desc=…) 时使用；Vulkan 对应 VkDescriptorSetLayout |
| 008-RHI | TenEngine::rhi | IDescriptorSetLayout | 抽象接口 | 描述符集布局句柄 | TenEngine/rhi/DescriptorSet.h | IDescriptorSetLayout | 由 CreateDeviceResource(type=DescriptorSetLayout, …) 返回；不直接构造；供创建 DescriptorSet 时引用 |
| 008-RHI | TenEngine::rhi | — | struct | 描述符集创建描述 | TenEngine/rhi/DeviceResource.h | DescriptorSetDesc | layout：IDescriptorSetLayout*；pool 或 per-frame 策略（可选）；CreateDeviceResource(type=DescriptorSet, …) 时使用 |
| 008-RHI | TenEngine::rhi | IDevice | 抽象接口 | **更新 GPU 资源**（显式控制位置） | TenEngine/rhi/Device.h | IDevice::UpdateDeviceResource | `void UpdateDeviceResource(IDeviceResource* resource, void const* data, size_t size);` 或重载（纹理子资源、描述符写入等）；必须在线程 D 调用 |
| 008-RHI | TenEngine::rhi | — | 自由函数 | 销毁设备 | TenEngine/rhi/Device.h | DestroyDevice | `void DestroyDevice(IDevice* device);` 释放设备及未释放资源；调用方停止使用后调用 |
| 008-RHI | TenEngine::rhi | IBuffer | 抽象接口 | GPU 缓冲 | TenEngine/rhi/Buffer.h | IBuffer | 顶点/索引/Uniform 等缓冲；由 IDevice::CreateDeviceResource 创建；不直接构造 |
| 008-RHI | TenEngine::rhi | ITexture | 抽象接口 | GPU 纹理 | TenEngine/rhi/Texture.h | ITexture | 纹理及视图（SRV/RTV/DSV 等）；由 IDevice::CreateDeviceResource 创建；不直接构造 |
| 008-RHI | TenEngine::rhi | ISampler | 抽象接口 | 采样器 | TenEngine/rhi/Sampler.h | ISampler | 采样器；由 IDevice::CreateDeviceResource 创建；不直接构造 |
| 008-RHI | TenEngine::rhi | IPSO | 抽象接口 | 管线状态对象 | TenEngine/rhi/PSO.h | IPSO | 图形/计算 PSO；与 Shader 字节码绑定；由 IDevice::CreateDeviceResource 创建；可缓存 |

*来源：用户故事 US-002（一帧渲染）、US-004（流水线式多帧渲染）、US-rendering-004（多线程管线阶段；线程 D = 唯一 GPU/Device 线程）。契约能力：设备与队列、资源管理、PSO、同步。*
