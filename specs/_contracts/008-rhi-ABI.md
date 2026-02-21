# 008-RHI Module ABI

- **Contract**: [008-rhi-public-api.md](./008-rhi-public-api.md) (capabilities and types description)
- **This file**: 008-RHI external ABI explicit table.
- **CMake Target Name**: **`te_rhi`** (project name `te_rhi`, not `TenEngine_RHI`). Downstream should use **`te_rhi`** in `target_link_libraries`, not `TenEngine_RHI`. Depends on `te_core`.
- **Platform & Interface**: Engine supports **Android, iOS** and other platforms (see 001-Core); RHI supports **Vulkan**, **Metal (MTL)**, **D3D12/DXIL**, **D3D11** graphics interfaces, and **GLSL**, **HLSL/DXIL**, **MSL** shader interfaces. **Macros can be used to determine which code path to execute** (e.g., TE_RHI_VULKAN, TE_RHI_METAL, TE_RHI_D3D12), selecting backend implementation at compile time.
- **Reference**: Vulkan (vkCmd*, VkBuffer/VkImage/VkDescriptorSet, vkCmdPipelineBarrier), Metal (MTLCommandEncoder, MTLBuffer/MTLTexture, setVertexBuffer/setFragmentTexture, drawIndexedPrimitives), D3D12 (ID3D12GraphicsCommandList, ResourceBarrier, DrawIndexedInstanced), Unreal FRHICommandList, Unity low-level graphics wrapper.
- **Render Resource Explicit Control Location**: **Prepare/Create/Update GPU resources** (**CreateDeviceResource**, **UpdateDeviceResource**) provided by this module; **Submit to actual GPU Command** (**SubmitCommandBuffer**) see ExecuteLogicalCommandBuffer / SubmitCommandList. Creating logical render resources, CollectCommandBuffer, PrepareRenderMaterial/Mesh see 019-PipelineCore/020-Pipeline.

## ABI Table

Column definitions: **Module | Namespace | Symbol/Type | Export Form | Interface Description | Header | Description**

### Types and Enums (te/rhi/types.hpp)

| Module | Namespace | Symbol | Export Form | Interface Description | Header | Description |
|--------|-----------|--------|-------------|----------------------|--------|-------------|
| 008-RHI | te::rhi | Backend | enum | Graphics backend | te/rhi/types.hpp | `enum class Backend : unsigned { Vulkan = 0, D3D12 = 1, Metal = 2, D3D11 = 3 };` |
| 008-RHI | te::rhi | DeviceLimits | struct | Device limits | te/rhi/types.hpp | `size_t maxBufferSize; uint32_t maxTextureDimension2D; uint32_t maxTextureDimension3D; size_t minUniformBufferOffsetAlignment;` |
| 008-RHI | te::rhi | QueueType | enum | Queue type | te/rhi/types.hpp | `enum class QueueType : unsigned { Graphics = 0, Compute = 1, Copy = 2 };` |
| 008-RHI | te::rhi | DeviceFeatures | struct | Device features (minimal set) | te/rhi/types.hpp | `uint32_t maxTextureDimension2D; uint32_t maxTextureDimension3D;` |
| 008-RHI | te::rhi | ResourceState | enum | Resource state (for barrier) | te/rhi/types.hpp | `enum class ResourceState : uint32_t { Common = 0, VertexBuffer = 1, IndexBuffer = 2, RenderTarget = 3, DepthWrite = 4, ShaderResource = 5, CopySrc = 6, CopyDst = 7, Present = 8 };` |
| 008-RHI | te::rhi | BufferBarrier | struct | Buffer barrier | te/rhi/types.hpp | `IBuffer* buffer; size_t offset; size_t size; ResourceState srcState; ResourceState dstState;` |
| 008-RHI | te::rhi | TextureBarrier | struct | Texture barrier | te/rhi/types.hpp | `ITexture* texture; uint32_t mipLevel; uint32_t arrayLayer; ResourceState srcState; ResourceState dstState;` |
| 008-RHI | te::rhi | IDevice, IQueue, ICommandList, IBuffer, ITexture, ISampler, IPSO, IFence, ISemaphore, ISwapChain, IRenderPass | forward decl | Handle types | te/rhi/types.hpp | Forward declarations only; defined in respective headers |

### Resource Descriptions (te/rhi/resources.hpp)

| Module | Namespace | Symbol | Export Form | Interface Description | Header | Description |
|--------|-----------|--------|-------------|----------------------|--------|-------------|
| 008-RHI | te::rhi | BufferUsage | enum | Buffer usage bits | te/rhi/resources.hpp | `enum class BufferUsage : uint32_t { Vertex = 1u << 0, Index = 1u << 1, Uniform = 1u << 2, Storage = 1u << 3, CopySrc = 1u << 4, CopyDst = 1u << 5 };` BufferDesc.usage uses this bitmask |
| 008-RHI | te::rhi | BufferDesc | struct | Buffer creation desc | te/rhi/resources.hpp | `size_t size; uint32_t usage;` **usage is BufferUsage bitmask; Uniform bit means usable for uniform buffer** |
| 008-RHI | te::rhi | TextureDesc | struct | Texture creation desc | te/rhi/resources.hpp | `uint32_t width, height, depth, format;` |
| 008-RHI | te::rhi | SamplerDesc | struct | Sampler creation desc | te/rhi/resources.hpp | `uint32_t filter;` |
| 008-RHI | te::rhi | ViewDesc | struct | View creation desc | te/rhi/resources.hpp | `void* resource; uint32_t type;` |
| 008-RHI | te::rhi | ViewHandle | type alias | View handle | te/rhi/resources.hpp | `using ViewHandle = uintptr_t;` |
| 008-RHI | te::rhi | IBuffer | abstract interface | GPU buffer | te/rhi/resources.hpp | Virtual destructor; no other members |
| 008-RHI | te::rhi | ITexture | abstract interface | GPU texture | te/rhi/resources.hpp | Virtual destructor; no other members |
| 008-RHI | te::rhi | ISampler | abstract interface | Sampler | te/rhi/resources.hpp | Virtual destructor; no other members |

