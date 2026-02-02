# 008-RHI 模块 ABI

- **契约**：[008-rhi-public-api.md](./008-rhi-public-api.md)（能力与类型描述）
- **本文件**：008-RHI 对外 ABI 显式表。
- **平台与接口**：引擎支持 **Android、iOS** 等平台（见 001-Core）；RHI 支持 **Vulkan**、**Metal（MTL）**、**D3D12/DXIL** 等图形接口，及 **GLSL**、**HLSL/DXIL**、**MSL** 等 Shader 接口。**可以通过宏来判断执行哪一段代码**（如 TE_RHI_VULKAN、TE_RHI_METAL、TE_RHI_D3D12），编译时选择后端实现路径。
- **参考**：Vulkan（vkCmd*、VkBuffer/VkImage/VkDescriptorSet、vkCmdPipelineBarrier）、Metal（MTLCommandEncoder、MTLBuffer/MTLTexture、setVertexBuffer/setFragmentTexture、drawIndexedPrimitives）、D3D12（ID3D12GraphicsCommandList、ResourceBarrier、DrawIndexedInstanced）、Unreal FRHICommandList、Unity 底层图形封装。
- **渲染资源显式控制位置**：**准备/创建/更新 GPU 资源**（**CreateDeviceResource**、**UpdateDeviceResource**）由本模块提供；**提交到实际 GPU Command**（**SubmitCommandBuffer**）见 ExecuteLogicalCommandBuffer / SubmitCommandList。创建逻辑渲染资源、CollectCommandBuffer、PrepareRenderMaterial/Mesh 见 019-PipelineCore/020-Pipeline。

## ABI 表

列定义：**模块名 | 命名空间 | 符号/类型 | 导出形式 | 接口说明 | 头文件 | 说明**

### 类型与枚举（te/rhi/types.hpp）

| 模块名 | 命名空间 | 符号 | 导出形式 | 接口说明 | 头文件 | 说明 |
|--------|----------|------|----------|----------|--------|------|
| 008-RHI | te::rhi | Backend | 枚举 | 图形后端 | te/rhi/types.hpp | `enum class Backend : unsigned { Vulkan = 0, D3D12 = 1, Metal = 2 };` |
| 008-RHI | te::rhi | QueueType | 枚举 | 队列类型 | te/rhi/types.hpp | `enum class QueueType : unsigned { Graphics = 0, Compute = 1, Copy = 2 };` |
| 008-RHI | te::rhi | DeviceFeatures | struct | 设备特性（最小集） | te/rhi/types.hpp | `maxTextureDimension2D`, `maxTextureDimension3D` (uint32_t) |
| 008-RHI | te::rhi | ResourceState | 枚举 | 资源状态（屏障用） | te/rhi/types.hpp | `enum class ResourceState : uint32_t { Common, VertexBuffer, IndexBuffer, RenderTarget, DepthWrite, ShaderResource, CopySrc, CopyDst, Present };` |
| 008-RHI | te::rhi | BufferBarrier | struct | 缓冲屏障 | te/rhi/types.hpp | `IBuffer* buffer; size_t offset, size; ResourceState srcState, dstState;` |
| 008-RHI | te::rhi | TextureBarrier | struct | 纹理屏障 | te/rhi/types.hpp | `ITexture* texture; uint32_t mipLevel, arrayLayer; ResourceState srcState, dstState;` |
| 008-RHI | te::rhi | IDevice, IQueue, ICommandList, IBuffer, ITexture, ISampler, IPSO, IFence, ISemaphore, ISwapChain | 前向声明 | 句柄类型 | te/rhi/types.hpp | 仅前向声明；定义在对应头文件 |

### 资源描述（te/rhi/resources.hpp）

| 模块名 | 命名空间 | 符号 | 导出形式 | 接口说明 | 头文件 | 说明 |
|--------|----------|------|----------|----------|--------|------|
| 008-RHI | te::rhi | BufferDesc | struct | 缓冲创建描述 | te/rhi/resources.hpp | `size_t size; uint32_t usage;` |
| 008-RHI | te::rhi | TextureDesc | struct | 纹理创建描述 | te/rhi/resources.hpp | `uint32_t width, height, depth, format;` |
| 008-RHI | te::rhi | SamplerDesc | struct | 采样器创建描述 | te/rhi/resources.hpp | `uint32_t filter;` |
| 008-RHI | te::rhi | ViewDesc | struct | 视图创建描述 | te/rhi/resources.hpp | `void* resource; uint32_t type;` |
| 008-RHI | te::rhi | ViewHandle | 类型别名 | 视图句柄 | te/rhi/resources.hpp | `using ViewHandle = uintptr_t;` |
| 008-RHI | te::rhi | IBuffer | 抽象接口 | GPU 缓冲 | te/rhi/resources.hpp | 虚析构；无其他成员 |
| 008-RHI | te::rhi | ITexture | 抽象接口 | GPU 纹理 | te/rhi/resources.hpp | 虚析构；无其他成员 |
| 008-RHI | te::rhi | ISampler | 抽象接口 | 采样器 | te/rhi/resources.hpp | 虚析构；无其他成员 |

