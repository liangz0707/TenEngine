# Data Model: 009-render-core full

**Branch**: `009-render-core-full` | **Date**: 2026-01-29  
**Source**: [spec.md](./spec.md) Key Entities + 契约类型与句柄。

## 实体与契约对应

| 实体 | 契约名称 | 语义 | 生命周期 | 主要字段/关系 |
|------|----------|------|----------|----------------|
| UniformLayout | UniformLayout | Uniform Buffer 布局、常量块；与 Shader 名称/类型一致 | 定义后直至卸载 | 常量块列表、成员名与类型、总大小与对齐 |
| VertexFormat | VertexFormat | 顶点属性、步长；与 RHI 顶点输入对接 | 定义后直至卸载 | 属性列表（location、format、offset、stride） |
| IndexFormat | IndexFormat | 索引类型（16/32 位）；与 RHI 创建参数对接 | 定义后直至卸载 | 枚举或描述（Index16/Index32） |
| TextureDesc | TextureDesc | 纹理描述；与 RHI 纹理创建桥接 | 由调用方管理 | width、height、depth、format、mipCount、usage 等 |
| BufferDesc | BufferDesc | 缓冲描述；与 RHI 缓冲创建桥接 | 由调用方管理 | size、usage、alignment 等 |
| PassResourceDecl | PassResourceDecl | Pass 输入/输出资源声明；与 PipelineCore RDG 对接 | 单次 Pass 图构建周期 | pass、resource、read/write、ResourceLifetime |
| UniformBufferHandle | UniformBufferHandle | Uniform 缓冲句柄；布局、更新、RingBuffer、RHI 绑定 | 创建后直至显式释放 | 对应 UniformLayout；内部槽位/环缓冲状态 |
| ResourceLifetime | ResourceLifetime | 声明资源的生命周期 | 与 PassResourceDecl 关联 | 单帧/多帧/Pass 图内等；与 PipelineCore 协议一致 |

## 关系与约束

- **ShaderParams**：DefineLayout 产出 UniformLayout；GetOffset(UniformLayout, member) 返回字节偏移。
- **ResourceDesc**：VertexFormat、IndexFormat、TextureDesc、BufferDesc 由工厂或描述符创建；与 RHI 创建参数一一对应；不支持的格式/尺寸在 API 调用时拒绝。
- **PassProtocol**：DeclareRead/DeclareWrite 产出 PassResourceDecl；SetResourceLifetime 绑定 ResourceLifetime；同资源同 pass 读写由 PipelineCore 定义。
- **UniformBuffer**：CreateLayout(UniformLayout) 产出 UniformBufferHandle；Update/RingBufferAdvance（或 AllocSlot）/Bind 作用于 handle；RingBuffer 耗尽时 Block。

## 校验规则（来自 spec）

- 描述符：请求不支持格式/尺寸 → 拒绝（错误或无效句柄）。
- Uniform 与 Shader：可选在 CreateLayout 或 Bind 时校验；不一致 → 返回错误；行为须文档化。
- 初始化顺序：须在 Core、RHI 初始化之后使用本模块 API。
