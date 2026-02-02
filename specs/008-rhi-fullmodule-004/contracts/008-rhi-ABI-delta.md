# 008-RHI ABI 增量（本 feature 新增/修改）

**Feature**: 008-rhi-fullmodule-004 | **相对**：`specs/_contracts/008-rhi-ABI.md`  
**说明**：本文件仅保存**相对于现有 ABI 的新增与修改**；实现时使用**全量 ABI**（现有表 + 本表）。写回时将本表合并到 `specs/_contracts/008-rhi-ABI.md`。

## 类型与枚举（te/rhi/types.hpp）

| 操作 | 符号 | 导出形式 | 接口说明 | 说明 |
|------|------|----------|----------|------|
| 修改 | Backend | 枚举 | 图形后端 | 增加 `D3D11 = 3` |
| 新增 | DeviceLimits | struct | 设备限制 | 如 maxBufferSize, maxTextureDimension2D, maxTextureDimension3D, minUniformBufferOffsetAlignment (uint32_t/size_t) |

## 设备与队列（te/rhi/device.hpp, te/rhi/queue.hpp）

| 操作 | 符号 | 导出形式 | 接口说明 | 说明 |
|------|------|----------|----------|------|
| 修改 | IDevice::CreateFence | 成员函数 | 创建 Fence | `IFence* CreateFence(bool initialSignaled = false) = 0;` 失败返回 nullptr |
| 新增 | IDevice::GetLimits | 成员函数 | 设备限制 | `DeviceLimits const& GetLimits() const = 0;` |

## 命令列表（te/rhi/command_list.hpp）

| 操作 | 符号 | 导出形式 | 接口说明 | 说明 |
|------|------|----------|----------|------|
| 新增 | Submit (重载) | 自由函数 | 提交到队列（全参） | `void Submit(ICommandList* cmd, IQueue* queue, IFence* signalFence, ISemaphore* waitSem, ISemaphore* signalSem);` |
| 新增 | ICommandList::DrawIndexed | 成员函数 | 索引绘制 | `void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) = 0;` |
| 新增 | Viewport | struct | 视口 | x, y, width, height, minDepth, maxDepth (float) |
| 新增 | ScissorRect | struct | 裁剪矩形 | x, y, width, height (int32_t 或 uint32_t) |
| 新增 | ICommandList::SetViewport | 成员函数 | 设置视口 | `void SetViewport(uint32_t first, uint32_t count, Viewport const* viewports) = 0;` |
| 新增 | ICommandList::SetScissor | 成员函数 | 设置裁剪 | `void SetScissor(uint32_t first, uint32_t count, ScissorRect const* scissors) = 0;` |
| 新增 | RenderPassDesc | struct | 渲染通道描述 | color/depth 附件、loadOp/storeOp（P2） |
| 新增 | ICommandList::BeginRenderPass | 成员函数 | 开始渲染通道 | `void BeginRenderPass(RenderPassDesc const& desc) = 0;`（P2） |
| 新增 | ICommandList::EndRenderPass | 成员函数 | 结束渲染通道 | `void EndRenderPass() = 0;`（P2） |
| 新增 | ICommandList::CopyBuffer | 成员函数 | 缓冲间拷贝 | `void CopyBuffer(IBuffer* src, size_t srcOffset, IBuffer* dst, size_t dstOffset, size_t size) = 0;`（P2） |
| 新增 | ICommandList::CopyBufferToTexture | 成员函数 | 缓冲到纹理 | 见 data-model（P2） |
| 新增 | ICommandList::CopyTextureToBuffer | 成员函数 | 纹理到缓冲 | 见 data-model（P2） |

## 描述符集（te/rhi/descriptor_set.hpp 或 resources.hpp）（P2）

| 操作 | 符号 | 导出形式 | 接口说明 | 说明 |
|------|------|----------|----------|------|
| 新增 | DescriptorSetLayoutDesc | struct | 描述符集布局描述 | bindings[] |
| 新增 | DescriptorWrite | struct | 描述符写入 | dstSet, binding, type, resource |
| 新增 | IDescriptorSetLayout | 抽象接口 | 描述符集布局 | 虚析构 |
| 新增 | IDescriptorSet | 抽象接口 | 描述符集 | 虚析构 |
| 新增 | IDevice::CreateDescriptorSetLayout | 成员函数 | 创建布局 | 失败返回 nullptr |
| 新增 | IDevice::AllocateDescriptorSet | 成员函数 | 分配描述符集 | 失败返回 nullptr |
| 新增 | IDevice::UpdateDescriptorSet | 成员函数 | 更新描述符 | void UpdateDescriptorSet(IDescriptorSet*, DescriptorWrite const*, uint32_t); |

## 光追（te/rhi/raytracing.hpp 或扩展）（仅 D3D12）

| 操作 | 符号 | 导出形式 | 接口说明 | 说明 |
|------|------|----------|----------|------|
| 新增 | RaytracingAccelerationStructureDesc | struct | 加速结构描述 | 映射 D3D12 BLAS/TLAS 输入 |
| 新增 | DispatchRaysDesc | struct | 派发光线描述 | 映射 D3D12_DISPATCH_RAYS_DESC |
| 新增 | ICommandList::BuildAccelerationStructure | 成员函数 | 构建加速结构 | 仅 D3D12 实现 |
| 新增 | ICommandList::DispatchRays | 成员函数 | 派发光线 | 仅 D3D12 实现 |

（P2 项可在本 feature 或后续迭代实现；全量实现时 tasks 按 P1/P2 拆分。）
