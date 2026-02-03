# 009-RenderCore 模块 ABI

- **契约**：[009-rendercore-public-api.md](./009-rendercore-public-api.md)（能力与类型描述）
- **本文件**：009-RenderCore 对外 ABI 显式表。
- **CMake Target 名称**：**`te_rendercore`**（与 `te_rhi`、`te_core` 命名风格一致）。下游在 `target_link_libraries` 中应使用 **`te_rendercore`**。依赖上游 target: **`te_rhi`** (008-RHI) 和 **`te_core`** (001-Core，经 te_rhi 传递)。
- **命名空间**：**`te::rendercore`**（与 `te::rhi`、`te::core` 风格一致）。实现文件统一使用此命名空间，不再使用 `TenEngine::RenderCore` 或 `TenEngine::rendercore`。
- **头文件路径**：**`te/rendercore/`**（与 `te/rhi/`、`te/core/` 风格一致）。
- **角色**：渲染类型与 Pass 协议，介于 RHI 与管线之间；不直接持有 GPU 资源，与 RHI 桥接。

## ABI 表

列定义：**模块名 | 命名空间 | 符号/类型 | 导出形式 | 接口说明 | 头文件 | 说明**

### 类型与句柄（te/rendercore/types.hpp）

| 模块名 | 命名空间 | 符号 | 导出形式 | 接口说明 | 头文件 | 说明 |
|--------|----------|------|----------|----------|--------|------|
| 009-RenderCore | te::rendercore | ResultCode | 枚举 | 结果码 | te/rendercore/types.hpp | `enum class ResultCode : uint32_t { Success, InvalidHandle, UnsupportedFormat, UnsupportedSize, ValidationFailed, RingBufferExhausted, Unknown };` |
| 009-RenderCore | te::rendercore | PassHandle | struct | Pass 句柄 | te/rendercore/types.hpp | `struct PassHandle { uint64_t id; bool IsValid() const; };` 不透明句柄；用于 DeclareRead/DeclareWrite |
| 009-RenderCore | te::rendercore | ResourceHandle | struct | 资源句柄 | te/rendercore/types.hpp | `struct ResourceHandle { uint64_t id; bool IsValid() const; };` 管线内资源句柄；与 PipelineCore 资源生命周期一致 |
| 009-RenderCore | te::rendercore | FrameSlotId | 类型别名 | 帧 slot | te/rendercore/types.hpp | `using FrameSlotId = uint32_t;` 用于 RingBuffer 多帧 slot |
| 009-RenderCore | te::rendercore | ResourceLifetime | 枚举 | 资源生命周期 | te/rendercore/types.hpp | `enum class ResourceLifetime : uint8_t { Transient, Persistent, External };` 与 PipelineCore RDG 对接 |
| 009-RenderCore | te::rendercore | BindSlot | struct | 绑定槽位 | te/rendercore/types.hpp | `struct BindSlot { uint32_t set; uint32_t binding; };` 与 Shader/RHI 槽位约定一致 |

### 资源描述（te/rendercore/resource_desc.hpp）