### PSO（te/rhi/pso.hpp）

| 模块名 | 命名空间 | 符号 | 导出形式 | 接口说明 | 头文件 | 说明 |
|--------|----------|------|----------|----------|--------|------|
| 008-RHI | te::rhi | GraphicsPSODesc | struct | 图形 PSO 描述 | te/rhi/pso.hpp | `void const* vertex_shader; size_t vertex_shader_size; void const* fragment_shader; size_t fragment_shader_size;` |
| 008-RHI | te::rhi | ComputePSODesc | struct | 计算 PSO 描述 | te/rhi/pso.hpp | `void const* compute_shader; size_t compute_shader_size;` |
| 008-RHI | te::rhi | IPSO | 抽象接口 | 管线状态对象 | te/rhi/pso.hpp | 虚析构；无其他成员 |

### 设备与队列（te/rhi/device.hpp, te/rhi/queue.hpp）

| 模块名 | 命名空间 | 符号 | 导出形式 | 接口说明 | 头文件 | 说明 |
|--------|----------|------|----------|----------|--------|------|
| 008-RHI | te::rhi | SelectBackend | 自由函数 | 设置默认后端 | te/rhi/device.hpp | `void SelectBackend(Backend b);` |
| 008-RHI | te::rhi | GetSelectedBackend | 自由函数 | 获取当前默认后端 | te/rhi/device.hpp | `Backend GetSelectedBackend();` |
| 008-RHI | te::rhi | CreateDevice | 自由函数 | 创建设备 | te/rhi/device.hpp | `IDevice* CreateDevice(Backend backend);` 失败返回 nullptr |
| 008-RHI | te::rhi | CreateDevice | 自由函数 | 创建设备（无参） | te/rhi/device.hpp | `IDevice* CreateDevice();` 使用 GetSelectedBackend() |
| 008-RHI | te::rhi | DestroyDevice | 自由函数 | 销毁设备 | te/rhi/device.hpp | `void DestroyDevice(IDevice* device);` nullptr 为 no-op |
| 008-RHI | te::rhi | IDevice | 抽象接口 | 图形设备 | te/rhi/device.hpp | 见下表 IDevice 成员 |
| 008-RHI | te::rhi | IDevice::GetBackend | 成员函数 | 返回创建时后端 | te/rhi/device.hpp | `Backend GetBackend() const = 0;` |
| 008-RHI | te::rhi | IDevice::GetQueue | 成员函数 | 获取队列 | te/rhi/device.hpp | `IQueue* GetQueue(QueueType type, uint32_t index) = 0;` 越界返回 nullptr |
| 008-RHI | te::rhi | IDevice::GetFeatures | 成员函数 | 设备特性 | te/rhi/device.hpp | `DeviceFeatures const& GetFeatures() const = 0;` |
| 008-RHI | te::rhi | IDevice::CreateCommandList | 成员函数 | 创建命令列表 | te/rhi/device.hpp | `ICommandList* CreateCommandList() = 0;` 失败返回 nullptr |
| 008-RHI | te::rhi | IDevice::DestroyCommandList | 成员函数 | 销毁命令列表 | te/rhi/device.hpp | `void DestroyCommandList(ICommandList* cmd) = 0;` |
| 008-RHI | te::rhi | IDevice::CreateBuffer | 成员函数 | 创建缓冲 | te/rhi/device.hpp | `IBuffer* CreateBuffer(BufferDesc const& desc) = 0;` 失败返回 nullptr |
| 008-RHI | te::rhi | IDevice::CreateTexture | 成员函数 | 创建纹理 | te/rhi/device.hpp | `ITexture* CreateTexture(TextureDesc const& desc) = 0;` 失败返回 nullptr |
| 008-RHI | te::rhi | IDevice::CreateSampler | 成员函数 | 创建采样器 | te/rhi/device.hpp | `ISampler* CreateSampler(SamplerDesc const& desc) = 0;` 失败返回 nullptr |
| 008-RHI | te::rhi | IDevice::CreateView | 成员函数 | 创建视图 | te/rhi/device.hpp | `ViewHandle CreateView(ViewDesc const& desc) = 0;` |
| 008-RHI | te::rhi | IDevice::DestroyBuffer | 成员函数 | 销毁缓冲 | te/rhi/device.hpp | `void DestroyBuffer(IBuffer* b) = 0;` |
| 008-RHI | te::rhi | IDevice::DestroyTexture | 成员函数 | 销毁纹理 | te/rhi/device.hpp | `void DestroyTexture(ITexture* t) = 0;` |
| 008-RHI | te::rhi | IDevice::DestroySampler | 成员函数 | 销毁采样器 | te/rhi/device.hpp | `void DestroySampler(ISampler* s) = 0;` |
| 008-RHI | te::rhi | IDevice::CreateGraphicsPSO | 成员函数 | 创建图形 PSO | te/rhi/device.hpp | `IPSO* CreateGraphicsPSO(GraphicsPSODesc const& desc) = 0;` 失败返回 nullptr |
| 008-RHI | te::rhi | IDevice::CreateComputePSO | 成员函数 | 创建计算 PSO | te/rhi/device.hpp | `IPSO* CreateComputePSO(ComputePSODesc const& desc) = 0;` 失败返回 nullptr |
| 008-RHI | te::rhi | IDevice::SetShader | 成员函数 | 绑定 shader 数据 | te/rhi/device.hpp | `void SetShader(IPSO* pso, void const* data, size_t size) = 0;` |
| 008-RHI | te::rhi | IDevice::Cache | 成员函数 | PSO 缓存 | te/rhi/device.hpp | `void Cache(IPSO* pso) = 0;` |
| 008-RHI | te::rhi | IDevice::DestroyPSO | 成员函数 | 销毁 PSO | te/rhi/device.hpp | `void DestroyPSO(IPSO* pso) = 0;` |
| 008-RHI | te::rhi | IDevice::CreateFence | 成员函数 | 创建 Fence | te/rhi/device.hpp | `IFence* CreateFence() = 0;` 失败返回 nullptr |
| 008-RHI | te::rhi | IDevice::CreateSemaphore | 成员函数 | 创建 Semaphore | te/rhi/device.hpp | `ISemaphore* CreateSemaphore() = 0;` 失败返回 nullptr |
| 008-RHI | te::rhi | IDevice::DestroyFence | 成员函数 | 销毁 Fence | te/rhi/device.hpp | `void DestroyFence(IFence* f) = 0;` |
| 008-RHI | te::rhi | IDevice::DestroySemaphore | 成员函数 | 销毁 Semaphore | te/rhi/device.hpp | `void DestroySemaphore(ISemaphore* s) = 0;` |
| 008-RHI | te::rhi | IDevice::CreateSwapChain | 成员函数 | 创建交换链 | te/rhi/device.hpp | `ISwapChain* CreateSwapChain(SwapChainDesc const& desc) = 0;` 失败返回 nullptr |
| 008-RHI | te::rhi | IQueue | 抽象接口 | 队列 | te/rhi/queue.hpp | 见下表 IQueue 成员 |
| 008-RHI | te::rhi | IQueue::Submit | 成员函数 | 提交命令列表 | te/rhi/queue.hpp | `void Submit(ICommandList* cmdList, IFence* signalFence = nullptr, ISemaphore* waitSemaphore = nullptr, ISemaphore* signalSemaphore = nullptr) = 0;` |
| 008-RHI | te::rhi | IQueue::WaitIdle | 成员函数 | 等待队列空闲 | te/rhi/queue.hpp | `void WaitIdle() = 0;` |

