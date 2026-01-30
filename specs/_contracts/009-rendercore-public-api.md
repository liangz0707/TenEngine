# 契约：009-RenderCore 模块对外 API

## 适用模块

- **实现方**：**009-RenderCore**（渲染类型与 Pass 协议，介于 RHI 与管线之间）
- **对应规格**：`docs/module-specs/009-render-core.md`
- **依赖**：001-Core（001-core-public-api）、008-RHI（008-rhi-public-api）

## 消费者（T0 下游）

- 010-Shader（Uniform 布局约定、资源描述）
- 011-Material（Shader 参数结构、Uniform Buffer）
- 012-Mesh（顶点/索引格式、资源描述）
- 019-PipelineCore（Pass 资源声明、资源生命周期协议）
- 020-Pipeline（Pass 协议、Uniform、资源描述）
- 021-Effects（Pass 协议、Uniform）
- 022-2D、023-Terrain（资源描述、Pass 协议）

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。
- 当前契约版本：（由实现或计划阶段填写）

## 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| UniformLayout | Uniform Buffer 布局、常量块、与 Shader 名称/类型一致 | 定义后直至卸载 |
| VertexFormat / IndexFormat | 顶点/索引格式描述，与 RHI 创建参数对接 | 定义后直至卸载 |
| PassResourceDecl | Pass 输入/输出资源声明、与 PipelineCore RDG 对接 | 单次 Pass 图构建周期 |
| UniformBufferHandle | Uniform 缓冲句柄；布局、更新、多帧环缓冲、与 RHI 绑定 | 创建后直至显式释放 |
| TextureDesc / BufferDesc | 纹理/缓冲描述，与 RHI 资源创建桥接 | 由调用方管理 |
| PassHandle | Pass 标识/句柄，用于 DeclareRead/DeclareWrite 声明所属 Pass | 单次 Pass 图构建周期或由 PipelineCore 管理 |
| ResourceHandle | 资源句柄，用于声明 Pass 读/写资源 | 与资源生命周期一致或由调用方/PipelineCore 管理 |

下游仅通过上述类型与句柄访问；不直接暴露 RHI 资源，由 RenderCore 与 RHI 桥接。

## 能力列表（提供方保证）

1. **ShaderParams**：DefineLayout、GetOffset；Uniform 布局与 Shader 反射或手写布局对接。
2. **ResourceDesc**：VertexFormat、IndexFormat、TextureDesc、BufferDesc；与 RHI 创建参数对接。
3. **PassProtocol**：DeclareRead、DeclareWrite、ResourceLifetime；与 PipelineCore RDG 协议对接。
4. **UniformBuffer**：CreateLayout、Update、RingBuffer、Bind；与 Shader 模块及 RHI 缓冲绑定对接。

## 调用顺序与约束

- 须在 Core、RHI 初始化之后使用；与 Shader 模块的 Uniform 布局约定须一致。
- Pass 资源声明与 PipelineCore（019）的 Pass 图协议须一致；资源生命周期不得违反 RHI 要求。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 按 009-RenderCore 模块规格与依赖表新增契约；类型与能力与 docs/module-specs/009-render-core.md 一致 |
| 2026-01-29 | contract(009-rendercore): sync API sketch from plan 009-render-core-full |
| 2026-01-29 | contract(009-rendercore): add PassHandle and ResourceHandle to types table |