| 模块名 | 命名空间 | 符号 | 导出形式 | 接口说明 | 头文件 | 说明 |
|--------|----------|------|----------|----------|--------|------|
| 009-RenderCore | te::rendercore | VertexAttributeFormat | 枚举 | 顶点属性格式 | te/rendercore/resource_desc.hpp | Float, Float2, Float3, Float4, Int, Int2, Int3, Int4, UInt, UInt2, UInt3, UInt4, Unknown |
| 009-RenderCore | te::rendercore | VertexAttribute | struct | 顶点属性 | te/rendercore/resource_desc.hpp | `uint32_t location; VertexAttributeFormat format; uint32_t offset;` |
| 009-RenderCore | te::rendercore | VertexFormat | struct | 顶点格式 | te/rendercore/resource_desc.hpp | attributes[], attributeCount, stride；与 RHI 创建参数对接 |
| 009-RenderCore | te::rendercore | VertexFormatDesc | struct | 顶点格式描述 | te/rendercore/resource_desc.hpp | CreateVertexFormat 输入参数 |
| 009-RenderCore | te::rendercore | IndexType | 枚举 | 索引类型 | te/rendercore/resource_desc.hpp | UInt16, UInt32, Unknown |
| 009-RenderCore | te::rendercore | IndexFormat | struct | 索引格式 | te/rendercore/resource_desc.hpp | `IndexType type;` 与 RHI 对接 |
| 009-RenderCore | te::rendercore | IndexFormatDesc | struct | 索引格式描述 | te/rendercore/resource_desc.hpp | CreateIndexFormat 输入参数 |
| 009-RenderCore | te::rendercore | TextureFormat | 枚举 | 纹理格式 | te/rendercore/resource_desc.hpp | R8_UNorm, RGBA8_UNorm, RGBA8_SRGB, Depth16, Depth24_Stencil8, ... |
| 009-RenderCore | te::rendercore | TextureUsage | 枚举 | 纹理用途 | te/rendercore/resource_desc.hpp | Sampled, Storage, RenderTarget, DepthStencil, TransferSrc, TransferDst（位掩码） |
| 009-RenderCore | te::rendercore | TextureDesc | struct | 纹理描述 | te/rendercore/resource_desc.hpp | width, height, depth, mipLevels, format, usage；与 RHI 资源创建桥接 |
| 009-RenderCore | te::rendercore | TextureDescParams | struct | 纹理描述参数 | te/rendercore/resource_desc.hpp | CreateTextureDesc 输入参数 |
| 009-RenderCore | te::rendercore | BufferUsage | 枚举 | 缓冲用途 | te/rendercore/resource_desc.hpp | Vertex, Index, Uniform, Storage, TransferSrc, TransferDst（位掩码） |
| 009-RenderCore | te::rendercore | BufferDesc | struct | 缓冲描述 | te/rendercore/resource_desc.hpp | size, usage, alignment；与 RHI 资源创建桥接 |
| 009-RenderCore | te::rendercore | BufferDescParams | struct | 缓冲描述参数 | te/rendercore/resource_desc.hpp | CreateBufferDesc 输入参数 |
| 009-RenderCore | te::rendercore | CreateVertexFormat | 自由函数 | 创建顶点格式 | te/rendercore/resource_desc.hpp | `VertexFormat CreateVertexFormat(VertexFormatDesc const& desc);` 失败返回 invalid |
| 009-RenderCore | te::rendercore | CreateIndexFormat | 自由函数 | 创建索引格式 | te/rendercore/resource_desc.hpp | `IndexFormat CreateIndexFormat(IndexFormatDesc const& desc);` 失败返回 invalid |
| 009-RenderCore | te::rendercore | CreateTextureDesc | 自由函数 | 创建纹理描述 | te/rendercore/resource_desc.hpp | `TextureDesc CreateTextureDesc(TextureDescParams const& params);` 失败返回 invalid |
| 009-RenderCore | te::rendercore | CreateBufferDesc | 自由函数 | 创建缓冲描述 | te/rendercore/resource_desc.hpp | `BufferDesc CreateBufferDesc(BufferDescParams const& params);` 失败返回 invalid |

### Uniform 布局（te/rendercore/uniform_layout.hpp）

| 模块名 | 命名空间 | 符号 | 导出形式 | 接口说明 | 头文件 | 说明 |
|--------|----------|------|----------|----------|--------|------|
| 009-RenderCore | te::rendercore | UniformMemberType | 枚举 | Uniform 成员类型 | te/rendercore/uniform_layout.hpp | Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Unknown |
| 009-RenderCore | te::rendercore | UniformMember | struct | Uniform 成员 | te/rendercore/uniform_layout.hpp | `char name[64]; UniformMemberType type; uint32_t offset; uint32_t size;` |
| 009-RenderCore | te::rendercore | UniformLayoutDesc | struct | Uniform 布局描述 | te/rendercore/uniform_layout.hpp | `UniformMember const* members; uint32_t memberCount; uint32_t totalSize;` |
| 009-RenderCore | te::rendercore | IUniformLayout | 抽象接口 | Uniform 布局 | te/rendercore/uniform_layout.hpp | 常量块、与 Shader 名称/类型一致 |
| 009-RenderCore | te::rendercore | IUniformLayout::GetOffset | 成员函数 | 按名称取偏移 | te/rendercore/uniform_layout.hpp | `size_t GetOffset(char const* name) const = 0;` 未找到返回 0 |
| 009-RenderCore | te::rendercore | IUniformLayout::GetTotalSize | 成员函数 | 取总大小 | te/rendercore/uniform_layout.hpp | `size_t GetTotalSize() const = 0;` |
| 009-RenderCore | te::rendercore | CreateUniformLayout | 自由函数 | 创建布局 | te/rendercore/uniform_layout.hpp | `IUniformLayout* CreateUniformLayout(UniformLayoutDesc const& desc);` 失败返回 nullptr |
| 009-RenderCore | te::rendercore | ReleaseUniformLayout | 自由函数 | 释放布局 | te/rendercore/uniform_layout.hpp | `void ReleaseUniformLayout(IUniformLayout* layout);` nullptr 为 no-op |

### Pass 协议（te/rendercore/pass_protocol.hpp）