### 命令列表（te/rhi/command_list.hpp）

| 模块名 | 命名空间 | 符号 | 导出形式 | 接口说明 | 头文件 | 说明 |
|--------|----------|------|----------|----------|--------|------|
| 008-RHI | te::rhi | ICommandList | 抽象接口 | 命令缓冲 | te/rhi/command_list.hpp | 见下表 ICommandList 成员 |
| 008-RHI | te::rhi | ICommandList::Begin | 成员函数 | 开始录制 | te/rhi/command_list.hpp | `void Begin() = 0;` |
| 008-RHI | te::rhi | ICommandList::End | 成员函数 | 结束录制 | te/rhi/command_list.hpp | `void End() = 0;` |
| 008-RHI | te::rhi | ICommandList::Draw | 成员函数 | 非索引绘制 | te/rhi/command_list.hpp | `void Draw(uint32_t vertex_count, uint32_t instance_count = 1, uint32_t first_vertex = 0, uint32_t first_instance = 0) = 0;` |
| 008-RHI | te::rhi | ICommandList::Dispatch | 成员函数 | 派发计算 | te/rhi/command_list.hpp | `void Dispatch(uint32_t x, uint32_t y = 1, uint32_t z = 1) = 0;` |
| 008-RHI | te::rhi | ICommandList::Copy | 成员函数 | 内存拷贝 | te/rhi/command_list.hpp | `void Copy(void const* src, void* dst, size_t size) = 0;` |
| 008-RHI | te::rhi | ICommandList::ResourceBarrier | 成员函数 | 资源屏障 | te/rhi/command_list.hpp | `void ResourceBarrier(uint32_t bufferBarrierCount, BufferBarrier const* bufferBarriers, uint32_t textureBarrierCount, TextureBarrier const* textureBarriers) = 0;` |
| 008-RHI | te::rhi | Begin | 自由函数 | 开始录制 | te/rhi/command_list.hpp | `void Begin(ICommandList* cmd);` |
| 008-RHI | te::rhi | End | 自由函数 | 结束录制 | te/rhi/command_list.hpp | `void End(ICommandList* cmd);` |
| 008-RHI | te::rhi | Submit | 自由函数 | 提交到队列 | te/rhi/command_list.hpp | `void Submit(ICommandList* cmd, IQueue* queue);` 内部调用 queue->Submit(cmd, nullptr, nullptr, nullptr) |

