# 008-RHI 模块 ABI

- **契约**：[008-rhi-public-api.md](./008-rhi-public-api.md)（能力与类型描述）
- **本文件**：008-RHI 对外 ABI 显式表。
- **平台与接口**：引擎支持 **Android、iOS** 等平台（见 001-Core）；RHI 支持 **Vulkan**、**Metal（MTL）**、**D3D12/DXIL** 等图形接口，及 **GLSL**、**HLSL/DXIL**、**MSL** 等 Shader 接口。**可以通过宏来判断执行哪一段代码**（如 TE_RHI_VULKAN、TE_RHI_METAL、TE_RHI_D3D12），编译时选择后端实现路径。
- **参考**：Vulkan（vkCmd*、VkBuffer/VkImage/VkDescriptorSet、vkCmdPipelineBarrier）、Metal（MTLCommandEncoder、MTLBuffer/MTLTexture、setVertexBuffer/setFragmentTexture、drawIndexedPrimitives）、D3D12（ID3D12GraphicsCommandList、ResourceBarrier、DrawIndexedInstanced）、Unreal FRHICommandList、Unity 底层图形封装。
- **渲染资源显式控制位置**：**准备/创建/更新 GPU 资源**（**CreateDeviceResource**、**UpdateDeviceResource**）由本模块提供；**提交到实际 GPU Command**（**SubmitCommandBuffer**）见 ExecuteLogicalCommandBuffer / SubmitCommandList。创建逻辑渲染资源、CollectCommandBuffer、PrepareRenderMaterial/Mesh 见 019-PipelineCore/020-Pipeline。

## ABI 表

列定义：**模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明**

