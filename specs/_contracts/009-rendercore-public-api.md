# Contract: 009-RenderCore Module Public API

## Applicable Modules

- **Implementer**: 009-RenderCore (L2; Shader parameter structures, render resource descriptions, Pass parameter protocol, Uniform Buffer; between RHI and pipeline)
- **Corresponding Spec**: `docs/module-specs/009-render-core.md`
- **Dependencies**: 001-Core, 008-RHI

## Consumers

- 010-Shader, 011-Material, 012-Mesh, 019-PipelineCore, 020-Pipeline, 021-Effects, 022-2D, 023-Terrain, 028-Texture

## Capability List

### Types and Handles (Cross-Boundary)

| Name | Semantics | Lifetime |
|------|-----------|----------|
| ResultCode | Result code enumeration (Success, InvalidHandle, UnsupportedFormat, UnsupportedSize, ValidationFailed, RingBufferExhausted, Unknown) | Enum type |
| PassHandle | Pass identifier/handle, used for DeclareRead/DeclareWrite | Single Pass graph construction cycle or managed by PipelineCore |
| ResourceHandle | Resource handle, used to declare Pass read/write resources | Consistent with resource lifetime |
| FrameSlotId | Frame slot ID type alias (`using FrameSlotId = uint32_t;`) | Per-frame |
| ResourceLifetime | Resource lifetime enumeration (Transient, Persistent, External) | Enum type |
| BindSlot | Bind slot struct (set, binding) | Consistent with Shader/RHI slot convention |
| UniformLayout | Uniform Buffer layout, constant block, consistent with Shader name/type | Defined until unloaded |
| IUniformLayout | Uniform layout interface; GetOffset(name), GetTotalSize() | Created until explicit release |
| UniformMemberType | Uniform member type enumeration (Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Unknown) | Enum type |
| UniformMember | Uniform member struct (name[64], type, offset, size) | Defined until unloaded |
| UniformLayoutDesc | Uniform layout description (members, memberCount, totalSize) | Used for layout creation |
| VertexFormat / IndexFormat | Vertex/index format description, interfaces with RHI creation parameters | Defined until unloaded |
| VertexAttributeFormat | Vertex attribute format enumeration (Float, Float2, Float3, Float4, Int, Int2, Int3, Int4, UInt, UInt2, UInt3, UInt4, Unknown) | Enum type |
| VertexAttribute | Vertex attribute struct (location, format, offset) | Defined until unloaded |
| VertexFormatDesc | Vertex format description for CreateVertexFormat | Used for format creation |
| IndexType | Index type enumeration (UInt16, UInt32, Unknown) | Enum type |
| IndexFormatDesc | Index format description for CreateIndexFormat | Used for format creation |
| TextureFormat | Texture format enumeration (R8_UNorm, RGBA8_UNorm, RGBA8_SRGB, Depth16, Depth24_Stencil8, etc.) | Enum type |
| TextureUsage | Texture usage bitmask (Sampled, Storage, RenderTarget, DepthStencil, TransferSrc, TransferDst) | Bitmask type |
| TextureDesc / TextureDescParams | Texture description, bridges with RHI resource creation | Managed by caller |
| BufferUsage | Buffer usage bitmask (Vertex, Index, Uniform, Storage, TransferSrc, TransferDst) | Bitmask type |
| BufferDesc / BufferDescParams | Buffer description, bridges with RHI resource creation | Managed by caller |
| PassResourceDecl | Pass input/output resource declaration, interfaces with PipelineCore RDG | Single Pass graph construction cycle |
| IUniformBuffer | Uniform buffer handle; layout, update, multi-frame ring buffer, bind to RHI | Created until explicit release |
| ShaderReflectionDesc | Full shader reflection: Uniform block + resource bindings (Texture, Sampler) | Bound to Shader or cache |
| ShaderResourceKind | Shader resource kind enumeration (UniformBuffer, SampledImage, Sampler, StorageBuffer, StorageImage, Unknown) | Enum type |
| ShaderResourceBinding | Single resource binding struct (name[64], kind, set, binding) | Defined until unloaded |
| IRenderElement | Single draw resource set = IRenderMesh + IRenderMaterial | Managed by caller |
| IRenderMesh | GPU mesh interface; vertex/index buffers, submeshes | Managed by caller |
| IRenderMaterial | Render material interface; IShadingState + uniform buffer + bindings | Managed by caller |
| IRenderPipelineState | Render state interface; maps directly to RHI | Managed by caller |
| IRenderTexture | GPU texture abstraction (sampled or attachment) | Managed by caller |
| IShaderEntry | Shader entry interface; bytecode and reflection for PSO creation | Managed by caller |
| IShadingState | Shading state interface; IRenderPipelineState + IShaderEntry | Managed by caller |
| SubmeshRange | Submesh range struct for draw (indexOffset, indexCount, vertexOffset) | Per-submesh |

