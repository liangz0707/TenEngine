# 009-RenderCore Module ABI

- **Contract**: [009-rendercore-public-api.md](./009-rendercore-public-api.md) (capabilities and types description)
- **This file**: 009-RenderCore external ABI explicit table.
- **CMake Target Name**: **`te_rendercore`** (consistent with `te_rhi`, `te_core` naming style). Downstream should use **`te_rendercore`** in `target_link_libraries`. Depends on upstream target: **`te_rhi`** (008-RHI) and **`te_core`** (001-Core, via te_rhi).
- **Namespace**: **`te::rendercore`** (consistent with `te::rhi`, `te::core` style). Implementation files use this namespace uniformly.
- **Header Path**: **`te/rendercore/`** (consistent with `te/rhi/`, `te/core/` style).
- **Role**: Render types and Pass protocol, between RHI and pipeline; does not directly hold GPU resources, bridges to RHI.

## ABI Table

Column definitions: **Module | Namespace | Symbol/Type | Export Form | Interface Description | Header | Description**

### Types and Handles (te/rendercore/types.hpp)

| Module | Namespace | Symbol | Export Form | Interface Description | Header | Description |
|--------|-----------|--------|-------------|----------------------|--------|-------------|
| 009-RenderCore | te::rendercore | ResultCode | enum | Result code | te/rendercore/types.hpp | `enum class ResultCode : uint32_t { Success = 0, InvalidHandle, UnsupportedFormat, UnsupportedSize, ValidationFailed, RingBufferExhausted, Unknown };` |
| 009-RenderCore | te::rendercore | PassHandle | struct | Pass handle | te/rendercore/types.hpp | `struct PassHandle { uint64_t id = 0; bool IsValid() const; };` Opaque handle; used for DeclareRead/DeclareWrite |
| 009-RenderCore | te::rendercore | ResourceHandle | struct | Resource handle | te/rendercore/types.hpp | `struct ResourceHandle { uint64_t id = 0; bool IsValid() const; };` Pipeline resource handle; consistent with PipelineCore resource lifetime |
| 009-RenderCore | te::rendercore | FrameSlotId | type alias | Frame slot | te/rendercore/types.hpp | `using FrameSlotId = uint32_t;` Used for RingBuffer multi-frame slot |
| 009-RenderCore | te::rendercore | ResourceLifetime | enum | Resource lifetime | te/rendercore/types.hpp | `enum class ResourceLifetime : uint8_t { Transient, Persistent, External };` Interfaces with PipelineCore RDG |
| 009-RenderCore | te::rendercore | BindSlot | struct | Bind slot | te/rendercore/types.hpp | `struct BindSlot { uint32_t set = 0; uint32_t binding = 0; };` Consistent with Shader/RHI slot convention |

### Resource Descriptions (te/rendercore/resource_desc.hpp)