### 类型与枚举

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 008-RHI | te::rhi | BackendType | 全局枚举 | 图形后端 | te/rhi/Backend.h | BackendType::Vulkan, Metal, D3D12 | `enum class BackendType { Vulkan, Metal, D3D12 };` 或编译期宏 TE_RHI_VULKAN、TE_RHI_METAL、TE_RHI_D3D12 |
| 008-RHI | te::rhi | DeviceResourceType | 全局枚举 | 设备资源类型 | te/rhi/DeviceResource.h | DeviceResourceType::Buffer, Texture, Sampler, PSO, DescriptorSetLayout, DescriptorSet | `enum class DeviceResourceType { Buffer, Texture, Sampler, PSO, DescriptorSetLayout, DescriptorSet };` |
| 008-RHI | te::rhi | BufferUsage | 全局枚举 | 缓冲用途 | te/rhi/DeviceResource.h | BufferUsage::Vertex, Index, Uniform, Staging, CopySrc, CopyDst | Vulkan VK_BUFFER_USAGE_*；Metal MTLResourceUsage；D3D12 D3D12_RESOURCE_FLAGS |
| 008-RHI | te::rhi | TextureUsage | 全局枚举 | 纹理用途 | te/rhi/DeviceResource.h | TextureUsage::ShaderRead, RenderTarget, DepthStencil, UnorderedAccess, CopySrc, CopyDst | SRV/RTV/DSV/UAV 等；Vulkan VK_IMAGE_USAGE_*；Metal MTLTextureUsage |
| 008-RHI | te::rhi | PixelFormat | 全局枚举 | 像素/纹理格式 | te/rhi/Format.h | PixelFormat::R8G8B8A8_UNORM, D32_SFLOAT, BC1_RGBA 等 | Vulkan VkFormat；Metal MTLPixelFormat；D3D12 DXGI_FORMAT |
| 008-RHI | te::rhi | TextureType | 全局枚举 | 纹理类型 | te/rhi/DeviceResource.h | TextureType::Tex2D, TexCube, Tex3D | 1D/2D/Cube/3D；Vulkan VkImageType |
| 008-RHI | te::rhi | ResourceState | 全局枚举 | 资源状态（屏障用） | te/rhi/Barrier.h | ResourceState::Common, VertexBuffer, IndexBuffer, RenderTarget, DepthWrite, ShaderResource, CopySrc, CopyDst, Present | Vulkan VkImageLayout/VkAccessFlags；D3D12 D3D12_RESOURCE_STATES；Metal 隐式 |
| 008-RHI | te::rhi | PrimitiveTopology | 全局枚举 | 图元类型 | te/rhi/PSO.h | PrimitiveTopology::TriangleList, TriangleStrip, PointList, LineList | Vulkan VkPrimitiveTopology；Metal MTLPrimitiveType；D3D12 D3D12_PRIMITIVE_TOPOLOGY |
| 008-RHI | te::rhi | PipelineBindPoint | 全局枚举 | 管线绑定点 | te/rhi/PSO.h | PipelineBindPoint::Graphics, Compute | Vulkan VkPipelineBindPoint |
| 008-RHI | te::rhi | FrameSlotId | 类型 | 帧槽 ID | te/rhi/Device.h | FrameSlotId | 无符号整数；双缓冲/三缓冲 slot（0～frameInFlightCount-1） |
| 008-RHI | te::rhi | DeviceDesc | struct | 设备创建描述 | te/rhi/Device.h | DeviceDesc | backend、windowHandle、frameInFlightCount（2～4）、validation 等；下游按需填充 |
| 008-RHI | te::rhi | BufferDesc | struct | 缓冲创建描述 | te/rhi/DeviceResource.h | BufferDesc | size、usage（BufferUsage）、stride（可选）；CreateDeviceResource(type=Buffer, desc=BufferDesc*) 时使用 |
| 008-RHI | te::rhi | TextureDesc | struct | 纹理创建描述 | te/rhi/DeviceResource.h | TextureDesc | width、height、depthOrArrayLayers、format（PixelFormat）、mipLevels、usage（TextureUsage）、type（TextureType）；CreateDeviceResource(type=Texture, …) 时使用 |
| 008-RHI | te::rhi | SamplerDesc | struct | 采样器创建描述 | te/rhi/DeviceResource.h | SamplerDesc | minFilter、magFilter、mipmapMode、addressModeU/V/W、anisotropy、borderColor 等；Vulkan VkSamplerCreateInfo；Metal MTLSamplerDescriptor |
| 008-RHI | te::rhi | PSODesc | struct | PSO 创建描述 | te/rhi/DeviceResource.h | PSODesc | pipelineType（Graphics/Compute）、vertexShader、fragmentShader、computeShader（字节码）、vertexInputLayout、blend、depthStencil、rasterizer、primitiveTopology 等；Vulkan VkGraphicsPipelineCreateInfo；Metal MTLRenderPipelineDescriptor |
| 008-RHI | te::rhi | DescriptorSetLayoutDesc | struct | 描述符集布局创建描述 | te/rhi/DeviceResource.h | DescriptorSetLayoutDesc | bindings[]：binding、type（UniformBuffer/Texture/Sampler/CombinedImageSampler/StorageBuffer 等）、count、stage（Vertex/Fragment/Compute）；Vulkan VkDescriptorSetLayoutCreateInfo |
| 008-RHI | te::rhi | DescriptorSetDesc | struct | 描述符集创建描述 | te/rhi/DeviceResource.h | DescriptorSetDesc | layout（IDescriptorSetLayout*）、pool 或 per-frame 策略（可选）；CreateDeviceResource(type=DescriptorSet, …) 时使用 |
| 008-RHI | te::rhi | Viewport | struct | 视口 | te/rhi/CommandList.h | Viewport | x、y、width、height、minDepth、maxDepth |
| 008-RHI | te::rhi | Rect2D | struct | 裁剪矩形 | te/rhi/CommandList.h | Rect2D | x、y、width、height（或 offset + extent） |
| 008-RHI | te::rhi | RenderPassDesc | struct | 渲染通道描述 | te/rhi/CommandList.h | RenderPassDesc | colorAttachments[]、depthStencilAttachment、loadOp、storeOp、clearValue；Vulkan VkRenderPassBeginInfo；Metal MTLRenderPassDescriptor |
| 008-RHI | te::rhi | BufferBarrier | struct | 缓冲屏障 | te/rhi/Barrier.h | BufferBarrier | buffer、offset、size、srcState、dstState |
| 008-RHI | te::rhi | TextureBarrier | struct | 纹理屏障 | te/rhi/Barrier.h | TextureBarrier | texture、mipLevel、arrayLayer、srcState、dstState |
| 008-RHI | te::rhi | DescriptorWrite | struct | 描述符写入 | te/rhi/DescriptorSet.h | DescriptorWrite | binding、type、buffer/texture/sampler 句柄、offset、range 等；Vulkan VkWriteDescriptorSet |
| 008-RHI | te::rhi | IndexType | 全局枚举 | 索引类型 | te/rhi/CommandList.h | IndexType::Uint16, Uint32 | `enum class IndexType { Uint16, Uint32 };` Vulkan VkIndexType；Metal MTLIndexType |
| 008-RHI | te::rhi | BufferTextureCopy | struct | 缓冲与纹理拷贝区域 | te/rhi/CommandList.h | BufferTextureCopy | bufferOffset、bufferRowLength、bufferImageHeight、imageSubresource（mipLevel、arrayLayer）、imageOffset、imageExtent；Vulkan VkBufferImageCopy |