| 模块名 | 命名空间 | 符号 | 导出形式 | 接口说明 | 头文件 | 说明 |
|--------|----------|------|----------|----------|--------|------|
| 009-RenderCore | te::rendercore | PassResourceDecl | struct | Pass 资源声明 | te/rendercore/pass_protocol.hpp | `PassHandle pass; ResourceHandle resource; bool isRead; bool isWrite; ResourceLifetime lifetime;` 与 PipelineCore RDG 对接 |
| 009-RenderCore | te::rendercore | DeclareRead | 自由函数 | 声明读资源 | te/rendercore/pass_protocol.hpp | `void DeclareRead(PassHandle pass, ResourceHandle resource);` |
| 009-RenderCore | te::rendercore | DeclareWrite | 自由函数 | 声明写资源 | te/rendercore/pass_protocol.hpp | `void DeclareWrite(PassHandle pass, ResourceHandle resource);` |
| 009-RenderCore | te::rendercore | SetResourceLifetime | 自由函数 | 设置生命周期 | te/rendercore/pass_protocol.hpp | `void SetResourceLifetime(PassResourceDecl& decl, ResourceLifetime lifetime);` |

### Uniform 缓冲（te/rendercore/uniform_buffer.hpp）

| 模块名 | 命名空间 | 符号 | 导出形式 | 接口说明 | 头文件 | 说明 |
|--------|----------|------|----------|----------|--------|------|
| 009-RenderCore | te::rendercore | IUniformBuffer | 抽象接口 | Uniform 缓冲 | te/rendercore/uniform_buffer.hpp | 布局、更新、与 RHI 缓冲绑定 |
| 009-RenderCore | te::rendercore | IUniformBuffer::Update | 成员函数 | 更新内容 | te/rendercore/uniform_buffer.hpp | `void Update(void const* data, size_t size) = 0;` 提交到当前帧 slot |
| 009-RenderCore | te::rendercore | IUniformBuffer::Bind | 成员函数 | 绑定到槽位 | te/rendercore/uniform_buffer.hpp | `void Bind(te::rhi::ICommandList* cmd, uint32_t slot) = 0;` 调用 RHI SetUniformBuffer |
| 009-RenderCore | te::rendercore | IUniformBuffer::GetRingBufferOffset | 成员函数 | 取环缓冲偏移 | te/rendercore/uniform_buffer.hpp | `size_t GetRingBufferOffset(FrameSlotId slot) const = 0;` |
| 009-RenderCore | te::rendercore | IUniformBuffer::SetCurrentFrameSlot | 成员函数 | 设置当前帧 slot | te/rendercore/uniform_buffer.hpp | `void SetCurrentFrameSlot(FrameSlotId slot) = 0;` |
| 009-RenderCore | te::rendercore | CreateUniformBuffer | 自由函数 | 创建缓冲 | te/rendercore/uniform_buffer.hpp | `IUniformBuffer* CreateUniformBuffer(IUniformLayout const* layout, te::rhi::IDevice* device);` 调用 RHI CreateBuffer(Uniform)；失败返回 nullptr |
| 009-RenderCore | te::rendercore | ReleaseUniformBuffer | 自由函数 | 释放缓冲 | te/rendercore/uniform_buffer.hpp | `void ReleaseUniformBuffer(IUniformBuffer* buffer);` 调用 RHI DestroyBuffer；nullptr 为 no-op |

### 头文件与包含关系

| 头文件 | 依赖 | 说明 |
|--------|------|------|
| te/rendercore/types.hpp | \<cstdint\>, \<cstddef\> | ResultCode, PassHandle, ResourceHandle, FrameSlotId, ResourceLifetime, BindSlot |
| te/rendercore/resource_desc.hpp | te/rendercore/types.hpp | VertexFormat, IndexFormat, TextureDesc, BufferDesc 及 Create* 函数 |
| te/rendercore/uniform_layout.hpp | te/rendercore/types.hpp | IUniformLayout, UniformLayoutDesc, CreateUniformLayout, ReleaseUniformLayout |
| te/rendercore/pass_protocol.hpp | te/rendercore/types.hpp | PassResourceDecl, DeclareRead, DeclareWrite, SetResourceLifetime |
| te/rendercore/uniform_buffer.hpp | te/rendercore/types.hpp, te/rendercore/uniform_layout.hpp, te/rhi/device.hpp, te/rhi/command_list.hpp | IUniformBuffer, CreateUniformBuffer, ReleaseUniformBuffer |
| te/rendercore/api.hpp | 以上所有 | 聚合头，下游只需 `#include <te/rendercore/api.hpp>` |

---

*来源：契约能力 ResourceDesc、UniformLayout、PassProtocol、UniformBuffer；与 010-Shader、019-PipelineCore、008-RHI 对接。*
*命名空间与头文件路径与 008-RHI (te::rhi, te/rhi/) 及 001-Core (te::core, te/core/) 保持一致。*

---

## 已实现（原 TODO：010-Shader 反射对接）

010-Shader 产出 Uniform 布局描述供本模块创建 IUniformLayout；以下能力已实现：

- [x] te/rendercore/uniform_layout.hpp 及 CreateUniformLayout/ReleaseUniformLayout
- [x] 与 010-Shader GetReflection 产出的 UniformLayoutDesc 格式对齐（成员类型映射、偏移、命名，std140）