| Module | Namespace | Symbol | Export Form | Interface Description | Header | Description |
|--------|-----------|--------|-------------|----------------------|--------|-------------|
| 009-RenderCore | te::rendercore | VertexAttributeFormat | enum | Vertex attribute format | te/rendercore/resource_desc.hpp | `enum class VertexAttributeFormat : uint8_t { Float, Float2, Float3, Float4, Int, Int2, Int3, Int4, UInt, UInt2, UInt3, UInt4, Unknown };` |
| 009-RenderCore | te::rendercore | VertexAttribute | struct | Vertex attribute | te/rendercore/resource_desc.hpp | `uint32_t location = 0; VertexAttributeFormat format = VertexAttributeFormat::Unknown; uint32_t offset = 0;` |
| 009-RenderCore | te::rendercore | kMaxVertexAttributes | constant | Max vertex attributes | te/rendercore/resource_desc.hpp | `constexpr size_t kMaxVertexAttributes = 16;` |
| 009-RenderCore | te::rendercore | VertexFormat | struct | Vertex format | te/rendercore/resource_desc.hpp | `VertexAttribute attributes[kMaxVertexAttributes]; uint32_t attributeCount = 0; uint32_t stride = 0; bool IsValid() const;` Interfaces with RHI creation |
| 009-RenderCore | te::rendercore | VertexFormatDesc | struct | Vertex format description | te/rendercore/resource_desc.hpp | `VertexAttribute const* attributes = nullptr; uint32_t attributeCount = 0; uint32_t stride = 0;` CreateVertexFormat input |
| 009-RenderCore | te::rendercore | IndexType | enum | Index type | te/rendercore/resource_desc.hpp | `enum class IndexType : uint8_t { UInt16, UInt32, Unknown };` |
| 009-RenderCore | te::rendercore | IndexFormat | struct | Index format | te/rendercore/resource_desc.hpp | `IndexType type = IndexType::Unknown; bool IsValid() const;` Interfaces with RHI |
| 009-RenderCore | te::rendercore | IndexFormatDesc | struct | Index format description | te/rendercore/resource_desc.hpp | `IndexType type = IndexType::Unknown;` CreateIndexFormat input |
| 009-RenderCore | te::rendercore | TextureFormat | enum | Texture format | te/rendercore/resource_desc.hpp | `enum class TextureFormat : uint16_t { Unknown = 0, R8_UNorm, RG8_UNorm, RGBA8_UNorm, RGBA8_SRGB, BGRA8_UNorm, R16_Float, RG16_Float, RGBA16_Float, R32_Float, RG32_Float, RGBA32_Float, Depth16, Depth24_Stencil8, Depth32_Float };` |
| 009-RenderCore | te::rendercore | TextureUsage | enum | Texture usage | te/rendercore/resource_desc.hpp | `enum class TextureUsage : uint32_t { None = 0, Sampled = 1u << 0, Storage = 1u << 1, RenderTarget = 1u << 2, DepthStencil = 1u << 3, TransferSrc = 1u << 4, TransferDst = 1u << 5 };` Bitmask |
| 009-RenderCore | te::rendercore | TextureDesc | struct | Texture description | te/rendercore/resource_desc.hpp | `uint32_t width = 0, height = 0, depth = 1, mipLevels = 1; TextureFormat format = TextureFormat::Unknown; TextureUsage usage = TextureUsage::None; bool IsValid() const;` Bridges to RHI resource creation |
| 009-RenderCore | te::rendercore | TextureDescParams | struct | Texture description params | te/rendercore/resource_desc.hpp | `uint32_t width = 0, height = 0, depth = 1, mipLevels = 1; TextureFormat format = TextureFormat::Unknown; TextureUsage usage = TextureUsage::None;` CreateTextureDesc input |
| 009-RenderCore | te::rendercore | BufferUsage | enum | Buffer usage | te/rendercore/resource_desc.hpp | `enum class BufferUsage : uint32_t { None = 0, Vertex = 1u << 0, Index = 1u << 1, Uniform = 1u << 2, Storage = 1u << 3, TransferSrc = 1u << 4, TransferDst = 1u << 5 };` Bitmask |
| 009-RenderCore | te::rendercore | BufferDesc | struct | Buffer description | te/rendercore/resource_desc.hpp | `uint64_t size = 0; BufferUsage usage = BufferUsage::None; uint32_t alignment = 0; bool IsValid() const;` Bridges to RHI resource creation |
| 009-RenderCore | te::rendercore | BufferDescParams | struct | Buffer description params | te/rendercore/resource_desc.hpp | `uint64_t size = 0; BufferUsage usage = BufferUsage::None; uint32_t alignment = 0;` CreateBufferDesc input |
| 009-RenderCore | te::rendercore | CreateVertexFormat | free function | Create vertex format | te/rendercore/resource_desc.hpp | `VertexFormat CreateVertexFormat(VertexFormatDesc const& desc);` Returns invalid on failure |
| 009-RenderCore | te::rendercore | CreateIndexFormat | free function | Create index format | te/rendercore/resource_desc.hpp | `IndexFormat CreateIndexFormat(IndexFormatDesc const& desc);` Returns invalid on failure |
| 009-RenderCore | te::rendercore | CreateTextureDesc | free function | Create texture description | te/rendercore/resource_desc.hpp | `TextureDesc CreateTextureDesc(TextureDescParams const& params);` Returns invalid on failure |
| 009-RenderCore | te::rendercore | CreateBufferDesc | free function | Create buffer description | te/rendercore/resource_desc.hpp | `BufferDesc CreateBufferDesc(BufferDescParams const& params);` Returns invalid on failure |

### Uniform Layout (te/rendercore/uniform_layout.hpp)

