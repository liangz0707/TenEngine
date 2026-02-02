# 008-RHI 模块 ABI（全量 - 实现参考）

本文件为 **plan 生成的全量 ABI 内容**，供 tasks 与 implement 阶段实现参考。原始 ABI 见 `specs/_contracts/008-rhi-ABI.md`；**本文件 = 原始 ABI + 本次新增/修改（原 TODO 实现后的正式条目）**。

- **契约**：[008-rhi-public-api.md](../../_contracts/008-rhi-public-api.md)
- **本 feature 增量**：Buffer CPU 写入（UpdateBuffer）、Uniform 绑定（SetUniformBuffer）、BufferUsage 含 Uniform、BufferDesc.usage 语义。

## 全量 ABI 表（按头文件分组）

### 类型与枚举（te/rhi/types.hpp）

| 模块名 | 命名空间 | 符号 | 导出形式 | 接口说明 | 头文件 | 说明 |
|--------|----------|------|----------|----------|--------|------|
| 008-RHI | te::rhi | Backend | 枚举 | 图形后端 | te/rhi/types.hpp | `enum class Backend : unsigned { Vulkan = 0, D3D12 = 1, Metal = 2, D3D11 = 3 };` |
| 008-RHI | te::rhi | DeviceLimits | struct | 设备限制 | te/rhi/types.hpp | maxBufferSize, maxTextureDimension2D/3D, minUniformBufferOffsetAlignment |
| 008-RHI | te::rhi | QueueType | 枚举 | 队列类型 | te/rhi/types.hpp | Graphics, Compute, Copy |
| 008-RHI | te::rhi | DeviceFeatures | struct | 设备特性 | te/rhi/types.hpp | maxTextureDimension2D, maxTextureDimension3D |
| 008-RHI | te::rhi | ResourceState | 枚举 | 资源状态 | te/rhi/types.hpp | Common, VertexBuffer, IndexBuffer, RenderTarget, DepthWrite, ShaderResource, CopySrc, CopyDst, Present |
| 008-RHI | te::rhi | BufferBarrier, TextureBarrier | struct | 屏障 | te/rhi/types.hpp | 见现有 ABI |
| 008-RHI | te::rhi | IDevice, IQueue, ICommandList, IBuffer, ITexture, ISampler, IPSO, IFence, ISemaphore, ISwapChain | 前向声明 | 句柄 | te/rhi/types.hpp | 定义在对应头文件 |

### 资源描述（te/rhi/resources.hpp）

| 模块名 | 命名空间 | 符号 | 导出形式 | 接口说明 | 头文件 | 说明 |
|--------|----------|------|----------|----------|--------|------|
| 008-RHI | te::rhi | **BufferUsage** | **枚举** | **缓冲用途位（本次新增）** | te/rhi/resources.hpp | `enum class BufferUsage : uint32_t { Vertex = 1u << 0, Index = 1u << 1, Uniform = 1u << 2, Storage = 1u << 3, CopySrc = 1u << 4, CopyDst = 1u << 5 };` 或等价位掩码常量 |
| 008-RHI | te::rhi | BufferDesc | struct | 缓冲创建描述 | te/rhi/resources.hpp | `size_t size; uint32_t usage;` **usage 为 BufferUsage 位掩码，须含 Uniform 位以创建 Uniform 缓冲（本次修改语义）** |
| 008-RHI | te::rhi | TextureDesc | struct | 纹理创建描述 | te/rhi/resources.hpp | width, height, depth, format |
| 008-RHI | te::rhi | SamplerDesc, ViewDesc, ViewHandle | struct/别名 | 采样器/视图 | te/rhi/resources.hpp | 见现有 ABI |
| 008-RHI | te::rhi | IBuffer, ITexture, ISampler | 抽象接口 | GPU 资源 | te/rhi/resources.hpp | 虚析构；无其他成员 |

### 设备（te/rhi/device.hpp）

在现有 IDevice 成员基础上，**新增**：

| 模块名 | 命名空间 | 符号 | 导出形式 | 接口说明 | 头文件 | 说明 |
|--------|----------|------|----------|----------|--------|------|
| 008-RHI | te::rhi | **IDevice::UpdateBuffer** | **成员函数** | **CPU 写入 GPU 缓冲（本次新增）** | te/rhi/device.hpp | `void UpdateBuffer(IBuffer* buf, size_t offset, void const* data, size_t size) = 0;` 满足 009 UniformBuffer::Update |

其余 IDevice 成员（SelectBackend, GetSelectedBackend, CreateDevice, DestroyDevice, GetBackend, GetQueue, GetFeatures, GetLimits, CreateCommandList, DestroyCommandList, CreateBuffer, CreateTexture, CreateSampler, CreateView, DestroyBuffer/DestroyTexture/DestroySampler, CreateGraphicsPSO, CreateComputePSO, SetShader, Cache, DestroyPSO, CreateFence, CreateSemaphore, DestroyFence, DestroySemaphore, CreateSwapChain, CreateDescriptorSetLayout, AllocateDescriptorSet, UpdateDescriptorSet, DestroyDescriptorSetLayout, DestroyDescriptorSet）与现有 ABI 一致。

### 命令列表（te/rhi/command_list.hpp）

在现有 ICommandList 成员基础上，**新增**：

| 模块名 | 命名空间 | 符号 | 导出形式 | 接口说明 | 头文件 | 说明 |
|--------|----------|------|----------|----------|--------|------|
| 008-RHI | te::rhi | **ICommandList::SetUniformBuffer** | **成员函数** | **将 IBuffer 绑定到 slot（本次新增）** | te/rhi/command_list.hpp | `void SetUniformBuffer(uint32_t slot, IBuffer* buffer, size_t offset) = 0;` 满足 009 IUniformBuffer::Bind；slot 越界或 buffer==nullptr 行为由实现约定（no-op 或错误码） |

其余 ICommandList 成员（Begin, End, Draw, DrawIndexed, SetViewport, SetScissor, BeginRenderPass, EndRenderPass, CopyBuffer, CopyBufferToTexture, CopyTextureToBuffer, BuildAccelerationStructure, DispatchRays, Dispatch, Copy, ResourceBarrier）与现有 ABI 一致。

### 其余头文件（queue, sync, swapchain, pso, descriptor_set, raytracing）

与 `specs/_contracts/008-rhi-ABI.md` 中对应表一致，无新增或修改。

---

## 实现约束

- **全量实现**：tasks 与 implement 须实现**上述全量**（原始 ABI 全部符号 + 本次新增 UpdateBuffer、SetUniformBuffer、BufferUsage 及 BufferDesc.usage 语义），不得仅实现新增部分。
- **写回契约**：仅将「契约更新」小节中的**新增/修改条目**写回 `specs/_contracts/008-rhi-ABI.md`；原 TODO 节在写回后移除或改为“已实现”说明。
