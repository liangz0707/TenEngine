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

## API 雏形（本 feature 009-render-core-full 产出）

以下为对外暴露的类型与函数签名，仅使用 001-Core、008-RHI 契约已声明类型；不暴露 RHI 内部类型。

### 类型与句柄（跨边界，C++ 侧）

- **UniformLayout** — 不透明句柄或类；表示 Uniform Buffer 布局、常量块；与 Shader 名称/类型一致。生命周期：定义后直至卸载。
- **VertexFormat** — 结构体或描述符；顶点属性、步长、与 RHI 顶点输入对接。生命周期：定义后直至卸载。
- **IndexFormat** — 枚举或描述符；16/32 位索引类型，与 RHI 创建参数对接。生命周期：定义后直至卸载。
- **TextureDesc** — 结构体；宽度、高度、深度、格式、mip、用途等，与 RHI 纹理创建桥接。由调用方管理。
- **BufferDesc** — 结构体；大小、用途、对齐等，与 RHI 缓冲创建桥接。由调用方管理。
- **PassResourceDecl** — 结构体或句柄；单次 Pass 图构建内有效；表示一次 DeclareRead/DeclareWrite 及可选的 ResourceLifetime。
- **UniformBufferHandle** — 不透明句柄；表示由 CreateLayout 创建的 Uniform 缓冲；用于 Update、RingBuffer、Bind。生命周期：创建后直至显式释放。

### 1. ShaderParams

- `UniformLayout DefineLayout(UniformLayoutDesc const& desc);`
  - 根据描述（常量块、成员名/类型）创建 UniformLayout；与 Shader 反射或手写布局一致。失败返回空/无效句柄（依实现约定）。
- `size_t GetOffset(UniformLayout layout, char const* memberName);`
  - 返回 layout 中成员 `memberName` 的字节偏移；若不存在或 layout 无效，依实现约定（如 0 或 assert）。

### 2. ResourceDesc

- `VertexFormat CreateVertexFormat(VertexFormatDesc const& desc);`
  - 根据属性列表、步长等创建 VertexFormat；若格式/尺寸不被 RHI 支持则拒绝（返回空或错误码，不创建资源）。
- `IndexFormat CreateIndexFormat(IndexFormatDesc const& desc);`
  - 创建 IndexFormat（如 16/32 位）；不支持则拒绝。
- `TextureDesc CreateTextureDesc(TextureDescParams const& params);`
  - 填写 TextureDesc 结构体；若参数不被 RHI 支持则拒绝（返回无效描述或错误码）。
- `BufferDesc CreateBufferDesc(BufferDescParams const& params);`
  - 填写 BufferDesc 结构体；若参数不被 RHI 支持则拒绝。

### 3. PassProtocol

- `PassResourceDecl DeclareRead(PassHandle pass, ResourceHandle resource);`
  - 为 pass 声明读资源 resource；返回 PassResourceDecl（或 void，依协议）。
- `PassResourceDecl DeclareWrite(PassHandle pass, ResourceHandle resource);`
  - 为 pass 声明写资源 resource；返回 PassResourceDecl。
- `void SetResourceLifetime(PassResourceDecl decl, ResourceLifetime lifetime);`
  - 设置声明资源的生命周期；须与 PipelineCore RDG 协议及 RHI 要求一致。同资源同 pass 读写是否允许由 PipelineCore 定义，本模块不额外限制。

### 4. UniformBuffer

- `UniformBufferHandle CreateLayout(UniformLayout layout);`
  - 根据 UniformLayout 创建 Uniform 缓冲并返回 UniformBufferHandle；失败返回空/无效句柄。可选：在创建时校验 layout 与 Shader 约定一致，不一致则返回错误。
- `void Update(UniformBufferHandle handle, void const* data, size_t size);`
  - 更新 handle 对应缓冲内容；data/size 须与 layout 一致。
- `void RingBufferAdvance(UniformBufferHandle handle);` 或 `uint32_t RingBufferAllocSlot(UniformBufferHandle handle);`
  - 多帧环缓冲：推进槽位或分配槽位；当无可用槽位时 **Block**（阻塞或要求调用方等待/重试直到有槽位释放），不静默覆盖 in-flight 数据。
- `void Bind(UniformBufferHandle handle, BindSlot slot);`
  - 将 handle 绑定到 RHI 的 slot；与 Shader 模块及 RHI 绑定约定一致。

### 调用顺序与约束（API 雏形）

- 须在 Core、RHI 初始化之后调用上述 API。
- 描述符/布局与 RHI 创建参数、Shader 约定一致；Pass 声明与 PipelineCore（019）Pass 图协议一致；资源生命周期不违反 RHI 要求。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 按 009-RenderCore 模块规格与依赖表新增契约；类型与能力与 docs/module-specs/009-render-core.md 一致 |
| 2026-01-29 | contract(009-rendercore): sync API sketch from plan 009-render-core-full |
| 2026-01-29 | contract(009-rendercore): add PassHandle and ResourceHandle to types table |