| Module | Namespace | Symbol | Export Form | Interface Description | Header | Description |
|--------|-----------|--------|-------------|----------------------|--------|-------------|
| 009-RenderCore | te::rendercore | UniformMemberType | enum | Uniform member type | te/rendercore/uniform_layout.hpp | `enum class UniformMemberType : uint8_t { Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Unknown };` |
| 009-RenderCore | te::rendercore | UniformMember | struct | Uniform member | te/rendercore/uniform_layout.hpp | `char name[64] = {}; UniformMemberType type = UniformMemberType::Unknown; uint32_t offset = 0; uint32_t size = 0;` |
| 009-RenderCore | te::rendercore | UniformLayoutDesc | struct | Uniform layout description | te/rendercore/uniform_layout.hpp | `UniformMember const* members = nullptr; uint32_t memberCount = 0; uint32_t totalSize = 0;` totalSize=0 means auto-calculate per std140 |
| 009-RenderCore | te::rendercore | IUniformLayout | abstract interface | Uniform layout | te/rendercore/uniform_layout.hpp | Constant block, consistent with Shader name/type |
| 009-RenderCore | te::rendercore | IUniformLayout::GetOffset | member | Get offset by name | te/rendercore/uniform_layout.hpp | `size_t GetOffset(char const* name) const = 0;` Returns 0 if not found |
| 009-RenderCore | te::rendercore | IUniformLayout::GetTotalSize | member | Get total size | te/rendercore/uniform_layout.hpp | `size_t GetTotalSize() const = 0;` |
| 009-RenderCore | te::rendercore | CreateUniformLayout | free function | Create layout | te/rendercore/uniform_layout.hpp | `IUniformLayout* CreateUniformLayout(UniformLayoutDesc const& desc);` Returns nullptr on failure (stub) |
| 009-RenderCore | te::rendercore | ReleaseUniformLayout | free function | Release layout | te/rendercore/uniform_layout.hpp | `void ReleaseUniformLayout(IUniformLayout* layout);` nullptr is no-op (stub) |

### Shader Reflection (te/rendercore/shader_reflection.hpp)

| Module | Namespace | Symbol | Export Form | Interface Description | Header | Description |
|--------|-----------|--------|-------------|----------------------|--------|-------------|
| 009-RenderCore | te::rendercore | ShaderResourceKind | enum | Resource kind | te/rendercore/shader_reflection.hpp | `enum class ShaderResourceKind : uint8_t { UniformBuffer, SampledImage, Sampler, StorageBuffer, StorageImage, Unknown };` |
| 009-RenderCore | te::rendercore | ShaderResourceBinding | struct | Resource binding | te/rendercore/shader_reflection.hpp | `char name[64] = {}; ShaderResourceKind kind = ShaderResourceKind::Unknown; uint32_t set = 0; uint32_t binding = 0;` Texture/Sampler slot |
| 009-RenderCore | te::rendercore | ShaderReflectionDesc | struct | Full reflection description | te/rendercore/shader_reflection.hpp | `UniformLayoutDesc uniformBlock = {}; ShaderResourceBinding const* resourceBindings = nullptr; uint32_t resourceBindingCount = 0;` 010-Shader GetShaderReflection output |

### Pass Protocol (te/rendercore/pass_protocol.hpp)

| Module | Namespace | Symbol | Export Form | Interface Description | Header | Description |
|--------|-----------|--------|-------------|----------------------|--------|-------------|
| 009-RenderCore | te::rendercore | PassResourceDecl | struct | Pass resource declaration | te/rendercore/pass_protocol.hpp | `PassHandle pass; ResourceHandle resource; bool isRead = false; bool isWrite = false; ResourceLifetime lifetime = ResourceLifetime::Transient; bool IsValid() const;` Interfaces with PipelineCore RDG |
| 009-RenderCore | te::rendercore | DeclareRead | free function | Declare read resource | te/rendercore/pass_protocol.hpp | `void DeclareRead(PassHandle pass, ResourceHandle resource);` (stub) |
| 009-RenderCore | te::rendercore | DeclareWrite | free function | Declare write resource | te/rendercore/pass_protocol.hpp | `void DeclareWrite(PassHandle pass, ResourceHandle resource);` (stub) |
| 009-RenderCore | te::rendercore | SetResourceLifetime | free function | Set lifetime | te/rendercore/pass_protocol.hpp | `void SetResourceLifetime(PassResourceDecl& decl, ResourceLifetime lifetime);` |

