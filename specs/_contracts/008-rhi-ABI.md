# 008-RHI 模块 ABI

- **契约**：[008-rhi-public-api.md](./008-rhi-public-api.md)（能力与类型描述）
- **本文件**：008-RHI 对外 ABI 显式表。
- **CMake Target 名称**：**`te_rhi`**（project name `te_rhi`，不是 `TenEngine_RHI`）。下游在 `target_link_libraries` 中应使用 **`te_rhi`**，不是 `TenEngine_RHI`。依赖 `te_core`。
- **平台与接口**：引擎支持 **Android、iOS** 等平台（见 001-Core）；RHI 支持 **Vulkan**、**Metal（MTL）**、**D3D12/DXIL** 等图形接口，及 **GLSL**、**HLSL/DXIL**、**MSL** 等 Shader 接口。**可以通过宏来判断执行哪一段代码**（如 TE_RHI_VULKAN、TE_RHI_METAL、TE_RHI_D3D12），编译时选择后端实现路径。
- **参考**：Vulkan（vkCmd*、VkBuffer/VkImage/VkDescriptorSet、vkCmdPipelineBarrier）、Metal（MTLCommandEncoder、MTLBuffer/MTLTexture、setVertexBuffer/setFragmentTexture、drawIndexedPrimitives）、D3D12（ID3D12GraphicsCommandList、ResourceBarrier、DrawIndexedInstanced）、Unreal FRHICommandList、Unity 底层图形封装。
- **渲染资源显式控制位置**：**准备/创建/更新 GPU 资源**（**CreateDeviceResource**、**UpdateDeviceResource**）由本模块提供；**提交到实际 GPU Command**（**SubmitCommandBuffer**）见 ExecuteLogicalCommandBuffer / SubmitCommandList。创建逻辑渲染资源、CollectCommandBuffer、PrepareRenderMaterial/Mesh 见 019-PipelineCore/020-Pipeline。

## ABI 表

列定义：**模块名 | 命名空间 | 符号/类型 | 导出形式 | 接口说明 | 头文件 | 说明**

### 类型与枚举（te/rhi/types.hpp）

| 模块名 | 命名空间 | 符号 | 导出形式 | 接口说明 | 头文件 | 说明 |
|--------|----------|------|----------|----------|--------|------|
| 008-RHI | te::rhi | Backend | 枚举 | 图形后端 | te/rhi/types.hpp | `enum class Backend : unsigned { Vulkan = 0, D3D12 = 1, Metal = 2, D3D11 = 3 };` |
| 008-RHI | te::rhi | DeviceLimits | struct | 设备限制 | te/rhi/types.hpp | `maxBufferSize`, `maxTextureDimension2D`, `maxTextureDimension3D`, `minUniformBufferOffsetAlignment` (size_t/uint32_t) |
| 008-RHI | te::rhi | QueueType | 枚举 | 队列类型 | te/rhi/types.hpp | `enum class QueueType : unsigned { Graphics = 0, Compute = 1, Copy = 2 };` |
| 008-RHI | te::rhi | DeviceFeatures | struct | 设备特性（最小集） | te/rhi/types.hpp | `maxTextureDimension2D`, `maxTextureDimension3D` (uint32_t) |
| 008-RHI | te::rhi | ResourceState | 枚举 | 资源状态（屏障用） | te/rhi/types.hpp | `enum class ResourceState : uint32_t { Common, VertexBuffer, IndexBuffer, RenderTarget, DepthWrite, ShaderResource, CopySrc, CopyDst, Present };` |
| 008-RHI | te::rhi | BufferBarrier | struct | 缓冲屏障 | te/rhi/types.hpp | `IBuffer* buffer; size_t offset, size; ResourceState srcState, dstState;` |
| 008-RHI | te::rhi | TextureBarrier | struct | 纹理屏障 | te/rhi/types.hpp | `ITexture* texture; uint32_t mipLevel, arrayLayer; ResourceState srcState, dstState;` |
| 008-RHI | te::rhi | IDevice, IQueue, ICommandList, IBuffer, ITexture, ISampler, IPSO, IFence, ISemaphore, ISwapChain | 前向声明 | 句柄类型 | te/rhi/types.hpp | 仅前向声明；定义在对应头文件 |