### PSO (te/rhi/pso.hpp)

| Module | Namespace | Symbol | Export Form | Interface Description | Header | Description |
|--------|-----------|--------|-------------|----------------------|--------|-------------|
| 008-RHI | te::rhi | BlendFactor | enum | Blend factor | te/rhi/pso.hpp | `enum class BlendFactor : uint8_t { Zero = 0, One, SrcColor, OneMinusSrcColor, DstColor, OneMinusDstColor, SrcAlpha, OneMinusSrcAlpha, DstAlpha, OneMinusDstAlpha };` |
| 008-RHI | te::rhi | BlendOp | enum | Blend operation | te/rhi/pso.hpp | `enum class BlendOp : uint8_t { Add = 0, Subtract, ReverseSubtract, Min, Max };` |
| 008-RHI | te::rhi | CompareOp | enum | Depth/stencil compare | te/rhi/pso.hpp | `enum class CompareOp : uint8_t { Never = 0, Less, Equal, LessOrEqual, Greater, NotEqual, GreaterOrEqual, Always };` |
| 008-RHI | te::rhi | CullMode | enum | Cull mode | te/rhi/pso.hpp | `enum class CullMode : uint8_t { None = 0, Front, Back, FrontAndBack };` |
| 008-RHI | te::rhi | FrontFace | enum | Front face winding | te/rhi/pso.hpp | `enum class FrontFace : uint8_t { CounterClockwise = 0, Clockwise };` |
| 008-RHI | te::rhi | BlendAttachmentDesc | struct | Per-attachment blend | te/rhi/pso.hpp | `bool blendEnable; BlendFactor srcColorBlend, dstColorBlend; BlendOp colorBlendOp; BlendFactor srcAlphaBlend, dstAlphaBlend; BlendOp alphaBlendOp; uint8_t colorWriteMask;` |
| 008-RHI | te::rhi | DepthStencilStateDesc | struct | Depth/stencil state | te/rhi/pso.hpp | `bool depthTestEnable; bool depthWriteEnable; CompareOp depthCompareOp;` |
| 008-RHI | te::rhi | RasterizationStateDesc | struct | Rasterization state | te/rhi/pso.hpp | `CullMode cullMode; FrontFace frontFace;` |
| 008-RHI | te::rhi | GraphicsPipelineStateDesc | struct | Graphics pipeline state | te/rhi/pso.hpp | `BlendAttachmentDesc blendAttachments[8]; uint32_t blendAttachmentCount; DepthStencilStateDesc depthStencil; RasterizationStateDesc rasterization;` Optional for GraphicsPSODesc |
| 008-RHI | te::rhi | GraphicsPSODesc | struct | Graphics PSO desc | te/rhi/pso.hpp | `void const* vertex_shader; size_t vertex_shader_size; void const* fragment_shader; size_t fragment_shader_size; GraphicsPipelineStateDesc const* pipelineState;` |
| 008-RHI | te::rhi | ComputePSODesc | struct | Compute PSO desc | te/rhi/pso.hpp | `void const* compute_shader; size_t compute_shader_size;` |
| 008-RHI | te::rhi | IPSO | abstract interface | Pipeline state object | te/rhi/pso.hpp | Virtual destructor; no other members |

### Device & Queue (te/rhi/device.hpp, te/rhi/queue.hpp)