### Uniform Buffer (te/rendercore/uniform_buffer.hpp)

| Module | Namespace | Symbol | Export Form | Interface Description | Header | Description |
|--------|-----------|--------|-------------|----------------------|--------|-------------|
| 009-RenderCore | te::rendercore | IUniformBuffer | abstract interface | Uniform buffer | te/rendercore/uniform_buffer.hpp | Layout, update, bind to RHI buffer |
| 009-RenderCore | te::rendercore | IUniformBuffer::Update | member | Update content | te/rendercore/uniform_buffer.hpp | `void Update(void const* data, size_t size) = 0;` Submit to current frame slot |
| 009-RenderCore | te::rendercore | IUniformBuffer::Bind | member | Bind to slot | te/rendercore/uniform_buffer.hpp | `void Bind(te::rhi::ICommandList* cmd, uint32_t slot) = 0;` Calls RHI SetUniformBuffer |
| 009-RenderCore | te::rendercore | IUniformBuffer::GetBuffer | member | Get underlying RHI buffer | te/rendercore/uniform_buffer.hpp | `te::rhi::IBuffer* GetBuffer() = 0;` For 011 UpdateDescriptorSet binding 0 |
| 009-RenderCore | te::rendercore | IUniformBuffer::GetRingBufferOffset | member | Get ring buffer offset | te/rendercore/uniform_buffer.hpp | `size_t GetRingBufferOffset(FrameSlotId slot) const = 0;` |
| 009-RenderCore | te::rendercore | IUniformBuffer::SetCurrentFrameSlot | member | Set current frame slot | te/rendercore/uniform_buffer.hpp | `void SetCurrentFrameSlot(FrameSlotId slot) = 0;` |
| 009-RenderCore | te::rendercore | CreateUniformBuffer | free function | Create buffer | te/rendercore/uniform_buffer.hpp | `IUniformBuffer* CreateUniformBuffer(IUniformLayout const* layout, te::rhi::IDevice* device);` Calls RHI CreateBuffer(Uniform); returns nullptr on failure (stub) |
| 009-RenderCore | te::rendercore | ReleaseUniformBuffer | free function | Release buffer | te/rendercore/uniform_buffer.hpp | `void ReleaseUniformBuffer(IUniformBuffer* buffer);` Calls RHI DestroyBuffer; nullptr is no-op (stub) |

### Render Element (te/rendercore/IRenderElement.hpp, te/rendercore/RenderElement.hpp)

| Module | Namespace | Symbol | Export Form | Interface Description | Header | Description |
|--------|-----------|--------|-------------|----------------------|--------|-------------|
| 009-RenderCore | te::rendercore | IRenderElement | abstract interface | Render element | te/rendercore/IRenderElement.hpp | Aggregates mesh and material for one draw |
| 009-RenderCore | te::rendercore | IRenderElement::GetMesh | member | Get mesh | te/rendercore/IRenderElement.hpp | `IRenderMesh* GetMesh() = 0; IRenderMesh const* GetMesh() const = 0;` |
| 009-RenderCore | te::rendercore | IRenderElement::GetMaterial | member | Get material | te/rendercore/IRenderElement.hpp | `IRenderMaterial* GetMaterial() = 0; IRenderMaterial const* GetMaterial() const = 0;` |
| 009-RenderCore | te::rendercore | SimpleRenderElement | class | Simple render element | te/rendercore/RenderElement.hpp | Holds pointers to mesh and material; caller ensures validity |
| 009-RenderCore | te::rendercore | SimpleRenderElement::SetMesh | member | Set mesh | te/rendercore/RenderElement.hpp | `void SetMesh(IRenderMesh* mesh);` |
| 009-RenderCore | te::rendercore | SimpleRenderElement::SetMaterial | member | Set material | te/rendercore/RenderElement.hpp | `void SetMaterial(IRenderMaterial* material);` |
| 009-RenderCore | te::rendercore | OwningRenderElement | class | Owning render element | te/rendercore/RenderElement.hpp | Owns mesh and material via unique_ptr |
| 009-RenderCore | te::rendercore | OwningRenderElement::SetMesh | member | Set mesh (owning) | te/rendercore/RenderElement.hpp | `void SetMesh(std::unique_ptr<IRenderMesh> mesh);` |
| 009-RenderCore | te::rendercore | OwningRenderElement::SetMaterial | member | Set material (owning) | te/rendercore/RenderElement.hpp | `void SetMaterial(std::unique_ptr<IRenderMaterial> material);` |
| 009-RenderCore | te::rendercore | OwningRenderElement::TakeMesh | member | Take mesh ownership | te/rendercore/RenderElement.hpp | `void TakeMesh(IRenderMesh* mesh);` |
| 009-RenderCore | te::rendercore | OwningRenderElement::TakeMaterial | member | Take material ownership | te/rendercore/RenderElement.hpp | `void TakeMaterial(IRenderMaterial* material);` |
| 009-RenderCore | te::rendercore | CreateRenderElement | free function | Create render element | te/rendercore/RenderElement.hpp | `IRenderElement* CreateRenderElement(IRenderMesh* mesh, IRenderMaterial* material);` Returns new SimpleRenderElement |
| 009-RenderCore | te::rendercore | DestroyRenderElement | free function | Destroy render element | te/rendercore/RenderElement.hpp | `void DestroyRenderElement(IRenderElement* element);` |