### 设备与队列

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 008-RHI | te::rhi | — | 自由函数 | 创建设备 | te/rhi/Device.h | CreateDevice | `IDevice* CreateDevice(DeviceDesc const& desc);` 失败返回 nullptr |
| 008-RHI | te::rhi | — | 自由函数 | 销毁设备 | te/rhi/Device.h | DestroyDevice | `void DestroyDevice(IDevice* device);` 释放设备及未释放资源；调用方停止使用后调用 |
| 008-RHI | te::rhi | IDevice | 抽象接口 | 获取图形队列 | te/rhi/Device.h | IDevice::GetGraphicsQueue | `IQueue* GetGraphicsQueue();` 用于提交绘制/计算/拷贝；调用方不拥有返回指针 |
| 008-RHI | te::rhi | IDevice | 抽象接口 | 获取计算队列（可选） | te/rhi/Device.h | IDevice::GetComputeQueue | `IQueue* GetComputeQueue();` 可选；nullptr 表示与图形队列共用 |
| 008-RHI | te::rhi | IDevice | 抽象接口 | 获取拷贝队列（可选） | te/rhi/Device.h | IDevice::GetCopyQueue | `IQueue* GetCopyQueue();` 可选；用于异步拷贝 |
| 008-RHI | te::rhi | IDevice | 抽象接口 | 设备特性/能力查询 | te/rhi/Device.h | IDevice::GetFeatures | `DeviceFeatures const& GetFeatures() const;` 或 `bool GetFeature(FeatureId id) const;` 纹理压缩、多采样等 |
| 008-RHI | te::rhi | IDevice | 抽象接口 | 设备限制查询 | te/rhi/Device.h | IDevice::GetLimits | `DeviceLimits const& GetLimits() const;` maxTextureSize、maxUniformBufferSize 等；Vulkan VkPhysicalDeviceLimits |
| 008-RHI | te::rhi | IDevice | 抽象接口 | 按 slot 获取命令列表 | te/rhi/Device.h | IDevice::GetCommandListForSlot | `ICommandList* GetCommandListForSlot(FrameSlotId slot);` 每 slot 一个 CommandList，复用前需 WaitForSlot(slot) |
| 008-RHI | te::rhi | IDevice | 抽象接口 | 提交指定 slot 的命令列表 | te/rhi/Device.h | IDevice::SubmitCommandList | `void SubmitCommandList(FrameSlotId slot);` 提交后该 slot 的 Fence signal；Present 前等待对应帧 Fence |
| 008-RHI | te::rhi | IDevice | 抽象接口 | 等待某 slot 上 GPU 完成 | te/rhi/Device.h | IDevice::WaitForSlot | `void WaitForSlot(FrameSlotId slot);` 阻塞直到该 slot 上一轮提交的 work 完成 |
| 008-RHI | te::rhi | IDevice | 抽象接口 | 在线程 D 执行逻辑 CommandBuffer 并提交 | te/rhi/Device.h | IDevice::ExecuteLogicalCommandBuffer | `void ExecuteLogicalCommandBuffer(ILogicalCommandBuffer* logical_cb);` **必须在线程 D 调用**；将 logical_cb 转成真实 ICommandList 并 submit |
| 008-RHI | te::rhi | IDevice | 抽象接口 | **创建 GPU 资源**（显式控制位置） | te/rhi/Device.h | IDevice::CreateDeviceResource | `IDeviceResource* CreateDeviceResource(DeviceResourceType type, void const* desc);` type：Buffer/Texture/Sampler/PSO/DescriptorSetLayout/DescriptorSet；desc 指向对应描述结构；失败返回 nullptr；必须在线程 D 调用 |
| 008-RHI | te::rhi | IDevice | 抽象接口 | **更新 GPU 资源**（显式控制位置） | te/rhi/Device.h | IDevice::UpdateDeviceResource | `void UpdateDeviceResource(IDeviceResource* resource, void const* data, size_t size);` 或重载（纹理子资源、描述符写入等）；必须在线程 D 调用 |
| 008-RHI | te::rhi | IDevice | 抽象接口 | **销毁 GPU 资源** | te/rhi/Device.h | IDevice::DestroyResource | `void DestroyResource(IDeviceResource* resource);` 释放 Buffer/Texture/Sampler/PSO/DescriptorSet；不得在仍被命令引用时销毁；必须在线程 D 调用 |
| 008-RHI | te::rhi | IDevice | 抽象接口 | 创建交换链 | te/rhi/Device.h | IDevice::CreateSwapChain | `ISwapChain* CreateSwapChain(SwapChainDesc const& desc);` 窗口句柄、格式、宽度高度、bufferCount；失败返回 nullptr |
| 008-RHI | te::rhi | IDevice | 抽象接口 | 创建 Fence | te/rhi/Device.h | IDevice::CreateFence | `IFence* CreateFence(bool signaled = false);` Vulkan VkFence；Metal MTLSharedEvent/MTLFence；D3D12 ID3D12Fence |
| 008-RHI | te::rhi | IDevice | 抽象接口 | 创建 Semaphore | te/rhi/Device.h | IDevice::CreateSemaphore | `ISemaphore* CreateSemaphore();` Vulkan VkSemaphore；Metal 无直接对应可用 Fence 或事件；D3D12 跨队列同步 |
| 008-RHI | te::rhi | IDevice | 约定 | GPU 线程约束 | te/rhi/Device.h | — | 所有 IDevice 的 GPU 资源创建/更新/销毁、CommandBuffer 录制与提交必须在线程 D 执行 |
| 008-RHI | te::rhi | IQueue | 抽象接口 | 提交命令列表 | te/rhi/Queue.h | IQueue::Submit | `void Submit(ICommandList* cmdList, IFence* signalFence = nullptr, ISemaphore* waitSemaphore = nullptr, ISemaphore* signalSemaphore = nullptr);` 可选 Fence/Semaphore 用于跨帧/跨队列同步 |
| 008-RHI | te::rhi | IQueue | 抽象接口 | 等待队列空闲 | te/rhi/Queue.h | IQueue::WaitIdle | `void WaitIdle();` 阻塞直到该队列上已提交的 work 全部完成；Vulkan vkQueueWaitIdle |

