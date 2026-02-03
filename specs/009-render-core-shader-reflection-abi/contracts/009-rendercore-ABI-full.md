# 009-RenderCore 全量 ABI（实现参考）

> **用途**：本 feature 实现时参考的全量 ABI 内容。权威契约见 `specs/_contracts/009-rendercore-ABI.md`。
> **包含**：原始 ABI 全部条目 + TODO 实现要求。实现须基于本全量内容，不得仅实现变化部分。

## 元信息

- **CMake Target**：`te_rendercore`
- **命名空间**：`te::rendercore`
- **头文件路径**：`te/rendercore/`
- **依赖**：`te_rhi`（008-RHI）、`te_core`（001-Core，经 te_rhi 传递）

## 类型与句柄（te/rendercore/types.hpp）

| 符号 | 导出形式 | 说明 |
|------|----------|------|
| ResultCode | 枚举 | `enum class ResultCode : uint32_t { Success, InvalidHandle, UnsupportedFormat, UnsupportedSize, ValidationFailed, RingBufferExhausted, Unknown };` |
| PassHandle | struct | `struct PassHandle { uint64_t id; bool IsValid() const; };` |
| ResourceHandle | struct | `struct ResourceHandle { uint64_t id; bool IsValid() const; };` |
| FrameSlotId | 类型别名 | `using FrameSlotId = uint32_t;` |
| ResourceLifetime | 枚举 | `enum class ResourceLifetime : uint8_t { Transient, Persistent, External };` |
| BindSlot | struct | `struct BindSlot { uint32_t set; uint32_t binding; };` |

## 资源描述（te/rendercore/resource_desc.hpp）

| 符号 | 导出形式 | 说明 |
|------|----------|------|
| VertexAttributeFormat | 枚举 | Float, Float2, Float3, Float4, Int, Int2, Int3, Int4, UInt, UInt2, UInt3, UInt4, Unknown |
| VertexAttribute | struct | `uint32_t location; VertexAttributeFormat format; uint32_t offset;` |
| VertexFormat | struct | attributes[], attributeCount, stride |
| VertexFormatDesc | struct | CreateVertexFormat 输入 |
| IndexType | 枚举 | UInt16, UInt32, Unknown |
| IndexFormat | struct | `IndexType type;` |
| IndexFormatDesc | struct | CreateIndexFormat 输入 |
| TextureFormat | 枚举 | R8_UNorm, RGBA8_UNorm, RGBA8_SRGB, Depth16, Depth24_Stencil8, ... |
| TextureUsage | 枚举 | Sampled, Storage, RenderTarget, DepthStencil, TransferSrc, TransferDst（位掩码） |
| TextureDesc | struct | width, height, depth, mipLevels, format, usage |
| TextureDescParams | struct | CreateTextureDesc 输入 |
| BufferUsage | 枚举 | Vertex, Index, Uniform, Storage, TransferSrc, TransferDst（位掩码） |
| BufferDesc | struct | size, usage, alignment |
| BufferDescParams | struct | CreateBufferDesc 输入 |
| CreateVertexFormat | 自由函数 | `VertexFormat CreateVertexFormat(VertexFormatDesc const& desc);` |
| CreateIndexFormat | 自由函数 | `IndexFormat CreateIndexFormat(IndexFormatDesc const& desc);` |
| CreateTextureDesc | 自由函数 | `TextureDesc CreateTextureDesc(TextureDescParams const& params);` |
| CreateBufferDesc | 自由函数 | `BufferDesc CreateBufferDesc(BufferDescParams const& params);` |

## Uniform 布局（te/rendercore/uniform_layout.hpp）

| 符号 | 导出形式 | 说明 |
|------|----------|------|
| UniformMemberType | 枚举 | Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Unknown |
| UniformMember | struct | `char name[64]; UniformMemberType type; uint32_t offset; uint32_t size;` |
| UniformLayoutDesc | struct | `UniformMember const* members; uint32_t memberCount; uint32_t totalSize;` |
| IUniformLayout | 抽象接口 | GetOffset, GetTotalSize |
| CreateUniformLayout | 自由函数 | `IUniformLayout* CreateUniformLayout(UniformLayoutDesc const& desc);` |
| ReleaseUniformLayout | 自由函数 | `void ReleaseUniformLayout(IUniformLayout* layout);` |

## Pass 协议（te/rendercore/pass_protocol.hpp）

| 符号 | 导出形式 | 说明 |
|------|----------|------|
| PassResourceDecl | struct | pass, resource, isRead, isWrite, lifetime |
| DeclareRead | 自由函数 | `void DeclareRead(PassHandle pass, ResourceHandle resource);` |
| DeclareWrite | 自由函数 | `void DeclareWrite(PassHandle pass, ResourceHandle resource);` |
| SetResourceLifetime | 自由函数 | `void SetResourceLifetime(PassResourceDecl& decl, ResourceLifetime lifetime);` |

## Uniform 缓冲（te/rendercore/uniform_buffer.hpp）

| 符号 | 导出形式 | 说明 |
|------|----------|------|
| IUniformBuffer | 抽象接口 | Update, Bind, GetRingBufferOffset, SetCurrentFrameSlot |
| CreateUniformBuffer | 自由函数 | `IUniformBuffer* CreateUniformBuffer(IUniformLayout const* layout, te::rhi::IDevice* device);` |
| ReleaseUniformBuffer | 自由函数 | `void ReleaseUniformBuffer(IUniformBuffer* buffer);` |

## 聚合头

| 头文件 | 依赖 |
|--------|------|
| te/rendercore/api.hpp | 以上所有 |

## TODO 实现要求（ABI 中待完成项）

- [ ] 实现 te/rendercore/uniform_layout.hpp 及 CreateUniformLayout/ReleaseUniformLayout
- [ ] 与 010-Shader GetReflection 产出的 UniformLayoutDesc 格式对齐（成员类型映射、偏移、命名）