Does not directly expose RHI resources; RenderCore bridges to RHI.

### Capabilities (Provider Guarantees)

| # | Capability | Description |
|---|------------|-------------|
| 1 | Types | ResultCode, PassHandle, ResourceHandle, FrameSlotId, ResourceLifetime, BindSlot; basic types for render core operations |
| 2 | ResourceDesc | VertexFormat, IndexFormat, TextureDesc, BufferDesc, VertexAttribute, VertexAttributeFormat, IndexType, TextureFormat, TextureUsage, BufferUsage; CreateVertexFormat, CreateIndexFormat, CreateTextureDesc, CreateBufferDesc; interfaces with RHI creation parameters |
| 3 | UniformLayout | UniformMemberType, UniformMember, UniformLayoutDesc, IUniformLayout; CreateUniformLayout, ReleaseUniformLayout; GetOffset, GetTotalSize; Uniform layout consistent with Shader reflection or hand-written layout |
| 4 | ShaderReflection | ShaderResourceKind, ShaderResourceBinding, ShaderReflectionDesc; full shader reflection: Uniform block + Texture + Sampler bindings |
| 5 | PassProtocol | PassResourceDecl, DeclareRead, DeclareWrite, SetResourceLifetime; interfaces with PipelineCore RDG protocol |
| 6 | UniformBuffer | IUniformBuffer; CreateUniformBuffer, ReleaseUniformBuffer; Update, Bind, GetBuffer (for descriptor set write), GetRingBufferOffset, SetCurrentFrameSlot; interfaces with Shader and RHI buffer binding |
| 7 | RenderElement | IRenderElement, SimpleRenderElement, OwningRenderElement; CreateRenderElement, DestroyRenderElement; aggregates mesh and material for one draw |
| 8 | RenderMesh | IRenderMesh, SubmeshRange; vertex/index buffers, submesh support; SetDataVertex, SetDataIndex, SetDataIndexType, SetDataSubmeshCount, SetDataSubmesh, UpdateDeviceResource |
| 9 | RenderMaterial | IRenderMaterial; IShadingState + uniform buffer + descriptor set + PSO; GetUniformBuffer, GetDescriptorSet, GetGraphicsPSO; SetDataParameter, SetDataTexture, SetDataTextureByName; CreateDeviceResource, UpdateDeviceResource, IsDeviceReady |
| 10 | RenderPipelineState | IRenderPipelineState; GetRHIStateDesc(); render state (blend, depth, raster) maps directly to RHI |
| 11 | RenderTexture | IRenderTexture; GetRHITexture, GetUsage, IsAttachment; GPU-side texture (sampled or render target attachment) |
| 12 | ShaderEntry | IShaderEntry; GetVertexBytecode, GetFragmentBytecode, GetVertexInput, GetVertexReflection, GetFragmentReflection; bytecode and reflection per stage |
| 13 | ShadingState | IShadingState; IRenderPipelineState + IShaderEntry; GetPipelineState, GetShaderEntry; used to create PSO |

Namespace `te::rendercore`.

## Version / ABI

- Follows Constitution: Public API versioned; breaking changes increment MAJOR.

## Constraints

- Must be used after Core and RHI initialization; Uniform layout convention with Shader must be consistent. Pass resource declaration must be consistent with PipelineCore (019) Pass graph protocol.

## Change Log

| Date | Change Description |
|------|-------------------|
| T0 Initial | 009-RenderCore contract |
| 2026-02-05 | Unified directory; capability list in table format |
| 2026-02-10 | Capability 6: IUniformBuffer::GetBuffer() for 011 descriptor set write |
| 2026-02-22 | Code-aligned update: added IRenderElement (SimpleRenderElement, OwningRenderElement), IRenderMesh (SubmeshRange, SetData* methods, UpdateDeviceResource), IRenderMaterial (CreateDeviceResource overloads, SetDataParameter, SetDataTexture, SetDataTextureByName), IRenderPipelineState, IRenderTexture, IShaderEntry, IShadingState; extended resource_desc.hpp (VertexFormatDesc, IndexFormatDesc, TextureDescParams, BufferDescParams, Create* functions); added shader_reflection.hpp details; added api.hpp aggregate header |