| Module | Namespace | Symbol | Export Form | Interface Description | Header | Description |
|--------|-----------|--------|-------------|----------------------|--------|-------------|
| 008-RHI | te::rhi | SelectBackend | free function | Set default backend | te/rhi/device.hpp | `void SelectBackend(Backend b);` |
| 008-RHI | te::rhi | GetSelectedBackend | free function | Get current default backend | te/rhi/device.hpp | `Backend GetSelectedBackend();` |
| 008-RHI | te::rhi | CreateDevice | free function | Create device | te/rhi/device.hpp | `IDevice* CreateDevice(Backend backend);` Returns nullptr on failure |
| 008-RHI | te::rhi | CreateDevice | free function | Create device (no args) | te/rhi/device.hpp | `IDevice* CreateDevice();` Uses GetSelectedBackend() |
| 008-RHI | te::rhi | DestroyDevice | free function | Destroy device | te/rhi/device.hpp | `void DestroyDevice(IDevice* device);` nullptr is no-op |
| 008-RHI | te::rhi | IDevice | abstract interface | Graphics device | te/rhi/device.hpp | See IDevice members table below |
| 008-RHI | te::rhi | IDevice::GetBackend | member | Return creation backend | te/rhi/device.hpp | `Backend GetBackend() const = 0;` |
| 008-RHI | te::rhi | IDevice::GetQueue | member | Get queue | te/rhi/device.hpp | `IQueue* GetQueue(QueueType type, uint32_t index) = 0;` Returns nullptr if out of bounds |
| 008-RHI | te::rhi | IDevice::GetFeatures | member | Device features | te/rhi/device.hpp | `DeviceFeatures const& GetFeatures() const = 0;` |
| 008-RHI | te::rhi | IDevice::GetLimits | member | Device limits | te/rhi/device.hpp | `DeviceLimits const& GetLimits() const = 0;` |
| 008-RHI | te::rhi | IDevice::CreateCommandList | member | Create command list | te/rhi/device.hpp | `ICommandList* CreateCommandList() = 0;` Returns nullptr on failure |
| 008-RHI | te::rhi | IDevice::DestroyCommandList | member | Destroy command list | te/rhi/device.hpp | `void DestroyCommandList(ICommandList* cmd) = 0;` |
| 008-RHI | te::rhi | IDevice::CreateBuffer | member | Create buffer | te/rhi/device.hpp | `IBuffer* CreateBuffer(BufferDesc const& desc) = 0;` Returns nullptr on failure |
| 008-RHI | te::rhi | IDevice::UpdateBuffer | member | CPU write to GPU buffer | te/rhi/device.hpp | `void UpdateBuffer(IBuffer* buf, size_t offset, void const* data, size_t size) = 0;` |
| 008-RHI | te::rhi | IDevice::CreateTexture | member | Create texture | te/rhi/device.hpp | `ITexture* CreateTexture(TextureDesc const& desc) = 0;` Returns nullptr on failure |
| 008-RHI | te::rhi | IDevice::CreateSampler | member | Create sampler | te/rhi/device.hpp | `ISampler* CreateSampler(SamplerDesc const& desc) = 0;` Returns nullptr on failure |
| 008-RHI | te::rhi | IDevice::CreateView | member | Create view | te/rhi/device.hpp | `ViewHandle CreateView(ViewDesc const& desc) = 0;` |
| 008-RHI | te::rhi | IDevice::DestroyBuffer | member | Destroy buffer | te/rhi/device.hpp | `void DestroyBuffer(IBuffer* b) = 0;` |
| 008-RHI | te::rhi | IDevice::DestroyTexture | member | Destroy texture | te/rhi/device.hpp | `void DestroyTexture(ITexture* t) = 0;` |
| 008-RHI | te::rhi | IDevice::DestroySampler | member | Destroy sampler | te/rhi/device.hpp | `void DestroySampler(ISampler* s) = 0;` |
| 008-RHI | te::rhi | IDevice::CreateGraphicsPSO | member | Create graphics PSO | te/rhi/device.hpp | `IPSO* CreateGraphicsPSO(GraphicsPSODesc const& desc) = 0;` Returns nullptr on failure |
| 008-RHI | te::rhi | IDevice::CreateGraphicsPSO | member (overload) | Create graphics PSO with layout | te/rhi/device.hpp | `IPSO* CreateGraphicsPSO(GraphicsPSODesc const& desc, IDescriptorSetLayout* layout) = 0;` layout non-null couples PSO with layout |
| 008-RHI | te::rhi | IDevice::CreateGraphicsPSO | member (overload) | Create graphics PSO with pass | te/rhi/device.hpp | `IPSO* CreateGraphicsPSO(GraphicsPSODesc const& desc, IDescriptorSetLayout* layout, IRenderPass* pass, uint32_t subpassIndex, IDescriptorSetLayout* layoutSet1 = nullptr) = 0;` For render pass and subpass-specific PSO; layoutSet1 for second descriptor set (e.g., skin) |
| 008-RHI | te::rhi | IDevice::CreateRenderPass | member | Create render pass | te/rhi/device.hpp | `IRenderPass* CreateRenderPass(RenderPassDesc const& desc) = 0;` Returns nullptr on failure |
| 008-RHI | te::rhi | IDevice::DestroyRenderPass | member | Destroy render pass | te/rhi/device.hpp | `void DestroyRenderPass(IRenderPass* pass) = 0;` |
| 008-RHI | te::rhi | IDevice::CreateComputePSO | member | Create compute PSO | te/rhi/device.hpp | `IPSO* CreateComputePSO(ComputePSODesc const& desc) = 0;` Returns nullptr on failure |
| 008-RHI | te::rhi | IDevice::SetShader | member | Bind shader data | te/rhi/device.hpp | `void SetShader(IPSO* pso, void const* data, size_t size) = 0;` |
| 008-RHI | te::rhi | IDevice::Cache | member | PSO cache | te/rhi/device.hpp | `void Cache(IPSO* pso) = 0;` |
| 008-RHI | te::rhi | IDevice::DestroyPSO | member | Destroy PSO | te/rhi/device.hpp | `void DestroyPSO(IPSO* pso) = 0;` |
| 008-RHI | te::rhi | IDevice::CreateFence | member | Create Fence | te/rhi/device.hpp | `IFence* CreateFence(bool initialSignaled = false) = 0;` Returns nullptr on failure |
| 008-RHI | te::rhi | IDevice::CreateSemaphore | member | Create Semaphore | te/rhi/device.hpp | `ISemaphore* CreateSemaphore() = 0;` Returns nullptr on failure |
| 008-RHI | te::rhi | IDevice::DestroyFence | member | Destroy Fence | te/rhi/device.hpp | `void DestroyFence(IFence* f) = 0;` |
| 008-RHI | te::rhi | IDevice::DestroySemaphore | member | Destroy Semaphore | te/rhi/device.hpp | `void DestroySemaphore(ISemaphore* s) = 0;` |
| 008-RHI | te::rhi | IDevice::CreateSwapChain | member | Create swapchain | te/rhi/device.hpp | `ISwapChain* CreateSwapChain(SwapChainDesc const& desc) = 0;` Returns nullptr on failure |
| 008-RHI | te::rhi | IDevice::CreateDescriptorSetLayout | member | Create descriptor set layout | te/rhi/device.hpp | `IDescriptorSetLayout* CreateDescriptorSetLayout(DescriptorSetLayoutDesc const& desc) = 0;` Returns nullptr on failure |
| 008-RHI | te::rhi | IDevice::AllocateDescriptorSet | member | Allocate descriptor set | te/rhi/device.hpp | `IDescriptorSet* AllocateDescriptorSet(IDescriptorSetLayout* layout) = 0;` Returns nullptr on failure |
| 008-RHI | te::rhi | IDevice::UpdateDescriptorSet | member | Update descriptor | te/rhi/device.hpp | `void UpdateDescriptorSet(IDescriptorSet* set, DescriptorWrite const* writes, uint32_t writeCount) = 0;` |
| 008-RHI | te::rhi | IDevice::DestroyDescriptorSetLayout | member | Destroy descriptor set layout | te/rhi/device.hpp | `void DestroyDescriptorSetLayout(IDescriptorSetLayout* layout) = 0;` |
| 008-RHI | te::rhi | IDevice::DestroyDescriptorSet | member | Destroy descriptor set | te/rhi/device.hpp | `void DestroyDescriptorSet(IDescriptorSet* set) = 0;` |
| 008-RHI | te::rhi | IQueue | abstract interface | Queue | te/rhi/queue.hpp | See IQueue members table below |
| 008-RHI | te::rhi | IQueue::Submit | member | Submit command list | te/rhi/queue.hpp | `void Submit(ICommandList* cmdList, IFence* signalFence = nullptr, ISemaphore* waitSemaphore = nullptr, ISemaphore* signalSemaphore = nullptr) = 0;` |
| 008-RHI | te::rhi | IQueue::WaitIdle | member | Wait for queue idle | te/rhi/queue.hpp | `void WaitIdle() = 0;` |