### 同步（te/rhi/sync.hpp）

| 模块名 | 命名空间 | 符号 | 导出形式 | 接口说明 | 头文件 | 说明 |
|--------|----------|------|----------|----------|--------|------|
| 008-RHI | te::rhi | IFence | 抽象接口 | Fence | te/rhi/sync.hpp | `void Wait() = 0; void Signal() = 0; void Reset() = 0;` |
| 008-RHI | te::rhi | ISemaphore | 抽象接口 | Semaphore | te/rhi/sync.hpp | 虚析构；无其他成员 |
| 008-RHI | te::rhi | Wait | 自由函数 | Fence 等待 | te/rhi/sync.hpp | `void Wait(IFence* f);` 内部调用 f->Wait() |
| 008-RHI | te::rhi | Signal | 自由函数 | Fence 信号 | te/rhi/sync.hpp | `void Signal(IFence* f);` 内部调用 f->Signal() |

### 交换链（te/rhi/swapchain.hpp）

| 模块名 | 命名空间 | 符号 | 导出形式 | 接口说明 | 头文件 | 说明 |
|--------|----------|------|----------|----------|--------|------|
| 008-RHI | te::rhi | SwapChainDesc | struct | 交换链创建描述 | te/rhi/swapchain.hpp | `void* windowHandle; uint32_t width, height, format, bufferCount; bool vsync;` |
| 008-RHI | te::rhi | ISwapChain | 抽象接口 | 交换链 | te/rhi/swapchain.hpp | 见下表 ISwapChain 成员 |
| 008-RHI | te::rhi | ISwapChain::Present | 成员函数 | 呈现 | te/rhi/swapchain.hpp | `bool Present() = 0;` |
| 008-RHI | te::rhi | ISwapChain::GetCurrentBackBuffer | 成员函数 | 当前后备缓冲纹理 | te/rhi/swapchain.hpp | `ITexture* GetCurrentBackBuffer() = 0;` |
| 008-RHI | te::rhi | ISwapChain::GetCurrentBackBufferIndex | 成员函数 | 当前后备缓冲索引 | te/rhi/swapchain.hpp | `uint32_t GetCurrentBackBufferIndex() const = 0;` |
| 008-RHI | te::rhi | ISwapChain::Resize | 成员函数 | 调整大小 | te/rhi/swapchain.hpp | `void Resize(uint32_t width, uint32_t height) = 0;` |
| 008-RHI | te::rhi | ISwapChain::GetWidth | 成员函数 | 宽度 | te/rhi/swapchain.hpp | `uint32_t GetWidth() const = 0;` |
| 008-RHI | te::rhi | ISwapChain::GetHeight | 成员函数 | 高度 | te/rhi/swapchain.hpp | `uint32_t GetHeight() const = 0;` |

### 头文件与包含关系