### 资源描述（te/rhi/resources.hpp）

| 模块名 | 命名空间 | 符号 | 导出形式 | 接口说明 | 头文件 | 说明 |
|--------|----------|------|----------|----------|--------|------|
| 008-RHI | te::rhi | BufferUsage | 枚举 | 缓冲用途位 | te/rhi/resources.hpp | `enum class BufferUsage : uint32_t { Vertex = 1u << 0, Index = 1u << 1, Uniform = 1u << 2, Storage = 1u << 3, CopySrc = 1u << 4, CopyDst = 1u << 5 };` BufferDesc.usage 使用此位掩码；CreateUniformBuffer 等须传 Uniform 位 |
| 008-RHI | te::rhi | BufferDesc | struct | 缓冲创建描述 | te/rhi/resources.hpp | `size_t size; uint32_t usage;` **usage 为 BufferUsage 位掩码；含 Uniform 时表示可用于 Uniform 缓冲** |
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
| 008-RHI | te::rhi | IDevice::GetLimits | 成员函数 | 设备限制 | te/rhi/device.hpp | `DeviceLimits const& GetLimits() const = 0;` |
| 008-RHI | te::rhi | IDevice::CreateCommandList | 成员函数 | 创建命令列表 | te/rhi/device.hpp | `ICommandList* CreateCommandList() = 0;` 失败返回 nullptr |
| 008-RHI | te::rhi | IDevice::DestroyCommandList | 成员函数 | 销毁命令列表 | te/rhi/device.hpp | `void DestroyCommandList(ICommandList* cmd) = 0;` |
| 008-RHI | te::rhi | IDevice::CreateBuffer | 成员函数 | 创建缓冲 | te/rhi/device.hpp | `IBuffer* CreateBuffer(BufferDesc const& desc) = 0;` 失败返回 nullptr |
| 008-RHI | te::rhi | IDevice::UpdateBuffer | 成员函数 | CPU 写入 GPU 缓冲 | te/rhi/device.hpp | `void UpdateBuffer(IBuffer* buf, size_t offset, void const* data, size_t size) = 0;` 满足 009 UniformBuffer::Update |
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
| 008-RHI | te::rhi | IDevice::CreateFence | 成员函数 | 创建 Fence | te/rhi/device.hpp | `IFence* CreateFence(bool initialSignaled = false) = 0;` 失败返回 nullptr |
| 008-RHI | te::rhi | IDevice::CreateSemaphore | 成员函数 | 创建 Semaphore | te/rhi/device.hpp | `ISemaphore* CreateSemaphore() = 0;` 失败返回 nullptr |
| 008-RHI | te::rhi | IDevice::DestroyFence | 成员函数 | 销毁 Fence | te/rhi/device.hpp | `void DestroyFence(IFence* f) = 0;` |
| 008-RHI | te::rhi | IDevice::DestroySemaphore | 成员函数 | 销毁 Semaphore | te/rhi/device.hpp | `void DestroySemaphore(ISemaphore* s) = 0;` |
| 008-RHI | te::rhi | IDevice::CreateSwapChain | 成员函数 | 创建交换链 | te/rhi/device.hpp | `ISwapChain* CreateSwapChain(SwapChainDesc const& desc) = 0;` 失败返回 nullptr |
| 008-RHI | te::rhi | IDevice::CreateDescriptorSetLayout | 成员函数 | 创建描述符集布局 | te/rhi/device.hpp | `IDescriptorSetLayout* CreateDescriptorSetLayout(DescriptorSetLayoutDesc const& desc) = 0;` 失败返回 nullptr（P2） |
| 008-RHI | te::rhi | IDevice::AllocateDescriptorSet | 成员函数 | 分配描述符集 | te/rhi/device.hpp | `IDescriptorSet* AllocateDescriptorSet(IDescriptorSetLayout* layout) = 0;` 失败返回 nullptr（P2） |
| 008-RHI | te::rhi | IDevice::UpdateDescriptorSet | 成员函数 | 更新描述符 | te/rhi/device.hpp | `void UpdateDescriptorSet(IDescriptorSet* set, DescriptorWrite const* writes, uint32_t writeCount) = 0;`（P2） |
| 008-RHI | te::rhi | IDevice::DestroyDescriptorSetLayout | 成员函数 | 销毁描述符集布局 | te/rhi/device.hpp | `void DestroyDescriptorSetLayout(IDescriptorSetLayout* layout) = 0;`（P2） |
| 008-RHI | te::rhi | IDevice::DestroyDescriptorSet | 成员函数 | 销毁描述符集 | te/rhi/device.hpp | `void DestroyDescriptorSet(IDescriptorSet* set) = 0;`（P2） |
| 008-RHI | te::rhi | IQueue | 抽象接口 | 队列 | te/rhi/queue.hpp | 见下表 IQueue 成员 |
| 008-RHI | te::rhi | IQueue::Submit | 成员函数 | 提交命令列表 | te/rhi/queue.hpp | `void Submit(ICommandList* cmdList, IFence* signalFence = nullptr, ISemaphore* waitSemaphore = nullptr, ISemaphore* signalSemaphore = nullptr) = 0;` |
| 008-RHI | te::rhi | IQueue::WaitIdle | 成员函数 | 等待队列空闲 | te/rhi/queue.hpp | `void WaitIdle() = 0;` |