### Command List (te/rhi/command_list.hpp)

| Module | Namespace | Symbol | Export Form | Interface Description | Header | Description |
|--------|-----------|--------|-------------|----------------------|--------|-------------|
| 008-RHI | te::rhi | ICommandList | abstract interface | Command buffer | te/rhi/command_list.hpp | See ICommandList members table below |
| 008-RHI | te::rhi | ICommandList::Begin | member | Begin recording | te/rhi/command_list.hpp | `void Begin() = 0;` |
| 008-RHI | te::rhi | ICommandList::End | member | End recording | te/rhi/command_list.hpp | `void End() = 0;` |
| 008-RHI | te::rhi | LoadOp | enum | Render attachment load op | te/rhi/command_list.hpp | `enum class LoadOp : uint32_t { Load = 0, Clear = 1, DontCare = 2 };` |
| 008-RHI | te::rhi | StoreOp | enum | Render attachment store op | te/rhi/command_list.hpp | `enum class StoreOp : uint32_t { Store = 0, DontCare = 1 };` |
| 008-RHI | te::rhi | kMaxColorAttachments | constant | Max color attachments | te/rhi/command_list.hpp | `constexpr uint32_t kMaxColorAttachments = 8u;` |
| 008-RHI | te::rhi | kMaxSubpasses | constant | Max subpasses | te/rhi/command_list.hpp | `constexpr uint32_t kMaxSubpasses = 8u;` |
| 008-RHI | te::rhi | kDepthStencilAttachmentIndexNone | constant | No depth attachment | te/rhi/command_list.hpp | `constexpr uint32_t kDepthStencilAttachmentIndexNone = 0xFFFFFFFFu;` |
| 008-RHI | te::rhi | Viewport | struct | Viewport | te/rhi/command_list.hpp | `float x, y, width, height, minDepth, maxDepth;` |
| 008-RHI | te::rhi | ScissorRect | struct | Scissor rect | te/rhi/command_list.hpp | `int32_t x, y; uint32_t width, height;` |
| 008-RHI | te::rhi | RenderPassColorAttachment | struct | Color attachment | te/rhi/command_list.hpp | `ITexture* texture; uint32_t format; LoadOp loadOp; StoreOp storeOp; float clearColor[4];` |
| 008-RHI | te::rhi | RenderPassDepthStencilAttachment | struct | Depth/stencil attachment | te/rhi/command_list.hpp | `ITexture* texture; uint32_t format; LoadOp loadOp; StoreOp storeOp; float clearDepth; uint32_t clearStencil;` |
| 008-RHI | te::rhi | SubpassDesc | struct | Subpass description | te/rhi/command_list.hpp | `uint32_t colorAttachmentCount; uint32_t colorAttachmentIndices[kMaxColorAttachments]; uint32_t depthStencilAttachmentIndex;` |
| 008-RHI | te::rhi | RenderPassDesc | struct | Render pass desc | te/rhi/command_list.hpp | `uint32_t colorAttachmentCount; RenderPassColorAttachment colorAttachments[kMaxColorAttachments]; RenderPassDepthStencilAttachment depthStencilAttachment; LoadOp colorLoadOp, depthLoadOp; StoreOp colorStoreOp, depthStoreOp; uint32_t subpassCount; SubpassDesc subpasses[kMaxSubpasses];` |
| 008-RHI | te::rhi | BufferRegion | struct | Buffer copy region | te/rhi/command_list.hpp | `IBuffer* buffer; size_t offset; size_t size;` |
| 008-RHI | te::rhi | TextureRegion | struct | Texture copy region | te/rhi/command_list.hpp | `ITexture* texture; uint32_t mipLevel, arrayLayer; uint32_t x, y, z; uint32_t width, height, depth;` |
| 008-RHI | te::rhi | IRenderPass | abstract interface | Render pass | te/rhi/command_list.hpp | `virtual uint32_t GetSubpassColorAttachmentCount(uint32_t subpassIndex) const;` For PSO creation |
| 008-RHI | te::rhi | ICommandList::Draw | member | Non-indexed draw | te/rhi/command_list.hpp | `void Draw(uint32_t vertex_count, uint32_t instance_count = 1, uint32_t first_vertex = 0, uint32_t first_instance = 0) = 0;` |
| 008-RHI | te::rhi | ICommandList::DrawIndexed | member | Indexed draw | te/rhi/command_list.hpp | `void DrawIndexed(uint32_t index_count, uint32_t instance_count = 1, uint32_t first_index = 0, int32_t vertex_offset = 0, uint32_t first_instance = 0) = 0;` |
| 008-RHI | te::rhi | ICommandList::SetViewport | member | Set viewport | te/rhi/command_list.hpp | `void SetViewport(uint32_t first, uint32_t count, Viewport const* viewports) = 0;` |
| 008-RHI | te::rhi | ICommandList::SetScissor | member | Set scissor | te/rhi/command_list.hpp | `void SetScissor(uint32_t first, uint32_t count, ScissorRect const* scissors) = 0;` |
| 008-RHI | te::rhi | ICommandList::SetUniformBuffer | member | Bind IBuffer to slot | te/rhi/command_list.hpp | `void SetUniformBuffer(uint32_t slot, IBuffer* buffer, size_t offset) = 0;` |
| 008-RHI | te::rhi | ICommandList::SetVertexBuffer | member | Bind vertex buffer | te/rhi/command_list.hpp | `void SetVertexBuffer(uint32_t slot, IBuffer* buffer, size_t offset, uint32_t stride) = 0;` |
| 008-RHI | te::rhi | ICommandList::SetIndexBuffer | member | Bind index buffer | te/rhi/command_list.hpp | `void SetIndexBuffer(IBuffer* buffer, size_t offset, uint32_t indexFormat) = 0;` indexFormat: 0=16bit, 1=32bit |
| 008-RHI | te::rhi | ICommandList::SetGraphicsPSO | member | Bind graphics PSO | te/rhi/command_list.hpp | `void SetGraphicsPSO(IPSO* pso) = 0;` |
| 008-RHI | te::rhi | ICommandList::BindDescriptorSet | member | Bind material descriptor set | te/rhi/command_list.hpp | `void BindDescriptorSet(IDescriptorSet* set) = 0;` Binds to set 0 |
| 008-RHI | te::rhi | ICommandList::BindDescriptorSet | member (overload) | Bind descriptor set at index | te/rhi/command_list.hpp | `void BindDescriptorSet(uint32_t setIndex, IDescriptorSet* set) = 0;` Binds to specified set index |
| 008-RHI | te::rhi | ICommandList::BeginRenderPass | member | Begin render pass | te/rhi/command_list.hpp | `void BeginRenderPass(RenderPassDesc const& desc, IRenderPass* pass = nullptr) = 0;` |
| 008-RHI | te::rhi | ICommandList::NextSubpass | member | Transition to next subpass | te/rhi/command_list.hpp | `void NextSubpass() = 0;` |
| 008-RHI | te::rhi | ICommandList::EndRenderPass | member | End render pass | te/rhi/command_list.hpp | `void EndRenderPass() = 0;` |
| 008-RHI | te::rhi | ICommandList::BeginOcclusionQuery | member | Begin occlusion query | te/rhi/command_list.hpp | `void BeginOcclusionQuery(uint32_t queryIndex) = 0;` |
| 008-RHI | te::rhi | ICommandList::EndOcclusionQuery | member | End occlusion query | te/rhi/command_list.hpp | `void EndOcclusionQuery(uint32_t queryIndex) = 0;` |
| 008-RHI | te::rhi | ICommandList::CopyBuffer | member | Buffer-to-buffer copy | te/rhi/command_list.hpp | `void CopyBuffer(IBuffer* src, size_t srcOffset, IBuffer* dst, size_t dstOffset, size_t size) = 0;` |
| 008-RHI | te::rhi | ICommandList::CopyBufferToTexture | member | Buffer-to-texture copy | te/rhi/command_list.hpp | `void CopyBufferToTexture(IBuffer* src, size_t srcOffset, ITexture* dst, TextureRegion const& dstRegion) = 0;` |
| 008-RHI | te::rhi | ICommandList::CopyTextureToBuffer | member | Texture-to-buffer copy | te/rhi/command_list.hpp | `void CopyTextureToBuffer(ITexture* src, TextureRegion const& srcRegion, IBuffer* dst, size_t dstOffset) = 0;` |
| 008-RHI | te::rhi | ICommandList::BuildAccelerationStructure | member | Build ray tracing AS (D3D12) | te/rhi/command_list.hpp | `void BuildAccelerationStructure(RaytracingAccelerationStructureDesc const& desc, IBuffer* scratch, IBuffer* result) = 0;` |
| 008-RHI | te::rhi | ICommandList::DispatchRays | member | Dispatch rays (D3D12) | te/rhi/command_list.hpp | `void DispatchRays(DispatchRaysDesc const& desc) = 0;` |
| 008-RHI | te::rhi | ICommandList::Dispatch | member | Compute dispatch | te/rhi/command_list.hpp | `void Dispatch(uint32_t x, uint32_t y = 1, uint32_t z = 1) = 0;` |
| 008-RHI | te::rhi | ICommandList::Copy | member | Memory copy | te/rhi/command_list.hpp | `void Copy(void const* src, void* dst, size_t size) = 0;` |
| 008-RHI | te::rhi | ICommandList::ResourceBarrier | member | Resource barrier | te/rhi/command_list.hpp | `void ResourceBarrier(uint32_t bufferBarrierCount, BufferBarrier const* bufferBarriers, uint32_t textureBarrierCount, TextureBarrier const* textureBarriers) = 0;` |
| 008-RHI | te::rhi | Begin | free function | Begin recording | te/rhi/command_list.hpp | `void Begin(ICommandList* cmd);` |
| 008-RHI | te::rhi | End | free function | End recording | te/rhi/command_list.hpp | `void End(ICommandList* cmd);` |
| 008-RHI | te::rhi | Submit | free function | Submit to queue | te/rhi/command_list.hpp | `void Submit(ICommandList* cmd, IQueue* queue);` or `void Submit(ICommandList* cmd, IQueue* queue, IFence* signalFence, ISemaphore* waitSem, ISemaphore* signalSem);` |

