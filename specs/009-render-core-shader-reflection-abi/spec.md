# Feature Specification: 009-RenderCore 完整模块与 Shader Reflection 对接

**Feature Branch**: `009-render-core-shader-reflection-abi`  
**Created**: 2026-02-03  
**Status**: Draft  
**Input**: 本 feature 的完整模块规约见 `docs/module-specs/009-render-core.md`，契约见 `specs/_contracts/009-rendercore-public-api.md`；**本 feature 实现完整模块内容**，含 Shader Reflection 对接与 ABI 中 TODO 项的实现。

## 规约与契约引用 *(模块切片时必填)*

- **完整模块规约**：`docs/module-specs/009-render-core.md`（009-RenderCore 渲染类型与 Pass 协议，介于 RHI 与管线之间）。
- **本模块范围**（本 feature 实现完整模块内容）：
  1. **ResourceDesc**：VertexFormat、IndexFormat、TextureDesc、BufferDesc；CreateVertexFormat/CreateIndexFormat/CreateTextureDesc/CreateBufferDesc；与 RHI 创建参数对接
  2. **UniformLayout (ShaderParams)**：IUniformLayout、UniformLayoutDesc、CreateUniformLayout、ReleaseUniformLayout；GetOffset、GetTotalSize；与 Shader 反射或手写布局对接
  3. **PassProtocol**：DeclareRead、DeclareWrite、SetResourceLifetime；PassResourceDecl、PassHandle、ResourceHandle；与 PipelineCore RDG 对接
  4. **UniformBuffer**：IUniformBuffer、CreateUniformBuffer、ReleaseUniformBuffer；Update、Bind、GetRingBufferOffset、SetCurrentFrameSlot；与 RHI CreateBuffer/UpdateBuffer/SetUniformBuffer 对接
  5. **Shader Reflection 对接**：与 010-Shader 反射产出格式对齐；CreateUniformLayout 接受 010-Shader GetReflection 产出的 UniformLayoutDesc（成员类型映射、偏移、命名一致）
  6. **ABI TODO 实现**：完成 `specs/_contracts/009-rendercore-ABI.md` 中「TODO（010-Shader 反射对接）」所列待实现项

实现时只使用**本 feature 依赖的上游契约**中已声明的类型与 API：
- `specs/_contracts/001-core-public-api.md`（001-Core）
- `specs/_contracts/008-rhi-public-api.md`（008-RHI）
- `specs/_contracts/010-shader-public-api.md`（010-Shader，反射产出格式约定；可选依赖）

- **ABI 与构建规约**（`.specify/memory/constitution.md` §VI）：本模块须实现其 **ABI 文件**（`specs/_contracts/009-rendercore-ABI.md`）中的**全部**符号与能力；构建须通过**引入真实子模块源码**（如 CMake `add_subdirectory`）满足依赖，**禁止**使用 stub、mock 或与契约不一致的代替实现作为长期方案。**契约更新**：接口变更须在对应 **ABI 文件**中**增补或替换**对应的 ABI 条目；下游所需接口须在**上游模块的 ABI 文件**中以 **TODO** 登记。详见 `specs/_contracts/README.md`「契约更新流程」。

- **第三方依赖**：第三方库引入说明在契约 `specs/_contracts/009-rendercore-public-api.md` 中声明；本 spec 引用该契约即可。

## User Scenarios & Testing *(mandatory)*

### User Story 1 - 资源描述创建 (Priority: P1)

管线或材质系统需要创建顶点/索引/纹理/缓冲描述以向 RHI 申请资源；通过本模块的 ResourceDesc API 创建合法描述，并传递给 RHI 创建接口。

**Why this priority**: 资源描述是渲染管线的基础，所有渲染资源创建都依赖此能力。

**Independent Test**: 调用 CreateVertexFormat/CreateIndexFormat/CreateTextureDesc/CreateBufferDesc，传入合法参数，得到 IsValid 的描述；非法参数返回 invalid。可将描述传递给 008-RHI CreateBuffer/CreateTexture 验证对接。

