# Data Model: 008-RHI 完整模块实现

**Branch**: `008-rhi-fullmodule-005` | **Phase**: 1

## 实体与关系

### 设备与后端

| 实体 | 字段/属性 | 关系 | 说明 |
|------|-----------|------|------|
| Backend | Vulkan, D3D12, Metal, D3D11 | 枚举 | 编译期宏 TE_RHI_* 与运行时 SelectBackend 一致 |
| IDevice | （抽象接口）GetBackend, GetQueue, GetFeatures, GetLimits, Create* / Destroy* | 1 对多 IQueue、ICommandList、IBuffer、ITexture、IPSO、IFence、ISemaphore、ISwapChain、IDescriptorSetLayout、IDescriptorSet | 生命周期至 DestroyDevice |
| DeviceFeatures | maxTextureDimension2D, maxTextureDimension3D | 只读 | 由各后端查询真实能力填充 |
| DeviceLimits | maxBufferSize, maxTextureDimension2D/3D, minUniformBufferOffsetAlignment | 只读 | 由各后端查询真实限制填充 |

### 队列与命令

| 实体 | 字段/属性 | 关系 | 说明 |
|------|-----------|------|------|
| QueueType | Graphics, Compute, Copy | 枚举 | 与后端队列类型映射 |
| IQueue | Submit(cmdList, signalFence, waitSem, signalSem), WaitIdle | 属于 IDevice | 不拥有命令列表；提交后命令列表可复用或销毁 |
| ICommandList | Begin, End, Draw, DrawIndexed, Dispatch, Copy, ResourceBarrier, SetViewport, SetScissor, BeginRenderPass, EndRenderPass, CopyBuffer, CopyBufferToTexture, CopyTextureToBuffer, BuildAccelerationStructure, DispatchRays | 由 IDevice 创建 | 单次录制周期内有效；End 后方可 Submit |

### 资源与视图

| 实体 | 字段/属性 | 关系 | 说明 |
|------|-----------|------|------|
| BufferDesc | size, usage | 输入 | CreateBuffer 入参 |
| TextureDesc | width, height, depth, format | 输入 | CreateTexture 入参 |
| SamplerDesc | filter | 输入 | CreateSampler 入参 |
| ViewDesc | resource, type | 输入 | CreateView 入参 |
| IBuffer, ITexture, ISampler | （抽象接口） | 由 IDevice 创建 | 生命周期至显式 Destroy* |
| ViewHandle | uintptr_t | 与 resource 绑定 | 用于 PSO/描述符绑定 |

### PSO

| 实体 | 字段/属性 | 关系 | 说明 |
|------|-----------|------|------|
| GraphicsPSODesc | vertex_shader, vertex_shader_size, fragment_shader, fragment_shader_size | 输入 | 各后端用 SPIR-V/DXIL/MSL 字节码创建真实 PSO |
| ComputePSODesc | compute_shader, compute_shader_size | 输入 | 同上 |
| IPSO | （抽象接口） | 由 IDevice 创建 | SetShader/Cache 后可用于绑定与绘制 |

### 同步

| 实体 | 字段/属性 | 关系 | 说明 |
|------|-----------|------|------|
| IFence | Wait, Signal, Reset | 由 IDevice 创建 | 真实后端对象（VkFence、ID3D12Fence、MTLFence 等） |
| ISemaphore | （抽象接口） | 由 IDevice 创建 | 用于队列间同步 |
| BufferBarrier / TextureBarrier | buffer/texture, offset/size, srcState, dstState | ICommandList::ResourceBarrier 入参 | Vulkan/D3D12/Metal 真实屏障；D3D11 文档化空实现 |

### 交换链与描述符集

| 实体 | 字段/属性 | 关系 | 说明 |
|------|-----------|------|------|
| SwapChainDesc | windowHandle, width, height, bufferCount, vsync | 输入 | 无窗口时可离屏 |
| ISwapChain | Present, GetCurrentBackBuffer, GetCurrentBackBufferIndex, Resize, GetWidth, GetHeight | 由 IDevice 创建 | 各后端真实 SwapChain/表面 |
| DescriptorSetLayoutDesc, DescriptorWrite | bindings, dstSet, binding, type, buffer/texture/sampler | 输入 | 描述符布局与写入 |
| IDescriptorSetLayout, IDescriptorSet | （抽象接口） | 由 IDevice 创建/分配 | 各后端真实描述符池/堆/Argument Buffer |

### 光追（可选）

| 实体 | 字段/属性 | 关系 | 说明 |
|------|-----------|------|------|
| RaytracingAccelerationStructureDesc, DispatchRaysDesc | 见 te/rhi/raytracing.hpp | ICommandList 入参 | D3D12 在 DXR 可用时真实实现；其余后端显式不支持 |

## 状态与校验

- **IDevice**：CreateDevice(Backend) 在对应后端不可用时返回 nullptr；不自动回退到其他后端。
- **ICommandList**：Begin 后未 End 即再次 Begin、或未 Begin 即 End/Submit 为未定义行为；实现可断言或文档化。
- **资源**：CreateBuffer(size==0)、CreateTexture(width==0 或 height==0) 返回 nullptr；Destroy* 后句柄不可再使用。
- **PSO**：无效或空 Shader 字节码可返回 nullptr；SetShader/Cache 失败由实现定义（可记录错误或断言）。

## 生命周期顺序

1. CreateDevice → GetQueue / CreateCommandList / CreateBuffer 等  
2. 录制命令：Begin → Draw/Dispatch/ResourceBarrier/... → End  
3. Submit(cmd, queue[, fence, ...])；可选 Wait(fence)  
4. DestroyCommandList / DestroyBuffer / ... → DestroyDevice（顺序不得违反后端要求，如先释放依赖该资源的命令或 PSO）