### 命令列表（ICommandList，对齐 Vulkan vkCmd* / Metal MTLRenderCommandEncoder / D3D12 ID3D12GraphicsCommandList）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 008-RHI | te::rhi | ICommandList | 抽象接口 | 开始录制 | te/rhi/CommandList.h | ICommandList::Begin | `void Begin();` 开始录制；End 前可多次 Set*/Draw/Dispatch/Copy/Barrier |
| 008-RHI | te::rhi | ICommandList | 抽象接口 | 结束录制 | te/rhi/CommandList.h | ICommandList::End | `void End();` 结束录制；之后可 Submit |
| 008-RHI | te::rhi | ICommandList | 抽象接口 | 提交到队列 | te/rhi/CommandList.h | ICommandList::Submit | `void Submit();` 或由 IDevice::SubmitCommandList(slot) 统一提交 |
| 008-RHI | te::rhi | ICommandList | 抽象接口 | 绑定图形/计算 PSO | te/rhi/CommandList.h | ICommandList::SetPipelineState | `void SetPipelineState(IPSO* pso);` Vulkan vkCmdBindPipeline；Metal setRenderPipelineState/setComputePipelineState |
| 008-RHI | te::rhi | ICommandList | 抽象接口 | 绑定顶点缓冲 | te/rhi/CommandList.h | ICommandList::SetVertexBuffers | `void SetVertexBuffers(uint32_t firstBinding, uint32_t count, IBuffer* const* buffers, size_t const* offsets);` Vulkan vkCmdBindVertexBuffers；Metal setVertexBuffer |
| 008-RHI | te::rhi | ICommandList | 抽象接口 | 绑定索引缓冲 | te/rhi/CommandList.h | ICommandList::SetIndexBuffer | `void SetIndexBuffer(IBuffer* buffer, IndexType type, size_t offset);` type 为 Uint16/Uint32；Vulkan vkCmdBindIndexBuffer；Metal setVertexBuffer（index buffer） |
| 008-RHI | te::rhi | ICommandList | 抽象接口 | 绑定描述符集 | te/rhi/CommandList.h | ICommandList::SetDescriptorSet | `void SetDescriptorSet(uint32_t setIndex, IDescriptorSet* descriptorSet, uint32_t const* dynamicOffsets, uint32_t dynamicOffsetCount);` Vulkan vkCmdBindDescriptorSets；Metal setFragmentTexture/setVertexBuffer 等或 argument buffer |
| 008-RHI | te::rhi | ICommandList | 抽象接口 | 设置视口 | te/rhi/CommandList.h | ICommandList::SetViewport | `void SetViewport(Viewport const& viewport);` 或 SetViewports(count, viewports)；Vulkan vkCmdSetViewport；Metal setViewport |
| 008-RHI | te::rhi | ICommandList | 抽象接口 | 设置裁剪矩形 | te/rhi/CommandList.h | ICommandList::SetScissor | `void SetScissor(Rect2D const& rect);` 或 SetScissors(count, rects)；Vulkan vkCmdSetScissor；Metal setScissorRect |
| 008-RHI | te::rhi | ICommandList | 抽象接口 | 开始渲染通道 | te/rhi/CommandList.h | ICommandList::BeginRenderPass | `void BeginRenderPass(RenderPassDesc const& desc);` 绑定 RTV/DSV、load/store、clear；Vulkan vkCmdBeginRenderPass；Metal 由 encoder 创建时指定 |
| 008-RHI | te::rhi | ICommandList | 抽象接口 | 结束渲染通道 | te/rhi/CommandList.h | ICommandList::EndRenderPass | `void EndRenderPass();` Vulkan vkCmdEndRenderPass；Metal endEncoding（或下一 pass） |
| 008-RHI | te::rhi | ICommandList | 抽象接口 | 非索引绘制 | te/rhi/CommandList.h | ICommandList::Draw | `void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);` Vulkan vkCmdDraw；Metal drawPrimitives |
| 008-RHI | te::rhi | ICommandList | 抽象接口 | 索引绘制 | te/rhi/CommandList.h | ICommandList::DrawIndexed | `void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);` Vulkan vkCmdDrawIndexed；Metal drawIndexedPrimitives |
| 008-RHI | te::rhi | ICommandList | 抽象接口 | 间接绘制（可选） | te/rhi/CommandList.h | ICommandList::DrawIndexedIndirect | `void DrawIndexedIndirect(IBuffer* buffer, size_t offset);` Vulkan vkCmdDrawIndexedIndirect；Metal drawIndexedPrimitives(indirectBuffer:…) |
| 008-RHI | te::rhi | ICommandList | 抽象接口 | 派发计算 | te/rhi/CommandList.h | ICommandList::Dispatch | `void Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);` Vulkan vkCmdDispatch；Metal dispatchThreadgroups |
| 008-RHI | te::rhi | ICommandList | 抽象接口 | 缓冲拷贝 | te/rhi/CommandList.h | ICommandList::CopyBuffer | `void CopyBuffer(IBuffer* src, size_t srcOffset, IBuffer* dst, size_t dstOffset, size_t size);` Vulkan vkCmdCopyBuffer；Metal blitEncoder copyFromBuffer:toBuffer: |
| 008-RHI | te::rhi | ICommandList | 抽象接口 | 缓冲到纹理拷贝 | te/rhi/CommandList.h | ICommandList::CopyBufferToTexture | `void CopyBufferToTexture(IBuffer* src, ITexture* dst, BufferTextureCopy const* region);` Vulkan vkCmdCopyBufferToImage |
| 008-RHI | te::rhi | ICommandList | 抽象接口 | 纹理到缓冲拷贝 | te/rhi/CommandList.h | ICommandList::CopyTextureToBuffer | `void CopyTextureToBuffer(ITexture* src, IBuffer* dst, BufferTextureCopy const* region);` Vulkan vkCmdCopyImageToBuffer |
| 008-RHI | te::rhi | ICommandList | 抽象接口 | 资源屏障 | te/rhi/CommandList.h | ICommandList::ResourceBarrier | `void ResourceBarrier(uint32_t bufferBarrierCount, BufferBarrier const* bufferBarriers, uint32_t textureBarrierCount, TextureBarrier const* textureBarriers);` Vulkan vkCmdPipelineBarrier；D3D12 ResourceBarrier；Metal 隐式或 synchronizeResource |
| 008-RHI | te::rhi | ICommandList | 抽象接口 | 清空颜色附件 | te/rhi/CommandList.h | ICommandList::ClearRenderTarget | `void ClearRenderTarget(ITexture* rt, float const* color, Rect2D const* rect = nullptr);` 或按 attachment 索引；Vulkan vkCmdClearColorImage；Metal clear 在 loadAction |
| 008-RHI | te::rhi | ICommandList | 抽象接口 | 清空深度模板 | te/rhi/CommandList.h | ICommandList::ClearDepthStencil | `void ClearDepthStencil(ITexture* ds, bool clearDepth, bool clearStencil, float depthValue, uint32_t stencilValue, Rect2D const* rect = nullptr);` Vulkan vkCmdClearDepthStencilImage |

