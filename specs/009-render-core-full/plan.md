# Implementation Plan: 009-render-core full

**Branch**: `009-render-core-full` | **Date**: 2026-01-29 | **Spec**: [spec.md](./spec.md)  
**Input**: Feature specification from `specs/009-render-core-full/spec.md`. Full module scope: ShaderParams, ResourceDesc, PassProtocol, UniformBuffer.

**规约与契约**：`docs/module-specs/009-render-core.md`、`specs/_contracts/009-rendercore-public-api.md`；上游 `specs/_contracts/001-core-public-api.md`、`specs/_contracts/008-rhi-public-api.md`。仅暴露/使用契约已声明的类型与 API。

---

## Summary

实现 009-RenderCore 完整模块：提供渲染类型与 Pass 协议（Shader 参数结构、渲染资源描述、Pass 参数协议、Uniform Buffer），介于 RHI 与管线之间。技术栈 C++17、CMake；依赖 001-Core（内存、数学）、008-RHI（设备、缓冲/纹理创建、绑定）；对外仅暴露契约中的类型与句柄，不暴露 RHI 内部类型。

---

## Technical Context

**Language/Version**: C++17  
**Build**: CMake（单一日志构建；可重现）  
**Primary Dependencies**: 001-Core（Alloc/Free、数学/容器）、008-RHI（IDevice、IBuffer、ITexture、资源创建与绑定）  
**Storage**: N/A（本模块仅提供描述符与句柄；GPU 资源由 RHI 持有）  
**Testing**: 单元测试（描述符与布局）、契约测试（与 RHI 创建参数对接）、集成测试（与 PipelineCore 协议对齐）  
**Target Platform**: Windows / Linux / macOS（与 RHI 后端一致）

**Project Type**: 引擎库（静态库或动态库，由主工程或下游模块链接）  
**Performance Goals**: 描述符与布局解析无热点；RingBuffer 槽位分配/等待与帧预算一致；具体帧时目标由 feature spec 或下游约定。  
**Constraints**: 须在 Core、RHI 初始化之后使用；仅使用上游契约声明的类型与 API；RingBuffer 耗尽时 Block；不支持的格式/尺寸在 API 调用时拒绝。  
**Scale/Scope**: 本切片为完整模块（四子模块：ShaderParams、ResourceDesc、PassProtocol、UniformBuffer）。

---

## 依赖引入方式（TenEngine 构建规约：必须澄清）

| 依赖模块 | 引入方式 | 说明 |
|----------|----------|------|
| 001-Core | **源码** | 通过 TenEngine 或工作区引入上游源码构建；使用 Alloc/Free、数学、容器等契约声明 API。 |
| 008-RHI | **源码** | 通过 TenEngine 或工作区引入上游源码构建；使用 IDevice、IBuffer/ITexture 创建与绑定等契约声明 API。 |

**说明**：未列出之依赖按**源码**处理。构建根目录与各模块 CMake 子目录由主工程或 `docs/build-module-convention.md` 约定；本 plan 不直接执行 cmake，由 tasks/implement 在澄清后的根目录执行构建。

---

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

| 原则 | 状态 | 说明 |
|------|------|------|
| I. Modular Renderer Architecture | PASS | 009-RenderCore 为独立模块，边界清晰，依赖 Core/RHI。 |
| II. Modern Graphics API First | PASS | 通过 RHI 抽象对接 Vulkan/D3D12/Metal；不暴露固定功能或后端类型。 |
| III. Data-Driven Pipeline | PASS | 描述符与布局为数据驱动；与 Shader/管线配置解耦。 |
| IV. Performance & Observability | PASS | 帧预算与 RingBuffer 行为在 spec 中约定；可加计时/日志（使用 Core 契约）。 |
| V. Versioning & ABI Stability | PASS | 对外 API 与契约一致；破坏性变更递增 MAJOR。 |
| VI. Module Boundaries & Contract-First | PASS | 仅暴露契约类型与能力；仅使用 001-Core、008-RHI 契约声明 API。 |
| Technology Stack | PASS | C++17、CMake；与 Constitution 一致。 |

---

## Project Structure

### Documentation (this feature)

```text
specs/009-render-core-full/
├── plan.md              # 本文件
├── spec.md              # Feature 规约
├── research.md          # Phase 0 研究结论
├── data-model.md        # Phase 1 数据模型
├── quickstart.md        # Phase 1 快速上手
├── checklists/
│   └── requirements.md
└── tasks.md             # /speckit.tasks 产出（本命令不创建）
```

### Source Code (repository root，建议布局)

```text
# 009-RenderCore 模块源码（实际路径以主工程 CMake 为准）
src/render_core/                    # 或 modules/009-render-core/src
├── shader_params.hpp / .cpp        # ShaderParams: DefineLayout, GetOffset
├── resource_desc.hpp / .cpp        # ResourceDesc: VertexFormat, IndexFormat, TextureDesc, BufferDesc
├── pass_protocol.hpp / .cpp        # PassProtocol: DeclareRead, DeclareWrite, ResourceLifetime
├── uniform_buffer.hpp / .cpp       # UniformBuffer: CreateLayout, Update, RingBuffer, Bind
├── types.hpp                       # 契约类型：UniformLayout, VertexFormat, IndexFormat, TextureDesc, BufferDesc, PassResourceDecl, UniformBufferHandle
└── api.hpp                         # 对外头文件聚合（仅暴露契约 API）

tests/
├── unit/                           # 描述符、布局、RingBuffer 单元测试
├── contract/                       # 与 RHI 创建参数对接的契约测试
└── integration/                   # 与 PipelineCore 协议对齐的集成测试（可选）
```

**Structure Decision**: 单库、按子模块分文件（ShaderParams、ResourceDesc、PassProtocol、UniformBuffer）；对外单一头文件或按子能力拆分，与契约「能力列表」一致。

---

## Complexity Tracking

无违规；不填写。

---

## Phase 0 / Phase 1 产出说明

- **research.md**：技术选型与澄清结论（C++17、CMake、上游契约类型；RingBuffer Block、CreateLayout 语义、拒绝策略、Pass 读写由 PipelineCore 定义、Uniform 可选校验已由 spec 澄清）。
- **data-model.md**：实体与字段（UniformLayout、VertexFormat、IndexFormat、TextureDesc、BufferDesc、PassResourceDecl、UniformBufferHandle、ResourceLifetime）；与契约类型一一对应。
- **quickstart.md**：构建与最小使用示例（初始化顺序、创建描述符、DefineLayout/CreateLayout、Update/Bind、Pass 声明示例）。
- **contracts/**：本 feature 对外 API 雏形已写入下方「契约更新」小节，可直接粘贴到 `specs/_contracts/009-rendercore-public-api.md` 的「API 雏形」小节；不重复建 contracts/ 子目录亦可。

---

## 契约更新（API 雏形）

以下内容可直接粘贴到 `specs/_contracts/009-rendercore-public-api.md` 的「API 雏形」小节（若该小节已存在可替换为以下内容）。命名空间与命名以 C++17 风格为例，实际命名可与项目一致。

```markdown
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

### 调用顺序与约束

- 须在 Core、RHI 初始化之后调用上述 API。
- 描述符/布局与 RHI 创建参数、Shader 约定一致；Pass 声明与 PipelineCore（019）Pass 图协议一致；资源生命周期不违反 RHI 要求。
```

---

**Plan 结束。** 下一步：运行 `/speckit.tasks` 将本 plan 拆解为可执行任务；实现时仅使用契约声明的类型与 API，并将上述「契约更新」同步到 `T0-contracts` 分支上的 `specs/_contracts/009-rendercore-public-api.md`。
