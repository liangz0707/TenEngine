# 008-RHI 后端任务索引（T0xx）与 ABI 状态

- **契约**：与 [008-rhi-ABI.md](./008-rhi-ABI.md) 配套，用于代码内 `// T0xx:` 注释与实现状态的对应。
- **用途**：补全 T0xx 注明代码时，可据此表核对实现状态并更新 ABI。

## Vulkan（T038–T042）

| ID | 位置 | 描述 | 实现状态 | 说明 |
|----|------|------|----------|------|
| T038 | CommandList Draw/Dispatch/Copy/ResourceBarrier | vkCmdDraw / vkCmdDrawIndexed / vkCmdDispatch；Copy 为 CPU 内存拷贝；ResourceBarrier 由 BufferBarrier/TextureBarrier 转 vkCmdPipelineBarrier | 已实现 | Draw/Dispatch 已调用 vkCmd*；ResourceBarrier 支持 0 屏障与全屏障/按资源过渡 |
| T039 | CreateBuffer/CreateTexture/CreateView | 内存类型选择（host-visible/device-local）；TextureDesc.format 映射 VkFormat；CreateView 创建 VkImageView/描述符 | 已实现 / 占位 | 内存类型按 physicalDevice 选择；format 映射已补；CreateView 仍为资源指针占位 |
| （DescriptorSet） | CreateDescriptorSetLayout / AllocateDescriptorSet / UpdateDescriptorSet | VkDescriptorSetLayout、VkDescriptorPool、VkDescriptorSet、vkUpdateDescriptorSets；texture SRV 创建 VkImageView 并缓存在 DescriptorSetVulkan | 已实现 | Vulkan 已完整实现（含 texture SRV 的 VkImageView） |
| T040 | PSO | VkShaderModule、VkPipelineLayout、VkGraphicsPipeline/VkComputePipeline 完整创建；SetShader/Cache | 占位 | 最小路径：仅校验 SPIR-V 后返回非空 IPSO；完整管线创建待补 |
| T041 | Sync | vkWaitForFences；Submit 时传入 signalFence 到 VkSubmitInfo | 已实现 | Fence Wait 调用 vkWaitForFences；Queue::Submit 支持 signalFence |
| T042 | SwapChain | vkQueuePresentKHR；vkGetSwapchainImagesKHR；vkCreateSwapchainKHR（需 VkSurfaceKHR/windowHandle） | 占位 | 无窗口时返回 stub；有 windowHandle 时待接 VkSurfaceKHR 与 vkCreateSwapchainKHR |

## D3D12（T046–T051）

| ID | 位置 | 描述 | 实现状态 | 说明 |
|----|------|------|----------|------|
| T046 | Queue Submit / WaitIdle | 提交时 signalFence 用 queue->Signal(fence, value)；WaitIdle 通过 fence + WaitForSingleObject | 已实现 | Submit 写入 signalFence；WaitIdle 使用内部 fence 等待 |
| T047 | CommandList Draw/Dispatch/Copy/ResourceBarrier | DrawInstanced；Dispatch；CopyBufferRegion；CopyBufferToTexture/CopyTextureToBuffer（GetCopyableFootprints + CopyTextureRegion）；BufferBarrier/TextureBarrier 转 D3D12_RESOURCE_BARRIER | 已实现 | Draw/Dispatch/ResourceBarrier/CopyBuffer* 均已实现 |
| （BeginRenderPass） | BeginRenderPass / EndRenderPass | RTV/DSV heap、CreateRenderTargetView/CreateDepthStencilView、OMSetRenderTargets、Clear | 已实现 | DeviceD3D12 持 RTV/DSV heap，EnsureRtvDsvHeaps；BeginRenderPassD3D12Impl 绑定并 Clear |
| T048 | Resources / CreateView | TextureDesc.format 映射 DXGI_FORMAT；CreateDescriptorHeap、SRV/UAV/RTV/DSV | 已实现 / 占位 | format 映射已补；CreateView 仍为资源指针占位；DescriptorSet 占位（见 ABI 不能实现原因） |
| T049 | PSO | Root signature、D3D12_GRAPHICS_PIPELINE_STATE_DESC 完整创建 | 占位 | 最小路径返回非空 IPSO；完整 PSO 待补 |
| T050 | Sync | Fence Wait/Signal：CreateEventEx、SetEventOnCompletion、WaitForSingleObject；queue->Signal | 已实现 | FenceD3D12 带 event；Wait() 使用 SetEventOnCompletion + WaitForSingleObject；Signal 由 Submit 触发 |
| T051 | SwapChain | GetBuffer、ResizeBuffers、CreateSwapChainForHwnd（需 windowHandle） | 占位 | 无窗口时 stub；需应用层提供窗口，见 ABI 不能实现原因 |

## Metal（T055–T060）

| ID | 位置 | 描述 | 实现状态 | 说明 |
|----|------|------|----------|------|
| T055 | Queue WaitIdle | 对上次提交的 MTLCommandBuffer 调用 waitUntilCompleted | 已实现 | QueueMetal 记录 lastCommitted；WaitIdle 等待该 buffer |
| T056 | CommandList Draw/Dispatch/Copy/ResourceBarrier | MTLRenderCommandEncoder drawPrimitives/drawIndexedPrimitives；MTLComputeCommandEncoder dispatchThreadgroups；MTLBlitCommandEncoder copyFromBuffer/copyFromBuffer:toTexture:/copyFromTexture:toBuffer:；memoryBarrierWithScope | 已实现 / 占位 | SetViewport/SetScissor 已实现；CopyBuffer/CopyBufferToTexture/CopyTextureToBuffer 已实现（blit encoder）；Draw/Dispatch 需 PSO 绑定；ResourceBarrier 占位 |
| T057 | Resources / CreateView | TextureDesc.format 映射 MTLPixelFormat；MTLTexture 直接作 view | 已实现 / 占位 | format 映射已补；CreateView 返回资源指针 |
| T058 | PSO | MTLLibrary、MTLRenderPipelineDescriptor、newRenderPipelineStateWithDescriptor | 占位 | 最小路径返回非空 IPSO |
| T059 | Sync | MTLSharedEvent waitUntilSignaledValue / signalEvent | 占位 | Fence 创建 newSharedEvent；Wait/Signal 待接 encoder 或 queue 时序 |
| T060 | SwapChain | CAMetalLayer、nextDrawable、present、drawableSize | 占位 | 无窗口时 stub |

## ABI 关联

- 上述“已实现”项与 [008-rhi-ABI.md](./008-rhi-ABI.md) 中「实现状态表（按后端）」一致。
- 新增或变更后端行为时，应同步更新本表与 008-rhi-ABI.md 的实现状态表。

---

*本文件为 008-rhi-fullmodule 后端任务索引；T0xx 注释与实现对应关系以本表为准。*