### Sync (te/rhi/sync.hpp)

| Module | Namespace | Symbol | Export Form | Interface Description | Header | Description |
|--------|-----------|--------|-------------|----------------------|--------|-------------|
| 008-RHI | te::rhi | IFence | abstract interface | Fence | te/rhi/sync.hpp | `void Wait() = 0; void Signal() = 0; void Reset() = 0;` |
| 008-RHI | te::rhi | ISemaphore | abstract interface | Semaphore | te/rhi/sync.hpp | Virtual destructor; no other members |
| 008-RHI | te::rhi | Wait | free function | Fence wait | te/rhi/sync.hpp | `void Wait(IFence* f);` Calls f->Wait() internally |
| 008-RHI | te::rhi | Signal | free function | Fence signal | te/rhi/sync.hpp | `void Signal(IFence* f);` Calls f->Signal() internally |

### SwapChain (te/rhi/swapchain.hpp)

| Module | Namespace | Symbol | Export Form | Interface Description | Header | Description |
|--------|-----------|--------|-------------|----------------------|--------|-------------|
| 008-RHI | te::rhi | VSyncMode | enum | VSync mode | te/rhi/swapchain.hpp | `enum class VSyncMode : uint8_t { Off = 0, On = 1, Adaptive = 2, Mailbox = 3 };` |
| 008-RHI | te::rhi | ColorSpace | enum | Color space | te/rhi/swapchain.hpp | `enum class ColorSpace : uint8_t { SRGB = 0, HDR10_ST2084 = 1, HDR10_HLG = 2, scRGB = 3, DisplayNative = 4 };` |
| 008-RHI | te::rhi | PresentMode | enum | Present mode | te/rhi/swapchain.hpp | `enum class PresentMode : uint8_t { Immediate = 0, FIFO = 1, FIFO_Relaxed = 2, Mailbox = 3 };` |
| 008-RHI | te::rhi | HDRMetadata | struct | HDR metadata | te/rhi/swapchain.hpp | Primary and luminance fields for HDR10 |
| 008-RHI | te::rhi | SwapChainDesc | struct | Swapchain creation desc | te/rhi/swapchain.hpp | `void* windowHandle; uint32_t width, height, format, bufferCount; bool vsync; VSyncMode vsyncMode; ColorSpace colorSpace; PresentMode presentMode; bool enableHDR; bool allowTearing; HDRMetadata hdrMetadata;` |
| 008-RHI | te::rhi | ISwapChain | abstract interface | Swapchain | te/rhi/swapchain.hpp | See ISwapChain members table below |
| 008-RHI | te::rhi | ISwapChain::Present | member | Present | te/rhi/swapchain.hpp | `bool Present() = 0;` |
| 008-RHI | te::rhi | ISwapChain::GetCurrentBackBuffer | member | Current back buffer | te/rhi/swapchain.hpp | `ITexture* GetCurrentBackBuffer() = 0;` |
| 008-RHI | te::rhi | ISwapChain::GetCurrentBackBufferIndex | member | Current back buffer index | te/rhi/swapchain.hpp | `uint32_t GetCurrentBackBufferIndex() const = 0;` |
| 008-RHI | te::rhi | ISwapChain::Resize | member | Resize | te/rhi/swapchain.hpp | `void Resize(uint32_t width, uint32_t height) = 0;` |
| 008-RHI | te::rhi | ISwapChain::GetWidth | member | Width | te/rhi/swapchain.hpp | `uint32_t GetWidth() const = 0;` |
| 008-RHI | te::rhi | ISwapChain::GetHeight | member | Height | te/rhi/swapchain.hpp | `uint32_t GetHeight() const = 0;` |
| 008-RHI | te::rhi | ISwapChain::SetVSyncMode | member | Set VSync mode | te/rhi/swapchain.hpp | `virtual bool SetVSyncMode(VSyncMode mode);` Default returns false |
| 008-RHI | te::rhi | ISwapChain::GetVSyncMode | member | Get VSync mode | te/rhi/swapchain.hpp | `virtual VSyncMode GetVSyncMode() const;` Default returns VSyncMode::On |
| 008-RHI | te::rhi | ISwapChain::SetHDRMode | member | Set HDR mode | te/rhi/swapchain.hpp | `virtual bool SetHDRMode(bool enable, ColorSpace colorSpace);` Default returns false |
| 008-RHI | te::rhi | ISwapChain::IsHDREnabled | member | Is HDR enabled | te/rhi/swapchain.hpp | `virtual bool IsHDREnabled() const;` Default returns false |
| 008-RHI | te::rhi | ISwapChain::GetColorSpace | member | Get color space | te/rhi/swapchain.hpp | `virtual ColorSpace GetColorSpace() const;` Default returns ColorSpace::SRGB |
| 008-RHI | te::rhi | ISwapChain::SetHDRMetadata | member | Set HDR metadata | te/rhi/swapchain.hpp | `virtual bool SetHDRMetadata(HDRMetadata const& metadata);` Default returns false |
| 008-RHI | te::rhi | ISwapChain::SupportsHDR | member | Supports HDR | te/rhi/swapchain.hpp | `virtual bool SupportsHDR() const;` Default returns false |
| 008-RHI | te::rhi | ISwapChain::SupportsTearing | member | Supports tearing | te/rhi/swapchain.hpp | `virtual bool SupportsTearing() const;` Default returns false |
| 008-RHI | te::rhi | ISwapChain::GetRefreshRate | member | Get refresh rate | te/rhi/swapchain.hpp | `virtual uint32_t GetRefreshRate() const;` Default returns 60 |
| 008-RHI | te::rhi | ISwapChain::GetBackBuffer | member | Back buffer (compat) | te/rhi/swapchain.hpp | `ITexture* GetBackBuffer();` Backward compat, calls GetCurrentBackBuffer() |

