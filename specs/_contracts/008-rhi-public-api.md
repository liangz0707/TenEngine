# Contract: 008-RHI Module Public API

## Applicable Modules

- **Implementer**: 008-RHI (L1; Graphics API abstraction: device, command list, resources, PSO, sync; multi-backend Vulkan/D3D12/Metal/D3D11; namespace `te::rhi`, backend macros TE_RHI_*)
- **Corresponding Spec**: `docs/module-specs/008-rhi.md`
- **Dependencies**: 001-Core

## Consumers

- 009-RenderCore, 010-Shader, 012-Mesh, 019-PipelineCore, 020-Pipeline, 024-Editor, 028-Texture

## Third-Party Dependencies

- volk, vulkan-headers (Vulkan); d3d11, d3d12 (Windows); metal (Apple). Selected by platform and compile options; see `docs/third_party/` for corresponding documentation.

## Capability List

### Types and Handles (Cross-Boundary)

| Name | Semantics | Lifetime |
|------|-----------|----------|
| IDevice | Graphics device abstraction; creates queues, resources, PSO; CreateDevice, DestroyDevice | Created until DestroyDevice |
| IQueue | Queue handle; IDevice::GetQueue(QueueType, index); QueueType: Graphics, Compute, Copy | Non-owning, consistent with IDevice |
| ICommandList | Command buffer; Begin/End, Draw, DrawIndexed, Dispatch, Copy, ResourceBarrier, SetViewport, SetScissor, BeginRenderPass/EndRenderPass, NextSubpass, Submit | Valid within single recording cycle |
| IBuffer / ITexture | Buffer, texture; CreateBuffer, CreateTexture, CreateView, Destroy | Created until explicit Destroy |
| ISampler | Sampler; CreateSampler, DestroySampler | Created until explicit destroy |
| IPSO | Pipeline state object (graphics/compute); CreateGraphicsPSO, CreateComputePSO, SetShader, Cache, DestroyPSO | Created until explicit destroy, cacheable |
| IFence / ISemaphore | Sync objects; CreateFence, CreateSemaphore, Wait, Signal, Reset, Destroy | Per implementation convention |
| IDescriptorSetLayout / IDescriptorSet | Descriptor set layout and descriptor set; CreateDescriptorSetLayout, AllocateDescriptorSet, UpdateDescriptorSet, BindDescriptorSet | Created until explicit destroy |
| IRenderPass | Render pass object; CreateRenderPass, DestroyRenderPass; supports multi-subpass | Created until explicit destroy |
| ViewHandle | Resource view handle, used to bind to PSO | Consistent with resource lifetime |
| Backend / DeviceFeatures / DeviceLimits / VSyncMode / ColorSpace / PresentMode | Backend enum, features and limits query; swapchain extended settings | Read-only |

Does not expose specific backend types; 012/028 calls this module's CreateBuffer/CreateTexture during EnsureDeviceResources. Submission convention with 020 see `pipeline-to-rci.md`.

### Capabilities (Provider Guarantees)

| # | Capability | Description |
|---|------------|-------------|
| 1 | Device & Queue | CreateDevice(Backend), DestroyDevice, GetQueue; SelectBackend, GetSelectedBackend; GetFeatures, GetLimits; multi-backend unified interface; CreateRenderPass, DestroyRenderPass |
| 2 | Command List | CreateCommandList, DestroyCommandList; Begin, End; Draw, DrawIndexed, Dispatch, Copy, ResourceBarrier; SetViewport, SetScissor; SetUniformBuffer, SetVertexBuffer, SetIndexBuffer, SetGraphicsPSO, BindDescriptorSet (single and multi-set); BeginRenderPass, NextSubpass, EndRenderPass; BeginOcclusionQuery, EndOcclusionQuery; Submit(cmd, queue) and Fence/Semaphore overload; CopyBuffer, CopyBufferToTexture, CopyTextureToBuffer, Copy; BuildAccelerationStructure, DispatchRays (D3D12 ray tracing) |
| 3 | Resource Management | CreateBuffer, CreateTexture, CreateSampler, CreateView; Destroy; memory and lifetime explicit; failure clearly reported |
| 4 | PSO | CreateGraphicsPSO(desc), CreateGraphicsPSO(desc, layout) (coupled with descriptor layout), CreateGraphicsPSO(desc, layout, pass, subpass, layoutSet1) (render pass and multi-set support), CreateComputePSO, SetShader, Cache, DestroyPSO; interfaces with RenderCore/Shader |
| 5 | Sync | CreateFence, CreateSemaphore, Wait, Signal, Reset, Destroy; resource barrier in ICommandList::ResourceBarrier |
| 6 | SwapChain | CreateSwapChain(SwapChainDesc); Present, GetCurrentBackBuffer, GetCurrentBackBufferIndex, Resize, GetWidth, GetHeight; extended: SetVSyncMode, GetVSyncMode, SetHDRMode, IsHDREnabled, GetColorSpace, SetHDRMetadata, SupportsHDR, SupportsTearing, GetRefreshRate; VSyncMode, ColorSpace, PresentMode enums; HDRMetadata struct |
| 7 | Descriptor Set | CreateDescriptorSetLayout, AllocateDescriptorSet, UpdateDescriptorSet, DestroyDescriptorSetLayout, DestroyDescriptorSet; DescriptorType enum; DescriptorWrite struct with bufferOffset for UB ring buffer |
| 8 | Error & Recovery | Device loss or runtime error can be reported; supports fallback or rebuild |
| 9 | Thread Safety | Multi-threaded behavior is implementation-defined and documented |

## Version / ABI

- Follows Constitution: Public API versioned; breaking changes increment MAJOR.

## Constraints

- Must complete RHI initialization before creating resources and submitting commands. Resource destruction order must conform to underlying API requirements. Command buffers produced by 020 are submitted via Submit to RHI; format and timing see `pipeline-to-rci.md`.

## Change Log

| Date | Change Description |
|------|-------------------|
| (Initial) | Extracted from 002-rendering-rci-interface spec |
| T0 Update | Aligned with T0 architecture; consumers by dependency graph |
| 2026-02-05 | Unified directory; capability list in table format; removed ABI back-reference |
| 2026-02-10 | Capability 2 command list: added SetVertexBuffer, SetIndexBuffer, SetGraphicsPSO, BeginOcclusionQuery, EndOcclusionQuery |
| 2026-02-10 | Capability 2/4: BindDescriptorSet; CreateGraphicsPSO(desc, layout); descriptor set API implemented |
| 2026-02-22 | Code-aligned update: added IRenderPass, multi-subpass support (NextSubpass), BindDescriptorSet with setIndex overload, CreateGraphicsPSO with renderPass/subpass/layoutSet1 overloads, extended swapchain (VSyncMode, ColorSpace, PresentMode, HDRMetadata, HDR support), ray tracing (BuildAccelerationStructure, DispatchRays) |