### Render Mesh (te/rendercore/IRenderMesh.hpp)

| Module | Namespace | Symbol | Export Form | Interface Description | Header | Description |
|--------|-----------|--------|-------------|----------------------|--------|-------------|
| 009-RenderCore | te::rendercore | SubmeshRange | struct | Submesh range | te/rendercore/IRenderMesh.hpp | `uint32_t indexOffset = 0; uint32_t indexCount = 0; uint32_t vertexOffset = 0;` For draw |
| 009-RenderCore | te::rendercore | IRenderMesh | abstract interface | GPU mesh | te/rendercore/IRenderMesh.hpp | Vertex/index buffers; supports submeshes |
| 009-RenderCore | te::rendercore | IRenderMesh::GetVertexBuffer | member | Get vertex buffer | te/rendercore/IRenderMesh.hpp | `rhi::IBuffer* GetVertexBuffer() = 0; rhi::IBuffer const* GetVertexBuffer() const = 0;` |
| 009-RenderCore | te::rendercore | IRenderMesh::GetIndexBuffer | member | Get index buffer | te/rendercore/IRenderMesh.hpp | `rhi::IBuffer* GetIndexBuffer() = 0; rhi::IBuffer const* GetIndexBuffer() const = 0;` |
| 009-RenderCore | te::rendercore | IRenderMesh::GetSubmeshCount | member | Get submesh count | te/rendercore/IRenderMesh.hpp | `uint32_t GetSubmeshCount() const = 0;` |
| 009-RenderCore | te::rendercore | IRenderMesh::GetSubmesh | member | Get submesh | te/rendercore/IRenderMesh.hpp | `bool GetSubmesh(uint32_t index, SubmeshRange* out) const = 0;` |
| 009-RenderCore | te::rendercore | IRenderMesh::SetDataVertex | member | Set vertex data | te/rendercore/IRenderMesh.hpp | `void SetDataVertex(void const* data, size_t size) = 0;` |
| 009-RenderCore | te::rendercore | IRenderMesh::SetDataIndex | member | Set index data | te/rendercore/IRenderMesh.hpp | `void SetDataIndex(void const* data, size_t size) = 0;` |
| 009-RenderCore | te::rendercore | IRenderMesh::SetDataIndexType | member | Set index type | te/rendercore/IRenderMesh.hpp | `void SetDataIndexType(IndexType type) = 0;` |
| 009-RenderCore | te::rendercore | IRenderMesh::SetDataSubmeshCount | member | Set submesh count | te/rendercore/IRenderMesh.hpp | `void SetDataSubmeshCount(uint32_t count) = 0;` |
| 009-RenderCore | te::rendercore | IRenderMesh::SetDataSubmesh | member | Set submesh | te/rendercore/IRenderMesh.hpp | `void SetDataSubmesh(uint32_t index, SubmeshRange const& range) = 0;` |
| 009-RenderCore | te::rendercore | IRenderMesh::UpdateDeviceResource | member | Upload to GPU | te/rendercore/IRenderMesh.hpp | `void UpdateDeviceResource(rhi::IDevice* device) = 0;` |