### Descriptor Set (te/rhi/descriptor_set.hpp)

| Module | Namespace | Symbol | Export Form | Interface Description | Header | Description |
|--------|-----------|--------|-------------|----------------------|--------|-------------|
| 008-RHI | te::rhi | DescriptorType | enum | Descriptor type | te/rhi/descriptor_set.hpp | `enum class DescriptorType : uint32_t { UniformBuffer = 0, CombinedImageSampler = 1, Sampler = 2, StorageBuffer = 3, StorageImage = 4 };` Maps to VK_DESCRIPTOR_TYPE_* |
| 008-RHI | te::rhi | DescriptorSetLayoutBinding | struct | Layout binding | te/rhi/descriptor_set.hpp | `uint32_t binding; uint32_t descriptorType; uint32_t descriptorCount;` |
| 008-RHI | te::rhi | DescriptorSetLayoutDesc | struct | Layout desc | te/rhi/descriptor_set.hpp | `static constexpr uint32_t kMaxBindings = 16u; DescriptorSetLayoutBinding bindings[kMaxBindings]; uint32_t bindingCount;` |
| 008-RHI | te::rhi | DescriptorWrite | struct | Descriptor write | te/rhi/descriptor_set.hpp | `IDescriptorSet* dstSet; uint32_t binding; uint32_t type; IBuffer* buffer; size_t bufferOffset; ITexture* texture; ISampler* sampler;` bufferOffset for UB ring offset |
| 008-RHI | te::rhi | IDescriptorSetLayout | abstract interface | Descriptor set layout | te/rhi/descriptor_set.hpp | Virtual destructor |
| 008-RHI | te::rhi | IDescriptorSet | abstract interface | Descriptor set | te/rhi/descriptor_set.hpp | Virtual destructor |