**Acceptance Scenarios**:

1. **Given** 合法 VertexFormatDesc，**When** 调用 CreateVertexFormat，**Then** 返回 IsValid 的 VertexFormat，attributeCount 与 stride 正确
2. **Given** 非法参数（如 nullptr attributes、Unknown format），**When** 调用 Create*，**Then** 返回 invalid 描述（reject at call）
3. **Given** 合法 TextureDescParams/BufferDescParams，**When** 调用 CreateTextureDesc/CreateBufferDesc，**Then** 返回可与 RHI 对接的描述

---

### User Story 2 - Uniform 布局与 Shader 反射对接 (Priority: P1)

材质或 Shader 系统需要从 Shader 反射获取 Uniform 布局并创建 IUniformLayout，用于 CreateUniformBuffer 或按名称查偏移。010-Shader 的 GetReflection 产出 UniformLayoutDesc，本模块 CreateUniformLayout 须能接受该格式。

**Why this priority**: Uniform 布局是 UniformBuffer 与 Shader 绑定的前提；与 010-Shader 反射对接是 ABI TODO 明确要求。

**Independent Test**: 从手写或 010-Shader 反射产出 UniformLayoutDesc，调用 CreateUniformLayout；对每个成员调用 GetOffset(name)，对不存在的成员返回 0；GetTotalSize 满足 std140 对齐。若 010-Shader 可用，用其 GetReflection 产出 desc 验证格式对齐。

**Acceptance Scenarios**:

1. **Given** 手写 UniformLayoutDesc（members、memberCount、totalSize），**When** 调用 CreateUniformLayout，**Then** 返回非空 IUniformLayout
2. **Given** 有效 layout，**When** 调用 GetOffset("memberName")，**Then** 返回正确字节偏移；对不存在的 name 返回 0
3. **Given** 010-Shader GetReflection 产出的 UniformLayoutDesc，**When** 调用 CreateUniformLayout，**Then** 成功创建；成员类型映射、偏移、命名与 010-Shader 约定一致

---

### User Story 3 - Pass 资源声明 (Priority: P2)

PipelineCore 在构建 Pass 图时需要声明 Pass 对资源的读/写及生命周期；通过 DeclareRead、DeclareWrite、SetResourceLifetime 完成声明。

**Why this priority**: Pass 协议是 RDG 风格管线的前提，但不阻塞单 Pass 渲染。

**Independent Test**: 调用 DeclareRead/DeclareWrite，传入有效 PassHandle、ResourceHandle；调用 SetResourceLifetime 设置 PassResourceDecl 的 lifetime。验证 decl 状态正确。

**Acceptance Scenarios**:

1. **Given** 有效 pass 与 resource，**When** 调用 DeclareRead，**Then** 产生 isRead=true 的 PassResourceDecl
2. **Given** PassResourceDecl，**When** 调用 SetResourceLifetime(decl, Persistent)，**Then** decl.lifetime 为 Persistent

---

### User Story 4 - UniformBuffer 创建、更新与绑定 (Priority: P1)

渲染 Pass 需要创建 UniformBuffer、每帧更新数据、绑定到命令列表 slot。直接调用 008-RHI 的 CreateBuffer(Uniform)、UpdateBuffer、SetUniformBuffer。

**Why this priority**: UniformBuffer 是逐 Pass/逐帧参数传递的核心能力。

**Independent Test**: 使用有效 IUniformLayout 与 te::rhi::IDevice* 调用 CreateUniformBuffer；调用 Update 写入数据；调用 Bind 绑定到 ICommandList；GetRingBufferOffset 返回各 slot 偏移；ReleaseUniformBuffer 释放。无 RHI device 时 CreateUniformBuffer 返回 nullptr。

**Acceptance Scenarios**:

1. **Given** 有效 layout 与 device，**When** 调用 CreateUniformBuffer，**Then** 返回非空 IUniformBuffer
2. **Given** 有效 ub，**When** 调用 Update(data, size)，**Then** 数据写入当前帧 slot 对应 GPU 缓冲
3. **Given** 有效 ub 与 cmd，**When** 调用 Bind(cmd, slot)，**Then** RHI SetUniformBuffer 被调用