### Render Material (te/rendercore/IRenderMaterial.hpp)

| Module | Namespace | Symbol | Export Form | Interface Description | Header | Description |
|--------|-----------|--------|-------------|----------------------|--------|-------------|
| 009-RenderCore | te::rendercore | IRenderMaterial | abstract interface | Render material | te/rendercore/IRenderMaterial.hpp | IShadingState + uniform buffer + descriptor set + PSO |
| 009-RenderCore | te::rendercore | IRenderMaterial::GetUniformBuffer | member | Get uniform buffer | te/rendercore/IRenderMaterial.hpp | `IUniformBuffer* GetUniformBuffer() = 0; IUniformBuffer const* GetUniformBuffer() const = 0;` |
| 009-RenderCore | te::rendercore | IRenderMaterial::GetDescriptorSet | member | Get descriptor set | te/rendercore/IRenderMaterial.hpp | `rhi::IDescriptorSet* GetDescriptorSet() = 0; rhi::IDescriptorSet const* GetDescriptorSet() const = 0;` |
| 009-RenderCore | te::rendercore | IRenderMaterial::GetGraphicsPSO | member | Get graphics PSO | te/rendercore/IRenderMaterial.hpp | `rhi::IPSO* GetGraphicsPSO(uint32_t subpassIndex = 0) = 0; rhi::IPSO const* GetGraphicsPSO(uint32_t subpassIndex = 0) const = 0;` |
| 009-RenderCore | te::rendercore | IRenderMaterial::CreateDeviceResource | member | Create GPU resources (basic) | te/rendercore/IRenderMaterial.hpp | `void CreateDeviceResource() = 0;` Creates PSO, UB, descriptor set |
| 009-RenderCore | te::rendercore | IRenderMaterial::CreateDeviceResource | member (overload) | Create GPU resources (with pass) | te/rendercore/IRenderMaterial.hpp | `void CreateDeviceResource(rhi::IRenderPass* renderPass, uint32_t subpassCount, rhi::IDescriptorSetLayout* skinLayout = nullptr) = 0;` Optional renderPass for subpass-specific PSO |
| 009-RenderCore | te::rendercore | IRenderMaterial::UpdateDeviceResource | member | Upload CPU data to GPU | te/rendercore/IRenderMaterial.hpp | `void UpdateDeviceResource(rhi::IDevice* device, uint32_t frameSlot) = 0;` Call after SetData* before draw |
| 009-RenderCore | te::rendercore | IRenderMaterial::SetDataParameter | member | Set uniform parameter | te/rendercore/IRenderMaterial.hpp | `void SetDataParameter(char const* name, void const* data, size_t size) = 0;` By name (std140) |
| 009-RenderCore | te::rendercore | IRenderMaterial::SetDataTexture | member | Set texture at binding | te/rendercore/IRenderMaterial.hpp | `void SetDataTexture(uint32_t binding, rhi::ITexture* texture) = 0;` RHI texture pointer |
| 009-RenderCore | te::rendercore | IRenderMaterial::SetDataTextureByName | member | Set texture by name | te/rendercore/IRenderMaterial.hpp | `void SetDataTextureByName(char const* name, rhi::ITexture* texture) = 0;` |
| 009-RenderCore | te::rendercore | IRenderMaterial::IsDeviceReady | member | Check device ready | te/rendercore/IRenderMaterial.hpp | `bool IsDeviceReady() const = 0;` |

### Render Pipeline State (te/rendercore/IRenderPipelineState.hpp)

| Module | Namespace | Symbol | Export Form | Interface Description | Header | Description |
|--------|-----------|--------|-------------|----------------------|--------|-------------|
| 009-RenderCore | te::rendercore | IRenderPipelineState | abstract interface | Render pipeline state | te/rendercore/IRenderPipelineState.hpp | Render state (blend, depth, raster); maps directly to RHI for PSO creation |
| 009-RenderCore | te::rendercore | IRenderPipelineState::GetRHIStateDesc | member | Get RHI state desc | te/rendercore/IRenderPipelineState.hpp | `rhi::GraphicsPipelineStateDesc const* GetRHIStateDesc() const = 0;` Must not be null |

### Render Texture (te/rendercore/IRenderTexture.hpp)