### 命令列表（te/rhi/command_list.hpp）

| 模块名 | 命名空间 | 符号 | 导出形式 | 接口说明 | 头文件 | 说明 |
|--------|----------|------|----------|----------|--------|------|
| 008-RHI | te::rhi | ICommandList | 抽象接口 | 命令缓冲 | te/rhi/command_list.hpp | 见下表 ICommandList 成员 |
| 008-RHI | te::rhi | ICommandList::Begin | 成员函数 | 开始录制 | te/rhi/command_list.hpp | `void Begin() = 0;` |
| 008-RHI | te::rhi | ICommandList::End | 成员函数 | 结束录制 | te/rhi/command_list.hpp | `void End() = 0;` |
| 008-RHI | te::rhi | LoadOp | 枚举 | 渲染附件加载操作 | te/rhi/command_list.hpp | `enum class LoadOp : uint32_t { Load = 0, Clear = 1, DontCare = 2 };` |
| 008-RHI | te::rhi | StoreOp | 枚举 | 渲染附件存储操作 | te/rhi/command_list.hpp | `enum class StoreOp : uint32_t { Store = 0, DontCare = 1 };` |
| 008-RHI | te::rhi | kMaxColorAttachments | 常量 | 颜色附件最大数量 | te/rhi/command_list.hpp | `constexpr uint32_t kMaxColorAttachments = 8u;` |
| 008-RHI | te::rhi | Viewport | struct | 视口 | te/rhi/command_list.hpp | x, y, width, height, minDepth, maxDepth (float) |
| 008-RHI | te::rhi | ScissorRect | struct | 裁剪矩形 | te/rhi/command_list.hpp | x, y, width, height (int32_t/uint32_t) |
| 008-RHI | te::rhi | RenderPassDesc | struct | 渲染通道描述（P2） | te/rhi/command_list.hpp | colorAttachmentCount, colorAttachments[kMaxColorAttachments], depthStencilAttachment, colorLoadOp/colorStoreOp, depthLoadOp/depthStoreOp, clearColor/clearDepth/clearStencil |
| 008-RHI | te::rhi | BufferRegion, TextureRegion | struct | 拷贝区域（P2） | te/rhi/command_list.hpp | CopyBuffer/CopyBufferToTexture/CopyTextureToBuffer 用 |
| 008-RHI | te::rhi | ICommandList::Draw | 成员函数 | 非索引绘制 | te/rhi/command_list.hpp | `void Draw(uint32_t vertex_count, uint32_t instance_count = 1, uint32_t first_vertex = 0, uint32_t first_instance = 0) = 0;` |
| 008-RHI | te::rhi | ICommandList::DrawIndexed | 成员函数 | 索引绘制 | te/rhi/command_list.hpp | `void DrawIndexed(uint32_t index_count, uint32_t instance_count = 1, uint32_t first_index = 0, int32_t vertex_offset = 0, uint32_t first_instance = 0) = 0;` |
| 008-RHI | te::rhi | ICommandList::SetViewport | 成员函数 | 设置视口 | te/rhi/command_list.hpp | `void SetViewport(uint32_t first, uint32_t count, Viewport const* viewports) = 0;` |
| 008-RHI | te::rhi | ICommandList::SetScissor | 成员函数 | 设置裁剪 | te/rhi/command_list.hpp | `void SetScissor(uint32_t first, uint32_t count, ScissorRect const* scissors) = 0;` |
| 008-RHI | te::rhi | ICommandList::SetUniformBuffer | 成员函数 | 将 IBuffer 绑定到 slot | te/rhi/command_list.hpp | `void SetUniformBuffer(uint32_t slot, IBuffer* buffer, size_t offset) = 0;` 满足 009 IUniformBuffer::Bind；slot 越界或 buffer==nullptr 行为由实现约定 |
| 008-RHI | te::rhi | ICommandList::SetVertexBuffer | 成员函数 | 绑定顶点缓冲 | te/rhi/command_list.hpp | `void SetVertexBuffer(uint32_t slot, IBuffer* buffer, size_t offset, uint32_t stride) = 0;` 020 在每条 Draw 前按 mesh 绑定 |
| 008-RHI | te::rhi | ICommandList::SetIndexBuffer | 成员函数 | 绑定索引缓冲 | te/rhi/command_list.hpp | `void SetIndexBuffer(IBuffer* buffer, size_t offset, uint32_t indexFormat) = 0;` indexFormat: 0=16bit, 1=32bit |
| 008-RHI | te::rhi | ICommandList::SetGraphicsPSO | 成员函数 | 绑定图形 PSO | te/rhi/command_list.hpp | `void SetGraphicsPSO(IPSO* pso) = 0;` 每条 Draw 前按材质绑定；pso 可为 nullptr 表示解绑 |
| 008-RHI | te::rhi | ICommandList::BeginRenderPass | 成员函数 | 开始渲染通道（P2） | te/rhi/command_list.hpp | `void BeginRenderPass(RenderPassDesc const& desc) = 0;` |
| 008-RHI | te::rhi | ICommandList::EndRenderPass | 成员函数 | 结束渲染通道（P2） | te/rhi/command_list.hpp | `void EndRenderPass() = 0;` |
| 008-RHI | te::rhi | ICommandList::BeginOcclusionQuery | 成员函数 | 开始遮挡查询 | te/rhi/command_list.hpp | `void BeginOcclusionQuery(uint32_t queryIndex) = 0;` 与 EndOcclusionQuery 成对；当前后端可为 no-op |
| 008-RHI | te::rhi | ICommandList::EndOcclusionQuery | 成员函数 | 结束遮挡查询 | te/rhi/command_list.hpp | `void EndOcclusionQuery(uint32_t queryIndex) = 0;` |
| 008-RHI | te::rhi | ICommandList::CopyBuffer | 成员函数 | 缓冲间拷贝（P2） | te/rhi/command_list.hpp | `void CopyBuffer(IBuffer* src, size_t srcOffset, IBuffer* dst, size_t dstOffset, size_t size) = 0;` |
| 008-RHI | te::rhi | ICommandList::CopyBufferToTexture | 成员函数 | 缓冲到纹理（P2） | te/rhi/command_list.hpp | `void CopyBufferToTexture(IBuffer* src, size_t srcOffset, ITexture* dst, TextureRegion const& dstRegion) = 0;` |
| 008-RHI | te::rhi | ICommandList::CopyTextureToBuffer | 成员函数 | 纹理到缓冲（P2） | te/rhi/command_list.hpp | `void CopyTextureToBuffer(ITexture* src, TextureRegion const& srcRegion, IBuffer* dst, size_t dstOffset) = 0;` |
| 008-RHI | te::rhi | ICommandList::BuildAccelerationStructure | 成员函数 | 构建光追加速结构（D3D12） | te/rhi/command_list.hpp | `void BuildAccelerationStructure(RaytracingAccelerationStructureDesc const& desc, IBuffer* scratch, IBuffer* result) = 0;` |
| 008-RHI | te::rhi | ICommandList::DispatchRays | 成员函数 | 派发光线（D3D12） | te/rhi/command_list.hpp | `void DispatchRays(DispatchRaysDesc const& desc) = 0;` |
| 008-RHI | te::rhi | ICommandList::Dispatch | 成员函数 | 派发计算 | te/rhi/command_list.hpp | `void Dispatch(uint32_t x, uint32_t y = 1, uint32_t z = 1) = 0;` |
| 008-RHI | te::rhi | ICommandList::Copy | 成员函数 | 内存拷贝 | te/rhi/command_list.hpp | `void Copy(void const* src, void* dst, size_t size) = 0;` |
| 008-RHI | te::rhi | ICommandList::ResourceBarrier | 成员函数 | 资源屏障 | te/rhi/command_list.hpp | `void ResourceBarrier(uint32_t bufferBarrierCount, BufferBarrier const* bufferBarriers, uint32_t textureBarrierCount, TextureBarrier const* textureBarriers) = 0;` |
| 008-RHI | te::rhi | Begin | 自由函数 | 开始录制 | te/rhi/command_list.hpp | `void Begin(ICommandList* cmd);` |
| 008-RHI | te::rhi | End | 自由函数 | 结束录制 | te/rhi/command_list.hpp | `void End(ICommandList* cmd);` |
| 008-RHI | te::rhi | Submit | 自由函数 | 提交到队列 | te/rhi/command_list.hpp | `void Submit(ICommandList* cmd, IQueue* queue);` 或 `void Submit(ICommandList* cmd, IQueue* queue, IFence* signalFence, ISemaphore* waitSem, ISemaphore* signalSem);` |

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

