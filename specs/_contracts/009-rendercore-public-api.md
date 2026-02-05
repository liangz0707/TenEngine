# 契约：009-RenderCore 模块对外 API

## 适用模块

- **实现方**：009-RenderCore（L2；Shader 参数结构、渲染资源描述、Pass 参数协议、Uniform Buffer；介于 RHI 与管线之间）
- **对应规格**：`docs/module-specs/009-render-core.md`
- **依赖**：001-Core、008-RHI

## 消费者

- 010-Shader、011-Material、012-Mesh、019-PipelineCore、020-Pipeline、021-Effects、022-2D、023-Terrain、028-Texture

## 能力列表

### 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| UniformLayout | Uniform Buffer 布局、常量块、与 Shader 名称/类型一致 | 定义后直至卸载 |
| VertexFormat / IndexFormat | 顶点/索引格式描述，与 RHI 创建参数对接 | 定义后直至卸载 |
| PassResourceDecl | Pass 输入/输出资源声明、与 PipelineCore RDG 对接 | 单次 Pass 图构建周期 |
| UniformBufferHandle | Uniform 缓冲句柄；布局、更新、多帧环缓冲、与 RHI 绑定 | 创建后直至显式释放 |
| TextureDesc / BufferDesc | 纹理/缓冲描述，与 RHI 资源创建桥接 | 由调用方管理 |
| PassHandle | Pass 标识/句柄，用于 DeclareRead/DeclareWrite | 单次 Pass 图构建或由 PipelineCore 管理 |
| ResourceHandle | 资源句柄，用于声明 Pass 读/写资源 | 与资源生命周期一致 |
| ShaderReflectionDesc（可选） | ShaderResourceKind、ShaderResourceBinding；Uniform、Texture、Sampler 反射 | 与 Shader 或缓存绑定 |

不直接暴露 RHI 资源，由 RenderCore 与 RHI 桥接。

### 能力（提供方保证）

| 序号 | 能力 | 说明 |
|------|------|------|
| 1 | ShaderParams | DefineLayout、GetOffset；Uniform 布局与 Shader 反射或手写布局对接 |
| 2 | ResourceDesc | VertexFormat、IndexFormat、TextureDesc、BufferDesc；与 RHI 创建参数对接 |
| 3 | PassProtocol | DeclareRead、DeclareWrite、ResourceLifetime；与 PipelineCore RDG 协议对接 |
| 4 | UniformBuffer | CreateLayout、Update、RingBuffer、Bind；与 Shader 及 RHI 缓冲绑定对接 |

命名空间 `te::rendercore`。

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。

## 约束

- 须在 Core、RHI 初始化之后使用；与 Shader 的 Uniform 布局约定须一致。Pass 资源声明与 PipelineCore（019）的 Pass 图协议须一致。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 009-RenderCore 契约 |
| 2026-02-05 | 统一目录；能力列表用表格 |
