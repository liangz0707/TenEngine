# Data Model: 008-RHI 完整模块实现（含 ABI TODO）

**Branch**: `008-rhi-fullmodule-006` | **Phase**: 1

## 实体与关系（增量：相对 005 新增/修改）

### Buffer 用途与 CPU 写入（本次新增）

| 实体 | 字段/属性 | 关系 | 说明 |
|------|-----------|------|------|
| BufferUsage | Vertex, Index, **Uniform**, Storage, CopySrc, CopyDst（位掩码） | 枚举 | BufferDesc.usage 使用此位掩码；CreateUniformBuffer 等须传 Uniform 位 |
| BufferDesc | size, **usage（BufferUsage 位掩码）** | 输入 | **语义补充**：usage 含 Uniform 时表示可用于 Uniform 缓冲 |
| IDevice::UpdateBuffer | buf, offset, data, size | 写入 IBuffer | CPU 数据写入 GPU 缓冲；满足 009 UniformBuffer::Update |

### Uniform 绑定（本次新增）

| 实体 | 字段/属性 | 关系 | 说明 |
|------|-----------|------|------|
| ICommandList::SetUniformBuffer | slot, buffer, offset | 绑定到命令列表 | 将 IBuffer 绑定到 slot，供后续 Draw/Dispatch 使用；满足 009 IUniformBuffer::Bind |

### 与 009 对接

| 约定 | 说明 |
|------|------|
| 009 BufferDesc/TextureDesc | 须可转换为 te::rhi::BufferDesc/TextureDesc（字段对应或转换）；008 不定义 009 专用类型 |
| VertexFormat / IndexFormat | 009 与 008 约定同一枚举或映射表，便于顶点缓冲与 Draw 参数一致 |

## 其余实体（与 005 data-model 一致）

设备与后端、队列与命令、资源与视图（除 BufferDesc.usage 与 UpdateBuffer 外）、PSO、同步、交换链与描述符集、光追等实体与 008-rhi-fullmodule-005 的 data-model 一致，不重复列出。
