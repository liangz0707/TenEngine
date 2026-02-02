# Research: 008-RHI 完整模块实现（含 ABI TODO）

**Branch**: `008-rhi-fullmodule-006` | **Phase**: 0

## 1. Buffer CPU 写入形式（UpdateBuffer vs Map/Unmap）

**Decision**: 采用 **IDevice::UpdateBuffer(IBuffer* buf, size_t offset, void const* data, size_t size)** 作为本 feature 的 CPU 写入接口，满足 009 UniformBuffer::Update。不强制要求 IBuffer::Map/Unmap；若后端需暂存或 staging 由实现内部完成。

**Rationale**: spec 允许“形式之一”；UpdateBuffer 对下游 009 调用简单（单次调用完成写入），且与 Unity/Unreal 的 Upload 语义一致。Map/Unmap 可作为后续扩展，不纳入本次 ABI 必选。

**Alternatives considered**:
- IBuffer::Map/Unmap：需扩展 IBuffer 接口，且各后端 Map 语义（持久映射 vs 临时）不一；UpdateBuffer 由设备统一封装更稳。
- 仅 Map/Unmap：D3D11 等后端 Map 语义明确，但 Vulkan 需 staging buffer 或 host-visible 分配，统一为 UpdateBuffer 便于实现分层。

---

## 2. Uniform 绑定形式（SetUniformBuffer vs BindDescriptorSet）

**Decision**: 采用 **ICommandList::SetUniformBuffer(uint32_t slot, IBuffer* buffer, size_t offset)** 作为本 feature 的 Uniform 绑定接口，满足 009 IUniformBuffer::Bind。描述符集路径（UpdateDescriptorSet + BindDescriptorSet）保留为 P2 能力；本次不强制各后端必须实现描述符集，但 SetUniformBuffer 须真实实现（映射到各后端的 constant buffer 绑定）。

**Rationale**: spec 允许“形式之一”；SetUniformBuffer 对 009 的 Bind(slot, buffer, offset) 一一对应，且不依赖描述符集是否完整实现。各后端均可通过现有 binding 机制实现（Vulkan push constant / descriptor、D3D12 CBV、Metal setVertexBuffer/setFragmentBuffer 等）。

**Alternatives considered**:
- 仅 BindDescriptorSet：需先 AllocateDescriptorSet、UpdateDescriptorSet，对 009 调用链更长；SetUniformBuffer 更直接。
- 两者都必选：增加实现与测试负担；本次以 SetUniformBuffer 为主，描述符集保持 P2。

---

## 3. BufferDesc.usage 与 BufferUsage 枚举

**Decision**: 在 te/rhi/resources.hpp 中新增 **BufferUsage** 枚举（位掩码），至少包含 **Uniform** 位；BufferDesc.usage 保持 `uint32_t`，语义为 BufferUsage 位掩码。CreateBuffer 当 usage 含 Uniform 时，须创建可用于 constant buffer 的缓冲（各后端使用 host-visible 或 upload heap 等等价策略）。

**Rationale**: ABI TODO 要求 BufferDesc.usage 含 Uniform 用途位；枚举位掩码与 Vulkan/D3D12/Metal 的 usage 语义一致，便于后端映射。

**Alternatives considered**:
- 仅常量 kBufferUsageUniform：可少一个枚举类型，但扩展性差；采用枚举便于后续 Vertex、Index、Storage 等统一。

---

## 4. 与 009-RenderCore 描述符对接

**Decision**: 008-RHI 的 BufferDesc/TextureDesc 保持当前定义；009 产出的描述须**可转换为** te::rhi::BufferDesc/TextureDesc（字段一一对应或简单转换）。不在 008 侧增加 009 专用类型；若 009 契约中定义共用结构，则 008 可引用同一结构或提供转换函数。本 feature 仅保证 008 侧 BufferDesc.usage 含 Uniform、UpdateBuffer、SetUniformBuffer 可用。

**Rationale**: spec 要求“类型转换或字段一一对应”；008 为下游提供稳定 ABI，009 适配 008 更符合依赖方向。

---

## 5. 其余（与 008-rhi-fullmodule-005 一致）

四后端真实实现、禁止 stub/no-op、依赖与构建、描述符集与 SwapChain 策略与 005 的 research 一致，不重复。
