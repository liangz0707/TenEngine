# 009-RenderCore 与 008-RHI 描述符对接约定

**目的**：说明 009-RenderCore 产出的 BufferDesc、TextureDesc、VertexFormat、IndexFormat 与 008-RHI 的 `te::rhi::BufferDesc`、`te::rhi::TextureDesc` 及顶点/索引创建参数的对应关系，供 009 实现与 008 对接时使用。

**适用**：本 feature（008-rhi-fullmodule-006）US8；009 未实现时可先按本约定设计转换或共用类型。

---

## 1. BufferDesc 对接

| 009-RenderCore（契约） | 008-RHI（te::rhi） | 说明 |
|------------------------|--------------------|------|
| BufferDesc：大小、用途、stride | BufferDesc：`size_t size; uint32_t usage;` | **字段对应**：009 的「大小」→ `size`；「用途」→ `usage`（008 侧为 BufferUsage 位掩码）。009 的 stride 用于顶点/索引缓冲时由 009 在创建顶点缓冲时使用，008 不要求 BufferDesc 带 stride；若 009 需传 stride，可放在扩展字段或由 009 在 CreateBuffer 前根据 VertexFormat 计算 size。 |
| 用途含 Uniform | `usage` 须包含 `BufferUsage::Uniform`（`1u << 2`） | CreateUniformBuffer 等价于 CreateBuffer(BufferDesc{ layout 大小, \| BufferUsage::Uniform })；009 IUniformBuffer::Update 对应 IDevice::UpdateBuffer，Bind 对应 ICommandList::SetUniformBuffer。 |

**转换规则**：009 的 BufferDesc 若与 008 字段一致，可**直接强转或逐字段拷贝**到 `te::rhi::BufferDesc`；若 009 使用不同命名空间（如 `TenEngine::rendercore::BufferDesc`），需提供转换函数或约定共用头文件（如 008 提供、009 引用）。

---

## 2. TextureDesc 对接

| 009-RenderCore（契约） | 008-RHI（te::rhi） | 说明 |
|------------------------|--------------------|------|
| TextureDesc：宽高、格式、mip、用途 | TextureDesc：`uint32_t width, height, depth, format;` | **字段对应**：009 的宽高深→ `width, height, depth`；格式→ `format`。008 当前无 `mipLevels`、`usage` 字段；若 009 需要，可在文档中标注为**扩展点**，后续 008 增加或 009 在调用 CreateTexture 前忽略/默认。 |

**转换规则**：009 TextureDesc 与 008 的 width/height/depth/format 一一对应即可转换；额外字段（mip、usage）由 009 侧填充默认或与 008 契约同步后扩展。

---

## 3. VertexFormat / IndexFormat 对接

| 009-RenderCore | 008-RHI | 说明 |
|----------------|---------|------|
| VertexFormat（顶点属性、格式） | 顶点缓冲由 CreateBuffer(BufferDesc{…}) 创建；格式由 PSO/管线或 SetVertexBuffers 时约定 | 008 当前无独立 VertexFormat 类型；009 的 VertexFormat 用于描述布局，008 接收 IBuffer + 偏移/stride。若 009 与 008 共用同一枚举（如 te::rhi::VertexFormat），可注明共用头文件；否则 009 提供到 RHI 顶点属性的映射表。 |
| IndexFormat（uint16_t / uint32_t） | 索引格式在 DrawIndexed 或后端约定（MTLIndexTypeUInt16/UInt32 等） | 008 未在 ABI 中显式暴露 IndexFormat 枚举；009 IndexFormat 可映射为后端所需类型，由 009 在调用 DrawIndexed 或创建索引缓冲时使用。 |

**约定**：009 产出的 VertexFormat、IndexFormat 与 008 的顶点/索引创建参数**可转换或一一对应**；若 009 使用同一类型则注明共用命名空间/头文件。

---

## 4. 扩展点与 009 契约同步

- **BufferDesc**：008 当前无 `stride`；若 009 强制需要，可后续在 008 BufferDesc 增加或由 009 在应用层维护。
- **TextureDesc**：008 当前无 `mipLevels`、`usage`；若 009 契约要求，需与 008 契约同步后在 008 侧增补或文档标注默认行为。
- **共用类型**：若 009 与 008 约定共用 `te::rhi::BufferDesc` / `te::rhi::TextureDesc`，则 009 直接引用 `include/te/rhi/resources.hpp`，无需转换层。

---

*本约定由 008-rhi-fullmodule-006 Phase 4（US8）产出；与 specs/_contracts/009-rendercore-public-api.md、009-rendercore-ABI.md 及 specs/_contracts/008-rhi-ABI.md 一致。*