### 资源接口（IBuffer / ITexture / ISampler / IPSO）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 008-RHI | te::rhi | IBuffer | 抽象接口 | GPU 缓冲 | te/rhi/Buffer.h | IBuffer | 顶点/索引/Uniform/Staging 等；由 IDevice::CreateDeviceResource 创建；不直接构造 |
| 008-RHI | te::rhi | IBuffer | 抽象接口 | 获取缓冲大小 | te/rhi/Buffer.h | IBuffer::GetSize | `size_t GetSize() const;` |
| 008-RHI | te::rhi | IBuffer | 抽象接口 | 获取 GPU 地址（可选） | te/rhi/Buffer.h | IBuffer::GetGPUAddress | `uint64_t GetGPUAddress() const;` D3D12/Vulkan 用于绑定；Metal 无直接地址时返回 0 或由实现约定 |
| 008-RHI | te::rhi | ITexture | 抽象接口 | GPU 纹理 | te/rhi/Texture.h | ITexture | 纹理及视图（SRV/RTV/DSV 等）；由 IDevice::CreateDeviceResource 创建；不直接构造 |
| 008-RHI | te::rhi | ITexture | 抽象接口 | 获取纹理宽度 | te/rhi/Texture.h | ITexture::GetWidth | `uint32_t GetWidth() const;` |
| 008-RHI | te::rhi | ITexture | 抽象接口 | 获取纹理高度 | te/rhi/Texture.h | ITexture::GetHeight | `uint32_t GetHeight() const;` |
| 008-RHI | te::rhi | ITexture | 抽象接口 | 获取纹理深度或数组层数 | te/rhi/Texture.h | ITexture::GetDepthOrArrayLayers | `uint32_t GetDepthOrArrayLayers() const;` |
| 008-RHI | te::rhi | ITexture | 抽象接口 | 获取 Mip 层数 | te/rhi/Texture.h | ITexture::GetMipLevelCount | `uint32_t GetMipLevelCount() const;` |
| 008-RHI | te::rhi | ITexture | 抽象接口 | 获取像素格式 | te/rhi/Texture.h | ITexture::GetFormat | `PixelFormat GetFormat() const;` |
| 008-RHI | te::rhi | ISampler | 抽象接口 | 采样器 | te/rhi/Sampler.h | ISampler | 由 IDevice::CreateDeviceResource 创建；不直接构造 |
| 008-RHI | te::rhi | IPSO | 抽象接口 | 管线状态对象 | te/rhi/PSO.h | IPSO | 图形/计算 PSO；与 Shader 字节码绑定；由 IDevice::CreateDeviceResource 创建；可缓存 |
| 008-RHI | te::rhi | IPSO | 抽象接口 | 获取管线布局（可选） | te/rhi/PSO.h | IPSO::GetPipelineLayout | `IPipelineLayout* GetPipelineLayout() const;` 用于动态 offset 等；可选 |