| Module | Namespace | Symbol | Export Form | Interface Description | Header | Description |
|--------|-----------|--------|-------------|----------------------|--------|-------------|
| 009-RenderCore | te::rendercore | IRenderTexture | abstract interface | GPU texture | te/rendercore/IRenderTexture.hpp | GPU-side texture: sampled image or render target attachment; no 013/028 refs |
| 009-RenderCore | te::rendercore | IRenderTexture::GetRHITexture | member | Get RHI texture | te/rendercore/IRenderTexture.hpp | `rhi::ITexture* GetRHITexture() const = 0;` For descriptor set, bind, destroy |
| 009-RenderCore | te::rendercore | IRenderTexture::GetUsage | member | Get usage | te/rendercore/IRenderTexture.hpp | `TextureUsage GetUsage() const = 0;` Sampled, RenderTarget, DepthStencil, etc. |
| 009-RenderCore | te::rendercore | IRenderTexture::IsAttachment | member | Is attachment | te/rendercore/IRenderTexture.hpp | `inline bool IsAttachment() const;` True if RenderTarget or DepthStencil |

### Shader Entry (te/rendercore/IShaderEntry.hpp)

| Module | Namespace | Symbol | Export Form | Interface Description | Header | Description |
|--------|-----------|--------|-------------|----------------------|--------|-------------|
| 009-RenderCore | te::rendercore | IShaderEntry | abstract interface | Shader entry | te/rendercore/IShaderEntry.hpp | Bytecode and reflection per stage; caller guarantees bytecode format matches device backend |
| 009-RenderCore | te::rendercore | IShaderEntry::GetVertexBytecode | member | Get vertex bytecode | te/rendercore/IShaderEntry.hpp | `void const* GetVertexBytecode() const = 0;` |
| 009-RenderCore | te::rendercore | IShaderEntry::GetVertexBytecodeSize | member | Get vertex bytecode size | te/rendercore/IShaderEntry.hpp | `size_t GetVertexBytecodeSize() const = 0;` |
| 009-RenderCore | te::rendercore | IShaderEntry::GetFragmentBytecode | member | Get fragment bytecode | te/rendercore/IShaderEntry.hpp | `void const* GetFragmentBytecode() const = 0;` |
| 009-RenderCore | te::rendercore | IShaderEntry::GetFragmentBytecodeSize | member | Get fragment bytecode size | te/rendercore/IShaderEntry.hpp | `size_t GetFragmentBytecodeSize() const = 0;` |
| 009-RenderCore | te::rendercore | IShaderEntry::GetVertexInput | member | Get vertex input | te/rendercore/IShaderEntry.hpp | `VertexFormatDesc const* GetVertexInput() const = 0;` |
| 009-RenderCore | te::rendercore | IShaderEntry::GetVertexReflection | member | Get vertex reflection | te/rendercore/IShaderEntry.hpp | `ShaderReflectionDesc const* GetVertexReflection() const = 0;` |
| 009-RenderCore | te::rendercore | IShaderEntry::GetFragmentReflection | member | Get fragment reflection | te/rendercore/IShaderEntry.hpp | `ShaderReflectionDesc const* GetFragmentReflection() const = 0;` |

### Shading State (te/rendercore/IShadingState.hpp)

| Module | Namespace | Symbol | Export Form | Interface Description | Header | Description |
|--------|-----------|--------|-------------|----------------------|--------|-------------|
| 009-RenderCore | te::rendercore | IShadingState | abstract interface | Shading state | te/rendercore/IShadingState.hpp | Pipeline state plus shader entry; used to create PSO; extends IRenderPipelineState |
| 009-RenderCore | te::rendercore | IShadingState::GetPipelineState | member | Get pipeline state | te/rendercore/IShadingState.hpp | `IRenderPipelineState const* GetPipelineState() const = 0;` |
| 009-RenderCore | te::rendercore | IShadingState::GetShaderEntry | member | Get shader entry | te/rendercore/IShadingState.hpp | `IShaderEntry const* GetShaderEntry() const = 0;` |

### Header Files and Include Relationships