| 头文件 | 依赖 | 说明 |
|--------|------|------|
| te/rhi/types.hpp | &lt;cstdint&gt; | Backend, QueueType, DeviceFeatures, ResourceState, BufferBarrier, TextureBarrier, 前向声明 |
| te/rhi/resources.hpp | te/rhi/types.hpp, &lt;cstddef&gt; | BufferDesc, TextureDesc, SamplerDesc, ViewDesc, ViewHandle, IBuffer, ITexture, ISampler |
| te/rhi/pso.hpp | te/rhi/types.hpp, &lt;cstddef&gt; | GraphicsPSODesc, ComputePSODesc, IPSO |
| te/rhi/queue.hpp | te/rhi/types.hpp | IQueue |
| te/rhi/sync.hpp | te/rhi/types.hpp | IFence, ISemaphore, Wait, Signal |
| te/rhi/swapchain.hpp | te/rhi/types.hpp, &lt;cstdint&gt; | SwapChainDesc, ISwapChain |
| te/rhi/command_list.hpp | te/rhi/types.hpp | ICommandList, Begin, End, Submit |
| te/rhi/device.hpp | te/rhi/queue.hpp, types.hpp, resources.hpp, pso.hpp, sync.hpp, swapchain.hpp | IDevice, SelectBackend, GetSelectedBackend, CreateDevice, DestroyDevice |

### 未在本次实现中导出的符号（与旧 ABI 差异说明）

以下为旧 ABI 或规约中出现、**当前实现未提供**的符号，下游若依赖须在上游 ABI 中以 TODO 登记或由后续 feature 补充：

- `DeviceDesc`、`CreateDevice(DeviceDesc const&)`：当前使用 `CreateDevice(Backend)` / `CreateDevice()`。
- `GetGraphicsQueue()` / `GetComputeQueue()` / `GetCopyQueue()`：当前使用 `GetQueue(QueueType type, uint32_t index)`。
- `GetCommandListForSlot`、`SubmitCommandList(FrameSlotId)`、`WaitForSlot`、`FrameSlotId`：当前无 slot 概念；命令列表由 `CreateCommandList()` 分配，由 `Submit(cmd, queue)` 提交。
- `CreateDeviceResource`、`UpdateDeviceResource`、`DestroyResource`、`DeviceResourceType`：当前为分立的 CreateBuffer/CreateTexture/CreateSampler/Create*PSO、Destroy*。
- `GetLimits()`、`DeviceLimits`：当前仅有 `GetFeatures()` 返回 `DeviceFeatures`（maxTextureDimension2D/3D）。
- `IDevice::DestroySwapChain`：当前未在 IDevice 中声明；交换链生命周期与设备一致或由调用方约定。
- `IFence::Wait(uint64_t timeoutNs)`、`IFence::GetStatus`、`IFence::GetCompletedValue`：当前为 `Wait()`、`Signal()`、`Reset()` 无参。
- `CreateFence(bool signaled)`：当前为 `CreateFence()` 无参。
- `ICommandList::SetPipelineState`、`SetVertexBuffers`、`SetIndexBuffer`、`SetDescriptorSet`、`SetViewport`、`SetScissor`、`BeginRenderPass`、`EndRenderPass`、`DrawIndexed`、`DrawIndexedIndirect`、`CopyBuffer`、`CopyBufferToTexture`、`CopyTextureToBuffer`、`ClearRenderTarget`、`ClearDepthStencil`：当前仅实现 Begin/End/Draw/Dispatch/Copy/ResourceBarrier。
- `IBuffer::GetSize`、`IBuffer::GetGPUAddress`；`ITexture::GetWidth`、`GetHeight`、`GetDepthOrArrayLayers`、`GetMipLevelCount`、`GetFormat`：当前 IBuffer/ITexture 为空接口（仅虚析构）。
- `IDescriptorSetLayout`、`IDescriptorSet`、`DescriptorSetLayoutDesc`、`DescriptorSetDesc`、`DescriptorWrite`：当前未实现。
- `PixelFormat`、`BufferUsage`、`TextureUsage`、`TextureType`、`PrimitiveTopology`、`PipelineBindPoint`、`Viewport`、`Rect2D`、`RenderPassDesc`、`IndexType`、`BufferTextureCopy`：当前描述符为最小集（如 BufferDesc.usage、TextureDesc.format 为 uint32_t）。

---

*本 ABI 表根据 008-rhi-fullmodule-003 当前实现（include/te/rhi/*.hpp、src/）反向更新；与 008-rhi-public-api.md 中 API 雏形对齐，差异见「未在本次实现中导出的符号」小节。*