### Ray Tracing (te/rhi/raytracing.hpp) (D3D12 only)

| Module | Namespace | Symbol | Export Form | Interface Description | Header | Description |
|--------|-----------|--------|-------------|----------------------|--------|-------------|
| 008-RHI | te::rhi | RaytracingAccelerationStructureDesc | struct | RT AS desc | te/rhi/raytracing.hpp | `uint32_t type; void* geometryDesc; size_t geometryCount;` BLAS/TLAS |
| 008-RHI | te::rhi | DispatchRaysDesc | struct | Dispatch rays desc | te/rhi/raytracing.hpp | `IBuffer* raygenShaderTable; size_t raygenSize, raygenStride; IBuffer* missShaderTable; size_t missSize, missStride; IBuffer* hitGroupTable; size_t hitGroupSize, hitGroupStride; uint32_t width, height, depth;` Maps to D3D12_DISPATCH_RAYS_DESC |

### Backend Factories (te/rhi/backend_*.hpp)

| Module | Namespace | Symbol | Export Form | Interface Description | Header | Description |
|--------|-----------|--------|-------------|----------------------|--------|-------------|
| 008-RHI | te::rhi | CreateDeviceVulkan | free function | Create Vulkan device | te/rhi/backend_vulkan.hpp | `IDevice* CreateDeviceVulkan();` Returns nullptr on failure |
| 008-RHI | te::rhi | DestroyDeviceVulkan | free function | Destroy Vulkan device | te/rhi/backend_vulkan.hpp | `void DestroyDeviceVulkan(IDevice* device);` |
| 008-RHI | te::rhi | CreateDeviceD3D12 | free function | Create D3D12 device | te/rhi/backend_d3d12.hpp | `IDevice* CreateDeviceD3D12();` Returns nullptr on failure |
| 008-RHI | te::rhi | DestroyDeviceD3D12 | free function | Destroy D3D12 device | te/rhi/backend_d3d12.hpp | `void DestroyDeviceD3D12(IDevice* device);` |
| 008-RHI | te::rhi | CreateDeviceD3D11 | free function | Create D3D11 device | te/rhi/backend_d3d11.hpp | `IDevice* CreateDeviceD3D11();` Returns nullptr on failure |
| 008-RHI | te::rhi | DestroyDeviceD3D11 | free function | Destroy D3D11 device | te/rhi/backend_d3d11.hpp | `void DestroyDeviceD3D11(IDevice* device);` |
| 008-RHI | te::rhi | CreateDeviceMetal | free function | Create Metal device | te/rhi/backend_metal.hpp | `IDevice* CreateDeviceMetal();` Returns nullptr on failure |
| 008-RHI | te::rhi | DestroyDeviceMetal | free function | Destroy Metal device | te/rhi/backend_metal.hpp | `void DestroyDeviceMetal(IDevice* device);` |

