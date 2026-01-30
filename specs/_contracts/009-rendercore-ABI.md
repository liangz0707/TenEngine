# 009-RenderCore 模块 ABI

- **契约**：[009-rendercore-public-api.md](./009-rendercore-public-api.md)（能力与类型描述）
- **本文件**：009-RenderCore 对外 ABI 显式表。
- **角色**：渲染类型与 Pass 协议，介于 RHI 与管线之间；不直接持有 GPU 资源，与 RHI 桥接。

## ABI 表

列定义：**模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明**

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 009-RenderCore | TenEngine::rendercore | IUniformLayout | 抽象接口 | Uniform 布局 | TenEngine/rendercore/UniformLayout.h | IUniformLayout | 常量块、与 Shader 名称/类型一致；由 CreateUniformLayout 或 Shader 反射产出 |
| 009-RenderCore | TenEngine::rendercore | — | 自由函数/工厂 | 创建 Uniform 布局 | TenEngine/rendercore/UniformLayout.h | CreateUniformLayout | `IUniformLayout* CreateUniformLayout(UniformLayoutDesc const& desc);` 与 Shader 模块约定一致；失败返回 nullptr |
| 009-RenderCore | TenEngine::rendercore | IUniformLayout | 抽象接口 | 按名称取偏移 | TenEngine/rendercore/UniformLayout.h | IUniformLayout::GetOffset | `size_t GetOffset(char const* name) const;` 与 Shader 反射或手写布局对接 |
| 009-RenderCore | TenEngine::rendercore | — | struct | Uniform 布局描述 | TenEngine/rendercore/UniformLayout.h | UniformLayoutDesc | 成员名、类型、对齐；下游按 Shader 约定填充 |
| 009-RenderCore | TenEngine::rendercore | — | 枚举/struct | 顶点格式 | TenEngine/rendercore/ResourceDesc.h | VertexFormat | 顶点属性（位置、法线、UV 等）及格式；与 RHI 创建参数对接 |
| 009-RenderCore | TenEngine::rendercore | — | 枚举/struct | 索引格式 | TenEngine/rendercore/ResourceDesc.h | IndexFormat | uint16_t / uint32_t 等；与 RHI 对接 |
| 009-RenderCore | TenEngine::rendercore | — | struct | 纹理描述 | TenEngine/rendercore/ResourceDesc.h | TextureDesc | 宽高、格式、mip、用途；与 RHI 资源创建桥接 |
| 009-RenderCore | TenEngine::rendercore | — | struct | 缓冲描述 | TenEngine/rendercore/ResourceDesc.h | BufferDesc | 大小、用途、stride；与 RHI 资源创建桥接 |
| 009-RenderCore | TenEngine::rendercore | — | struct/类型别名 | Pass 资源声明 | TenEngine/rendercore/PassProtocol.h | PassResourceDecl | Pass 输入/输出资源声明；与 PipelineCore RDG 对接；单次 Pass 图构建周期 |
| 009-RenderCore | TenEngine::rendercore | — | 自由函数/接口 | Pass 声明读资源 | TenEngine/rendercore/PassProtocol.h | DeclareRead | `void DeclareRead(PassHandle pass, ResourceHandle resource);` 与 PipelineCore DeclareRead 语义一致 |
| 009-RenderCore | TenEngine::rendercore | — | 自由函数/接口 | Pass 声明写资源 | TenEngine/rendercore/PassProtocol.h | DeclareWrite | `void DeclareWrite(PassHandle pass, ResourceHandle resource);` 与 PipelineCore DeclareWrite 语义一致 |
| 009-RenderCore | TenEngine::rendercore | — | 类型别名 | Pass 句柄 | TenEngine/rendercore/PassProtocol.h | PassHandle | 不透明句柄；用于 DeclareRead/DeclareWrite |
| 009-RenderCore | TenEngine::rendercore | — | 类型别名 | 资源句柄 | TenEngine/rendercore/PassProtocol.h | ResourceHandle | 管线内资源句柄；与 PipelineCore 资源生命周期一致 |
| 009-RenderCore | TenEngine::rendercore | IUniformBuffer | 抽象接口 | Uniform 缓冲 | TenEngine/rendercore/UniformBuffer.h | IUniformBuffer | 布局、更新、与 RHI 缓冲绑定；不直接持有 GPU 资源，由 RHI 创建 |
| 009-RenderCore | TenEngine::rendercore | — | 自由函数/工厂 | 创建 Uniform 缓冲 | TenEngine/rendercore/UniformBuffer.h | CreateUniformBuffer | `IUniformBuffer* CreateUniformBuffer(IUniformLayout const* layout, IDevice* device);` 与 RHI 桥接；多帧可 RingBuffer |
| 009-RenderCore | TenEngine::rendercore | IUniformBuffer | 抽象接口 | 更新内容 | TenEngine/rendercore/UniformBuffer.h | IUniformBuffer::Update | `void Update(void const* data, size_t size);` 提交到当前帧 slot；线程安全由实现约定 |
| 009-RenderCore | TenEngine::rendercore | IUniformBuffer | 抽象接口 | 绑定到槽位 | TenEngine/rendercore/UniformBuffer.h | IUniformBuffer::Bind | `void Bind(ICommandList* cmd, uint32_t slot);` 与 Shader/RHI 槽位约定一致 |
| 009-RenderCore | TenEngine::rendercore | IUniformBuffer | 抽象接口 | 多帧环缓冲 | TenEngine/rendercore/UniformBuffer.h | IUniformBuffer::GetRingBufferOffset | `size_t GetRingBufferOffset(FrameSlotId slot) const;` 按帧 slot 取当前环缓冲偏移；可选 |

*来源：契约能力 ShaderParams、ResourceDesc、PassProtocol、UniformBuffer；与 010-Shader、019-PipelineCore、008-RHI 对接。*