### 描述符集（IDescriptorSetLayout / IDescriptorSet）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 008-RHI | te::rhi | IDescriptorSetLayout | 抽象接口 | 描述符集布局句柄 | te/rhi/DescriptorSet.h | IDescriptorSetLayout | 由 CreateDeviceResource(type=DescriptorSetLayout, …) 返回；不直接构造；供创建 DescriptorSet 时引用 |
| 008-RHI | te::rhi | IDescriptorSet | 抽象接口 | 描述符集 | te/rhi/DescriptorSet.h | IDescriptorSet | 由 CreateDeviceResource(type=DescriptorSet, …) 返回；不直接构造 |
| 008-RHI | te::rhi | IDescriptorSet | 抽象接口 | 写入描述符 | te/rhi/DescriptorSet.h | IDescriptorSet::Update | `void Update(uint32_t writeCount, DescriptorWrite const* writes);` Vulkan vkUpdateDescriptorSets；在录制/提交前调用 |
| 008-RHI | te::rhi | IDevice | 抽象接口 | 更新描述符（设备层便捷） | te/rhi/Device.h | IDevice::UpdateDeviceResource | 重载：resource 为 IDescriptorSet* 时，data 指向 DescriptorWrite 数组；须在线程 D 调用 |

### 同步（IFence / ISemaphore）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 008-RHI | te::rhi | IFence | 抽象接口 | Fence 等待 | te/rhi/Fence.h | IFence::Wait | `void Wait(uint64_t timeoutNs = UINT64_MAX);` 阻塞直到 signal；Vulkan vkWaitForFences |
| 008-RHI | te::rhi | IFence | 抽象接口 | Fence 重置 | te/rhi/Fence.h | IFence::Reset | `void Reset();` 置为 unsignaled；Vulkan vkResetFences |
| 008-RHI | te::rhi | IFence | 抽象接口 | 查询 Fence 状态 | te/rhi/Fence.h | IFence::GetStatus | `bool GetStatus() const;` 已 signal 返回 true |
| 008-RHI | te::rhi | IFence | 抽象接口 | 获取当前值（可选） | te/rhi/Fence.h | IFence::GetCompletedValue | `uint64_t GetCompletedValue() const;` D3D12 风格 timeline；Vulkan 无则返回 0/1 |
| 008-RHI | te::rhi | IDevice | 抽象接口 | 销毁 Fence | te/rhi/Device.h | IDevice::DestroyFence | `void DestroyFence(IFence* fence);` 或统一 DestroyResource（若 Fence 归入 Resource 类型） |
| 008-RHI | te::rhi | IDevice | 抽象接口 | 销毁 Semaphore | te/rhi/Device.h | IDevice::DestroySemaphore | `void DestroySemaphore(ISemaphore* semaphore);` |