---

### Edge Cases

- 当 layout 或 device 为 nullptr 时，CreateUniformBuffer 返回 nullptr
- 当 CreateUniformLayout 接收的 UniformLayoutDesc 成员类型为 Unknown 时，返回 nullptr
- 当 010-Shader 尚未实现 GetReflection 时，本模块仍支持手写 UniformLayoutDesc；格式约定以 ABI 与 010-Shader 契约为准，待 010-Shader 实现后对齐验证

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: 模块 MUST 实现 `specs/_contracts/009-rendercore-ABI.md` 中列出的全部符号与能力
- **FR-002**: ResourceDesc 的 Create* 函数 MUST 对非法输入返回 invalid 描述（reject at call）
- **FR-003**: CreateUniformLayout MUST 接受 010-Shader GetReflection 产出的 UniformLayoutDesc 格式（成员类型映射、偏移、命名与 010-Shader 契约一致）
- **FR-004**: UniformBuffer 的 Update/Bind MUST 直接调用 008-RHI 的 UpdateBuffer、SetUniformBuffer；禁止 no-op 或 TODO 占位
- **FR-005**: 构建 MUST 通过 add_subdirectory 引入 008-RHI 源码，禁止使用 stub 或占位 target
- **FR-006**: 实现时 MUST 只使用 001-Core、008-RHI 契约中已声明的类型与 API
- **FR-007**: 完成 ABI 中 TODO（010-Shader 反射对接）所列两项：实现 uniform_layout 及 CreateUniformLayout/ReleaseUniformLayout；与 010-Shader GetReflection 产出格式对齐

### Key Entities

- **UniformLayoutDesc**：Uniform 成员列表、类型、偏移、总大小；可由手写或 010-Shader 反射产出
- **IUniformLayout**：不透明布局句柄；提供 GetOffset、GetTotalSize
- **IUniformBuffer**：Uniform 缓冲句柄；持有 RHI IBuffer，支持 RingBuffer 多帧 slot
- **PassResourceDecl**：Pass 对资源的读/写声明及生命周期

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: 全部 ABI 符号可被下游通过 `#include <te/rendercore/api.hpp>` 调用
- **SC-002**: 单元测试覆盖 ResourceDesc、UniformLayout、PassProtocol、UniformBuffer；契约测试验证与 RHI 对接
- **SC-003**: CreateUniformLayout 接受的 UniformLayoutDesc 格式与 010-Shader 反射产出约定一致（类型映射、std140 对齐）
- **SC-004**: CMake 配置与构建成功，无 stub 或占位；缺失 008-RHI 时 FATAL_ERROR

## Interface Contracts *(multi-agent sync)*

- **本模块实现的契约**：`specs/_contracts/009-rendercore-public-api.md`
- **本模块依赖的契约**：`specs/_contracts/001-core-public-api.md`、`specs/_contracts/008-rhi-public-api.md`；可选 `specs/_contracts/010-shader-public-api.md`（反射格式约定）
- **ABI/构建**：须实现 ABI 中全部符号；构建须引入真实子模块代码，禁止长期使用 stub 或代替方案。接口变更须在 ABI 文件中更新完整条目；下游所需接口须在上游 ABI 中以 TODO 登记（constitution §VI、`specs/_contracts/README.md`）。

## Dependencies

- **001-Core**：`specs/_contracts/001-core-public-api.md`（内存、数学、日志等）
- **008-RHI**：`specs/_contracts/008-rhi-public-api.md`（IDevice、IBuffer、ICommandList、CreateBuffer、UpdateBuffer、SetUniformBuffer、BufferUsage::Uniform）
- **010-Shader**（可选）：`specs/_contracts/010-shader-public-api.md`（Reflection 产出 Uniform 布局；格式约定用于 CreateUniformLayout 输入对齐）