### 描述符集（te/rhi/descriptor_set.hpp）

| 模块名 | 命名空间 | 符号 | 导出形式 | 接口说明 | 头文件 | 说明 |
|--------|----------|------|----------|----------|--------|------|
| 008-RHI | te::rhi | DescriptorSetLayoutDesc, DescriptorWrite | struct | 描述符布局/写入 | te/rhi/descriptor_set.hpp | bindings[]; dstSet, binding, type, buffer/texture/sampler |
| 008-RHI | te::rhi | IDescriptorSetLayout, IDescriptorSet | 抽象接口 | 描述符集布局/集 | te/rhi/descriptor_set.hpp | 虚析构 |

### 光追（te/rhi/raytracing.hpp）（仅 D3D12）

| 模块名 | 命名空间 | 符号 | 导出形式 | 接口说明 | 头文件 | 说明 |
|--------|----------|------|----------|----------|--------|------|
| 008-RHI | te::rhi | RaytracingAccelerationStructureDesc, DispatchRaysDesc | struct | 加速结构/派发描述 | te/rhi/raytracing.hpp | 映射 D3D12 BLAS/TLAS、D3D12_DISPATCH_RAYS_DESC |


## 已实现（原 TODO，008-rhi-fullmodule-006）

以下能力已在 008-rhi-fullmodule-006 中实现并写入本 ABI 表：