| Header | Dependencies | Description |
|--------|--------------|-------------|
| te/rendercore/types.hpp | <cstdint>, <cstddef> | ResultCode, PassHandle, ResourceHandle, FrameSlotId, ResourceLifetime, BindSlot |
| te/rendercore/resource_desc.hpp | te/rendercore/types.hpp, <cstdint> | VertexFormat, IndexFormat, TextureDesc, BufferDesc, Create* functions, enums |
| te/rendercore/uniform_layout.hpp | te/rendercore/types.hpp, <cstddef>, <cstdint> | IUniformLayout, UniformLayoutDesc, CreateUniformLayout, ReleaseUniformLayout |
| te/rendercore/shader_reflection.hpp | te/rendercore/uniform_layout.hpp, te/rendercore/types.hpp, <cstddef>, <cstdint> | ShaderResourceKind, ShaderResourceBinding, ShaderReflectionDesc |
| te/rendercore/pass_protocol.hpp | te/rendercore/types.hpp | PassResourceDecl, DeclareRead, DeclareWrite, SetResourceLifetime |
| te/rendercore/uniform_buffer.hpp | te/rendercore/types.hpp, te/rendercore/uniform_layout.hpp, te/rhi/device.hpp (fwd), te/rhi/command_list.hpp (fwd), te/rhi/resources.hpp (fwd) | IUniformBuffer, CreateUniformBuffer, ReleaseUniformBuffer |
| te/rendercore/IRenderPipelineState.hpp | te/rhi/pso.hpp (fwd) | IRenderPipelineState, GetRHIStateDesc |
| te/rendercore/IShaderEntry.hpp | te/rendercore/resource_desc.hpp, te/rendercore/shader_reflection.hpp, <cstddef> | IShaderEntry |
| te/rendercore/IShadingState.hpp | te/rendercore/IRenderPipelineState.hpp, te/rendercore/IShaderEntry.hpp (fwd) | IShadingState |
| te/rendercore/IRenderMaterial.hpp | te/rendercore/IShadingState.hpp, <cstdint>, te/rhi/device.hpp (fwd), te/rhi/renderpass.hpp (fwd), etc. | IRenderMaterial |
| te/rendercore/IRenderMesh.hpp | te/rendercore/resource_desc.hpp, <cstddef>, <cstdint>, te/rhi/device.hpp (fwd), te/rhi/resources.hpp (fwd) | IRenderMesh, SubmeshRange |
| te/rendercore/IRenderElement.hpp | te/rendercore/IRenderMesh.hpp (fwd), te/rendercore/IRenderMaterial.hpp (fwd) | IRenderElement |
| te/rendercore/IRenderTexture.hpp | te/rendercore/resource_desc.hpp, te/rhi/resources.hpp (fwd) | IRenderTexture |
| te/rendercore/RenderElement.hpp | te/rendercore/IRenderElement.hpp, te/rendercore/IRenderMesh.hpp, te/rendercore/IRenderMaterial.hpp, <memory> | SimpleRenderElement, OwningRenderElement, CreateRenderElement, DestroyRenderElement |
| te/rendercore/api.hpp | all above | Aggregate header; downstream only needs `#include <te/rendercore/api.hpp>` |

---

*Source: Contract capabilities ResourceDesc, UniformLayout, PassProtocol, UniformBuffer, RenderElement, RenderMesh, RenderMaterial, RenderPipelineState, RenderTexture, ShaderEntry, ShadingState; interfaces with 010-Shader, 019-PipelineCore, 008-RHI.*
*Namespace and header path consistent with 008-RHI (te::rhi, te/rhi/) and 001-Core (te::core, te/core/).*

## Change Log

| Date | Change Description |
|------|-------------------|
| 2026-02-10 | IUniformBuffer::GetBuffer() to get underlying RHI buffer for 011 descriptor set write |
| 2026-02-22 | Code-aligned update: added IRenderElement (SimpleRenderElement, OwningRenderElement, CreateRenderElement, DestroyRenderElement), IRenderMesh (SubmeshRange, SetData* methods, UpdateDeviceResource), IRenderMaterial (CreateDeviceResource overloads, SetDataParameter, SetDataTexture, SetDataTextureByName, GetUniformBuffer, GetDescriptorSet, GetGraphicsPSO, IsDeviceReady), IRenderPipelineState, IRenderTexture, IShaderEntry, IShadingState; extended resource_desc.hpp (VertexFormatDesc, IndexFormatDesc, TextureDescParams, BufferDescParams, Create* functions with validation); added shader_reflection.hpp details; added api.hpp aggregate header |