### Header Files and Include Relationships

| Header | Dependencies | Description |
|--------|--------------|-------------|
| te/rhi/types.hpp | <cstdint>, <cstddef> | Backend, QueueType, DeviceFeatures, ResourceState, BufferBarrier, TextureBarrier, forward declarations |
| te/rhi/resources.hpp | te/rhi/types.hpp, <cstddef> | BufferDesc, TextureDesc, SamplerDesc, ViewDesc, ViewHandle, IBuffer, ITexture, ISampler |
| te/rhi/pso.hpp | te/rhi/types.hpp, <cstddef> | GraphicsPSODesc, ComputePSODesc, IPSO, BlendFactor, BlendOp, CompareOp, CullMode, FrontFace, BlendAttachmentDesc, DepthStencilStateDesc, RasterizationStateDesc, GraphicsPipelineStateDesc |
| te/rhi/queue.hpp | te/rhi/types.hpp | IQueue |
| te/rhi/sync.hpp | te/rhi/types.hpp | IFence, ISemaphore, Wait, Signal |
| te/rhi/swapchain.hpp | te/rhi/types.hpp, <cstdint> | SwapChainDesc, ISwapChain, VSyncMode, ColorSpace, PresentMode, HDRMetadata |
| te/rhi/descriptor_set.hpp | te/rhi/types.hpp, te/rhi/resources.hpp | DescriptorType, DescriptorSetLayoutBinding, DescriptorSetLayoutDesc, DescriptorWrite, IDescriptorSetLayout, IDescriptorSet |
| te/rhi/raytracing.hpp | te/rhi/types.hpp, te/rhi/resources.hpp | RaytracingAccelerationStructureDesc, DispatchRaysDesc |
| te/rhi/command_list.hpp | te/rhi/types.hpp, te/rhi/resources.hpp, te/rhi/descriptor_set.hpp, te/rhi/raytracing.hpp | ICommandList, IRenderPass, Begin, End, Submit, LoadOp, StoreOp, Viewport, ScissorRect, RenderPassDesc, SubpassDesc, BufferRegion, TextureRegion |
| te/rhi/device.hpp | all above | IDevice, SelectBackend, GetSelectedBackend, CreateDevice, DestroyDevice, CreateGraphicsPSO overloads |

---

*Source: Contract capabilities Device & Queue, Command List, Resource Management, PSO, Sync, SwapChain, Descriptor Set; interfaces with 009-RenderCore, 010-Shader, 012-Mesh, 019-PipelineCore, 020-Pipeline, 028-Texture.*

## Change Log

| Date | Change Description |
|------|-------------------|
| 2026-02-10 | ABI sync: ICommandList added SetVertexBuffer, SetIndexBuffer, SetGraphicsPSO, BeginOcclusionQuery, EndOcclusionQuery |
| 2026-02-10 | Descriptor and PSO: ICommandList::BindDescriptorSet; IDevice::CreateGraphicsPSO(desc, layout) overload; DescriptorType, DescriptorWrite.bufferOffset; CreateDescriptorSetLayout/AllocateDescriptorSet/UpdateDescriptorSet implemented |
| 2026-02-22 | Code-aligned update: added IRenderPass, multi-subpass (NextSubpass, SubpassDesc, kMaxSubpasses), BindDescriptorSet(setIndex) overload, CreateGraphicsPSO with renderPass/subpass/layoutSet1 overloads, extended swapchain (VSyncMode, ColorSpace, PresentMode, HDRMetadata, HDR methods), ray tracing (BuildAccelerationStructure, DispatchRays, RaytracingAccelerationStructureDesc, DispatchRaysDesc), PSO enums (BlendFactor, BlendOp, CompareOp, CullMode, FrontFace, BlendAttachmentDesc, DepthStencilStateDesc, RasterizationStateDesc, GraphicsPipelineStateDesc), backend factories |