- **Buffer CPU 写入**：IDevice::UpdateBuffer(IBuffer* buf, size_t offset, void const* data, size_t size) 已实现；满足 009 UniformBuffer::Update。
- **Uniform 绑定到命令列表 slot**：ICommandList::SetUniformBuffer(uint32_t slot, IBuffer* buffer, size_t offset) 已实现；满足 009 IUniformBuffer::Bind。
- **BufferDesc 的 Uniform 用途**：BufferUsage 枚举（含 Uniform 位）与 BufferDesc.usage 语义已补充；CreateBuffer(BufferDesc{ usage 含 Uniform }) 可创建 Uniform 缓冲。
- **描述符与 RHI 对接**：009 与 008 的 BufferDesc/TextureDesc、VertexFormat/IndexFormat 对接约定见 specs/008-rhi-fullmodule-006/009-008-descriptor-convention.md。

---

*本 ABI 表已合并 008-rhi-fullmodule-004 增量；已补充 LoadOp/StoreOp/kMaxColorAttachments 及按后端的实现状态表；Copy/RenderPass/DescriptorSet 实现说明已更新；新增「不能实现或保留占位及原因」小节。*

---

数据与接口 TODO 已迁移至本模块契约 [008-rhi-public-api.md](./008-rhi-public-api.md) 的 TODO 列表；本文件仅保留 ABI 表与已实现说明。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| 2026-02-10 | ABI 同步：ICommandList 增加 SetVertexBuffer、SetIndexBuffer、SetGraphicsPSO、BeginOcclusionQuery、EndOcclusionQuery |