### 交换链（ISwapChain）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 008-RHI | te::rhi | ISwapChain | 抽象接口 | 呈现 | te/rhi/SwapChain.h | ISwapChain::Present | `bool Present();` 内部与当前待 Present 帧的 Fence 同步后再 Present；每帧调用一次；失败可重试 |
| 008-RHI | te::rhi | ISwapChain | 抽象接口 | 获取当前后备缓冲纹理 | te/rhi/SwapChain.h | ISwapChain::GetCurrentBackBuffer | `ITexture* GetCurrentBackBuffer();` 用于本帧渲染目标；Vulkan vkAcquireNextImageKHR 后对应 image view；Metal currentDrawable.texture |
| 008-RHI | te::rhi | ISwapChain | 抽象接口 | 获取当前后备缓冲索引 | te/rhi/SwapChain.h | ISwapChain::GetCurrentBackBufferIndex | `uint32_t GetCurrentBackBufferIndex() const;` 0～bufferCount-1 |
| 008-RHI | te::rhi | ISwapChain | 抽象接口 | 调整交换链大小 | te/rhi/SwapChain.h | ISwapChain::Resize | `void Resize(uint32_t width, uint32_t height);` 窗口 resize 时调用；可能需等待 GPU 空闲后重建 swapchain |
| 008-RHI | te::rhi | ISwapChain | 抽象接口 | 获取宽度 | te/rhi/SwapChain.h | ISwapChain::GetWidth | `uint32_t GetWidth() const;` |
| 008-RHI | te::rhi | ISwapChain | 抽象接口 | 获取高度 | te/rhi/SwapChain.h | ISwapChain::GetHeight | `uint32_t GetHeight() const;` |
| 008-RHI | te::rhi | — | struct | 交换链创建描述 | te/rhi/SwapChain.h | SwapChainDesc | windowHandle、width、height、format、bufferCount、vsync 等 |

---

*来源：用户故事 US-002（一帧渲染）、US-004（流水线式多帧渲染）、US-rendering-004（多线程管线阶段；线程 D = 唯一 GPU/Device 线程）。参考 Vulkan 1.3、Metal 3、D3D12、Unreal FRHICommandList、Unity 底层图形 API。*
