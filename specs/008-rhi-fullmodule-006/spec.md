# Feature Specification: 008-RHI 完整模块实现（含 ABI TODO）

**Feature Branch**: `008-rhi-fullmodule-006`  
**Created**: 2026-01-31  
**Status**: Draft  
**Input**: 本 feature 的完整模块规约见 `docs/module-specs/008-rhi.md`，契约见 `specs/_contracts/008-rhi-public-api.md`；**本 feature 实现完整模块内容**，并**额外实现 `specs/_contracts/008-rhi-ABI.md` 中「TODO」部分所列能力**。

## 规约与契约引用 *(模块切片时必填)*

- **完整模块规约**：`docs/module-specs/008-rhi.md`（RHI 图形 API 抽象：设备、命令列表、资源、PSO、同步、多后端、与 Core 上游依赖）。
- **本模块范围**（本 feature 实现完整模块内容）：
  1. **Device**：CreateDevice、GetQueue、GetFeatures、GetLimits、SelectBackend、DestroyDevice；多后端（Vulkan、D3D12、Metal、D3D11）统一接口，编译期宏选择实现路径。
  2. **CommandList**：Begin/End、Draw、DrawIndexed、Dispatch、Copy、ResourceBarrier、SetViewport、SetScissor、BeginRenderPass/EndRenderPass、CopyBuffer/CopyBufferToTexture/CopyTextureToBuffer、BuildAccelerationStructure/DispatchRays（光追占位）；Submit(ICommandList*, IQueue*)，可选 Fence/Semaphore 重载。
  3. **Resources**：CreateBuffer、CreateTexture、CreateSampler、CreateView、DestroyBuffer/DestroyTexture/DestroySampler；内存与生命周期由契约约定。
  4. **PSO**：CreateGraphicsPSO、CreateComputePSO、SetShader、Cache、DestroyPSO；与 Shader 字节码绑定。
  5. **Synchronization**：CreateFence(bool)、CreateSemaphore、Wait、Signal、DestroyFence、DestroySemaphore；资源屏障在 ICommandList::ResourceBarrier。
  6. **SwapChain**：CreateSwapChain、Present、GetCurrentBackBuffer、Resize（契约与 ABI 已列）。
  7. **描述符集（P2）**：CreateDescriptorSetLayout、AllocateDescriptorSet、UpdateDescriptorSet、DestroyDescriptorSetLayout/DestroyDescriptorSet；各后端可为占位。
  8. **光追**：类型与接口占位；仅 D3D12 后端在 SDK 支持时可实现，其余 no-op。

- **ABI TODO 实现范围**（见 `specs/_contracts/008-rhi-ABI.md` 末尾「TODO」节；本 feature 须一并实现）：
  1. **Buffer 的 CPU 写入**：支持将 CPU 数据写入已创建的 IBuffer（满足下游 009-RenderCore 的 UniformBuffer::Update）。形式之一：IBuffer::Map/Unmap，或 IDevice::UpdateBuffer(IBuffer* buf, size_t offset, void const* data, size_t size)。
  2. **Uniform 绑定到命令列表 slot**：支持将某 IBuffer（及可选 offset）绑定到 ICommandList 的某 slot，供后续 Draw/Dispatch 使用（满足 IUniformBuffer::Bind）。形式之一：ICommandList::SetUniformBuffer(uint32_t slot, IBuffer* buffer, size_t offset)，或通过 DescriptorSet（UpdateDescriptorSet + ICommandList::BindDescriptorSet 等）完成绑定。
  3. **BufferDesc 的 Uniform 用途**：BufferDesc.usage 须包含 Uniform 用途位（或等价语义），供 CreateUniformBuffer 等调用 CreateBuffer 时使用；若 008-RHI 尚未定义，须在 BufferDesc/usage 枚举中补充。
  4. **描述符与 RHI 的对接**：009-RenderCore 产出的 VertexFormat、IndexFormat、TextureDesc、BufferDesc 需能与 008-RHI 的顶点/纹理/缓冲创建参数对接（类型转换或字段一一对应）；若 RHI 使用 te::rhi::BufferDesc 等，需与 009 约定共用描述结构或提供转换。

实现时只使用**本 feature 依赖的上游契约**（`specs/_contracts/001-core-public-api.md`）中已声明的类型与 API；不实现本规约未列出的能力。

- **ABI 与构建规约**（`.specify/memory/constitution.md` §VI）：本模块须实现其 **ABI 文件**（`specs/_contracts/008-rhi-ABI.md`）中的**全部**符号与能力（含上述 TODO 转为正式 ABI 后的条目）；构建须通过**引入真实子模块源码**（如 CMake `add_subdirectory`）满足依赖，**禁止**使用 stub、mock 或与契约不一致的代替实现作为长期方案。**契约更新**：接口变更须在对应 **ABI 文件**中**增补或替换**对应的 ABI 条目；下游所需接口须在**上游模块的 ABI 文件**中以 **TODO** 登记。详见 `specs/_contracts/README.md`「契约更新流程」。

- **第三方依赖**：第三方库引入说明在契约 `specs/_contracts/008-rhi-public-api.md` 中声明（「第三方依赖」小节）；本 spec 引用该契约即可，不在 spec 中重复列出。

## User Scenarios & Testing *(mandatory)*

### User Story 1 - 创建设备并获取队列 (Priority: P1)

上层（如 Pipeline 或 Editor）需要在不依赖具体图形 API 的前提下创建设备、选择后端、获取图形/计算队列，并查询特性与限制。

**Why this priority**: 所有渲染路径的前提；无设备则无法创建资源或提交命令。

**Independent Test**: 调用 CreateDevice(Backend)、GetQueue、GetFeatures、GetLimits、DestroyDevice，不崩溃且返回一致；不支持的 Backend 返回 nullptr。

**Acceptance Scenarios**:

1. **Given** 未初始化 RHI，**When** 调用 SelectBackend(Vulkan) 后 CreateDevice()，**Then** 返回非空 IDevice 且 GetBackend() 为 Vulkan（当 Vulkan 可用时）。
2. **Given** 已创建设备，**When** GetQueue(Graphics, 0)，**Then** 返回非空 IQueue；GetQueue(Graphics, 1) 或 GetQueue(Compute, 0) 可按实现返回 nullptr。
3. **Given** 不支持的 Backend，**When** CreateDevice(该 Backend)，**Then** 返回 nullptr，无自动回退。

---

### User Story 2 - 录制并提交命令 (Priority: P1)

上层需要分配命令列表、录制 Draw/Dispatch/Copy/ResourceBarrier、可选 SetViewport/SetScissor/BeginRenderPass/EndRenderPass，并提交到队列；可选通过 Fence 等待完成。

**Why this priority**: 单帧渲染与多帧流水线的核心路径。

**Independent Test**: Begin → Draw/Dispatch/ResourceBarrier → End → Submit(cmd, queue) 或 Submit(cmd, queue, fence, ...)；fence->Wait() 不崩溃；DestroyCommandList/DestroyDevice 正确释放。

**Acceptance Scenarios**:

1. **Given** 有效 IDevice 与 IQueue，**When** CreateCommandList、Begin、Draw(3,1)、End、Submit(cmd, queue)，**Then** 无崩溃，命令语义由后端实现。
2. **Given** 同上，**When** Submit(cmd, queue, signalFence, nullptr, nullptr) 后 signalFence->Wait()，**Then** 等待语义由实现定义，不崩溃。
3. **Given** 已 Begin 的 cmd，**When** SetViewport/SetScissor/BeginRenderPass（若契约支持），**Then** 行为与契约及 ABI 一致。

---

### User Story 3 - 创建与管理 GPU 资源 (Priority: P1)

上层需要创建 Buffer、Texture、Sampler 与视图（CreateView），并在使用完毕后销毁；失败时得到明确反馈（如 nullptr）。

**Why this priority**: 无资源则无法绑定到 PSO 或执行拷贝。

**Independent Test**: CreateBuffer/CreateTexture/CreateSampler 合法描述符返回非空；CreateBuffer(size=0) 或 CreateTexture(width=0) 返回 nullptr；Destroy 后不再使用句柄。

**Acceptance Scenarios**:

1. **Given** 有效 IDevice，**When** CreateBuffer(BufferDesc{size>0})，**Then** 返回非空 IBuffer；DestroyBuffer 后该指针不再解引用。
2. **Given** 有效 IDevice，**When** CreateTexture(TextureDesc{width,height>0})、CreateSampler(SamplerDesc)，**Then** 返回非空 ITexture、ISampler；DestroyTexture/DestroySampler 正确释放。
3. **Given** 有效资源与 ViewDesc，**When** CreateView(desc)，**Then** 返回 ViewHandle，用于绑定（语义见契约）。

---

### User Story 4 - PSO 与 Shader 绑定 (Priority: P2)

上层需要创建图形/计算 PSO、传入 Shader 字节码（SetShader）、可选 Cache，并在绘制前绑定；销毁时 DestroyPSO。

**Why this priority**: 绘制与 Dispatch 依赖 PSO；与 010-Shader 对接。

**Independent Test**: CreateGraphicsPSO/CreateComputePSO 合法描述符返回非空 IPSO；SetShader、Cache 不崩溃；无效或空 Shader 可返回 nullptr。

**Acceptance Scenarios**:

1. **Given** 有效 IDevice 与顶点 Shader 数据，**When** CreateGraphicsPSO(desc)、SetShader(pso, data, size)、Cache(pso)，**Then** 返回非空 IPSO（或后端拒绝时 nullptr），无崩溃。
2. **Given** 有效 IDevice，**When** CreateComputePSO(desc)，**Then** 返回非空 IPSO；DestroyPSO 后不再使用。

---

### User Story 5 - 同步与资源屏障 (Priority: P2)

上层需要创建 Fence（可选初始已 signal）、Semaphore，在提交后 Wait(Fence)，以及通过 ICommandList::ResourceBarrier 声明资源状态转换。

**Why this priority**: 多帧流水线与多队列同步依赖；资源屏障保证读写顺序。

**Independent Test**: CreateFence(false/true)、CreateSemaphore、Wait、Signal、DestroyFence/DestroySemaphore 不崩溃；ResourceBarrier(0,nullptr,0,nullptr) 为合法 no-op。

**Acceptance Scenarios**:

1. **Given** 有效 IDevice，**When** CreateFence(false)、Signal(fence)、Wait(fence)、DestroyFence(fence)，**Then** 行为由实现定义，不崩溃。
2. **Given** 已录制的 cmd，**When** ResourceBarrier(bufferCount, bufferBarriers, textureCount, textureBarriers)，**Then** 屏障语义与契约及后端一致（D3D11 可为 no-op）。

---

### User Story 6 - SwapChain 与描述符集（P2） (Priority: P3)

上层需要创建 SwapChain（可选窗口句柄）、Present、GetCurrentBackBuffer、Resize；以及描述符集布局与分配、UpdateDescriptorSet（各后端可为占位）。

**Why this priority**: 呈现与绑定管线所需；占位满足接口完整性。

**Independent Test**: CreateSwapChain(desc) 返回非空 ISwapChain（或 stub）；Present、Resize 不崩溃；CreateDescriptorSetLayout/AllocateDescriptorSet 可返回 nullptr 或占位实现。

**Acceptance Scenarios**:

1. **Given** 有效 IDevice 与 SwapChainDesc（width/height>0），**When** CreateSwapChain(desc)、Present()、Resize(w,h)，**Then** 行为与契约一致；GetCurrentBackBuffer 可为 nullptr（无窗口时）。
2. **Given** 有效 IDevice，**When** CreateDescriptorSetLayout(desc)、AllocateDescriptorSet(layout)、UpdateDescriptorSet(set, writes, n)，**Then** 按契约与 ABI 实现或占位；Destroy* 正确释放。

---

### User Story 7 - Buffer CPU 写入与 Uniform 绑定（ABI TODO） (Priority: P1)

上层（009-RenderCore）需要将 CPU 数据写入已创建的 IBuffer（UniformBuffer::Update），并将该 IBuffer 绑定到命令列表的某 slot 供 Draw/Dispatch 使用（IUniformBuffer::Bind）；BufferDesc 须支持 Uniform 用途位。

**Why this priority**: 下游 009 的 UniformBuffer 更新与绑定依赖本模块提供上述能力；无则 009 无法实现。

**Independent Test**: CreateBuffer(BufferDesc{usage 含 Uniform})；通过 Map/Unmap 或 UpdateBuffer 写入数据；在已 Begin 的 ICommandList 上调用 SetUniformBuffer(slot, buffer, offset) 或等价 Bind；Draw/Dispatch 后数据对 Shader 可见（或由集成测试验证）。

**Acceptance Scenarios**:

1. **Given** 有效 IDevice 与 BufferDesc（usage 含 Uniform 用途），**When** CreateBuffer(desc)、UpdateBuffer(buf, 0, data, size) 或 Map/写/Unmap，**Then** 写入成功且后续 Draw/Dispatch 可访问该数据（或由后端保证一致性）。
2. **Given** 有效 ICommandList（已 Begin）与 IBuffer，**When** SetUniformBuffer(slot, buffer, offset) 或 BindDescriptorSet(…)，**Then** 绑定成功；同一 slot 可被后续绘制使用。
3. **Given** BufferDesc，**When** usage 包含 Uniform 位，**Then** CreateBuffer 接受该描述符并创建可用于 Uniform 的缓冲。

---

### User Story 8 - 描述符与 009-RenderCore 对接（ABI TODO） (Priority: P2)

009-RenderCore 产出的 VertexFormat、IndexFormat、TextureDesc、BufferDesc 需能与 008-RHI 的顶点/纹理/缓冲创建参数对接；类型可转换或字段一一对应。

**Why this priority**: 管线与渲染资源准备依赖 009 与 008 的类型一致或可转换。

**Independent Test**: 009 提供的 BufferDesc/TextureDesc 或等价结构可转换为 te::rhi::BufferDesc/TextureDesc 并成功调用 CreateBuffer/CreateTexture；或双方约定共用描述结构。

**Acceptance Scenarios**:

1. **Given** 009 产出的缓冲/纹理描述，**When** 转换为 RHI 的 BufferDesc/TextureDesc（或直接使用约定结构），**Then** CreateBuffer/CreateTexture 成功且语义一致。
2. **Given** 顶点/索引格式约定，**When** 009 与 008 使用同一枚举或映射表，**Then** 顶点缓冲创建与 Draw 参数一致。

---

### Edge Cases

- 后端不可用时 CreateDevice 返回 nullptr；上层负责回退或重试。
- 资源销毁顺序：先释放依赖该资源的命令或 PSO，再 DestroyBuffer/DestroyTexture/DestroySampler。
- 多线程行为由实现定义并文档化；本契约不要求默认并发安全。
- 设备丢失或运行时错误可上报；支持回退或重建，不导致引擎崩溃。
- Buffer 写入（Map/UpdateBuffer）与命令提交的同步由上层或实现约定；避免写后未提交即读。
- Uniform 绑定 slot 越界或 nullptr buffer 时行为由契约或 ABI 约定（no-op 或错误码）。

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: 系统必须提供 CreateDevice(Backend)、CreateDevice()、DestroyDevice(IDevice*)，且不支持的 Backend 返回 nullptr。
- **FR-002**: 系统必须提供 GetQueue(QueueType, index)、GetFeatures()、GetLimits()，返回与设备一致的队列与特性/限制。
- **FR-003**: 系统必须提供 CreateCommandList、Begin/End、Draw/DrawIndexed/Dispatch、Copy、ResourceBarrier、SetViewport/SetScissor、BeginRenderPass/EndRenderPass、CopyBuffer/CopyBufferToTexture/CopyTextureToBuffer（及光追占位）、Submit(cmd, queue) 与 Submit(cmd, queue, signalFence, waitSem, signalSem)。
- **FR-004**: 系统必须提供 CreateBuffer/CreateTexture/CreateSampler、CreateView、DestroyBuffer/DestroyTexture/DestroySampler；非法描述符（如 size=0）返回 nullptr。
- **FR-005**: 系统必须提供 CreateGraphicsPSO、CreateComputePSO、SetShader、Cache、DestroyPSO；无效 Shader 可返回 nullptr。
- **FR-006**: 系统必须提供 CreateFence(bool)、CreateSemaphore、Wait、Signal、DestroyFence、DestroySemaphore。
- **FR-007**: 实现仅使用 `specs/_contracts/001-core-public-api.md` 已声明的类型与 API；不暴露后端具体类型（Vulkan/Metal/D3D12 等）给下游。
- **FR-008**: 系统必须实现 `specs/_contracts/008-rhi-ABI.md` 中列出的全部符号与能力（含 TODO 转为正式 ABI 后的条目）；构建通过真实子模块源码满足依赖，禁止长期 stub/mock。
- **FR-009**（ABI TODO）：系统必须提供 Buffer 的 CPU 写入能力：IBuffer::Map/Unmap 或 IDevice::UpdateBuffer(IBuffer* buf, size_t offset, void const* data, size_t size)，使下游可实现 UniformBuffer::Update。
- **FR-010**（ABI TODO）：系统必须提供将 IBuffer 绑定到 ICommandList 某 slot 的能力：如 ICommandList::SetUniformBuffer(uint32_t slot, IBuffer* buffer, size_t offset) 或通过 DescriptorSet + BindDescriptorSet，使下游可实现 IUniformBuffer::Bind。
- **FR-011**（ABI TODO）：BufferDesc.usage 必须包含 Uniform 用途位（或等价枚举），供创建 Uniform 缓冲时使用。
- **FR-012**（ABI TODO）：008-RHI 的 BufferDesc/TextureDesc 等须与 009-RenderCore 产出的描述可对接（类型转换或共用结构），以便管线创建 RHI 资源。

### Key Entities

- **IDevice / IQueue / ICommandList**：设备、队列、命令列表抽象；生命周期见契约。
- **IBuffer / ITexture / ISampler / ViewHandle**：GPU 资源与视图；创建后直至显式 Destroy。
- **IPSO**：管线状态对象；与 Shader 字节码绑定，可缓存。
- **IFence / ISemaphore**：同步对象；多队列、跨帧同步。
- **ISwapChain / IDescriptorSetLayout / IDescriptorSet**：交换链与描述符集（P2）；各后端可为占位。
- **Buffer 写入与 Uniform 绑定**：Map/Unmap 或 UpdateBuffer；SetUniformBuffer(slot, buffer, offset) 或 BindDescriptorSet；BufferUsage 含 Uniform。

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: 在至少一种可用后端（Vulkan 或 D3D12 或 Metal）下，用户可完成创建设备、获取队列、创建并提交一条空命令列表、销毁设备，全程无崩溃。
- **SC-002**: 用户可创建 Buffer/Texture/Sampler、创建 PSO（含 SetShader/Cache）、录制 Draw 或 Dispatch、提交并可选 Wait(Fence)，无崩溃且语义与契约一致。
- **SC-003**: 不支持的 Backend 调用 CreateDevice 时返回 nullptr，且不触发未定义行为。
- **SC-004**: 所有对外类型与 API 与 `specs/_contracts/008-rhi-public-api.md` 及 `specs/_contracts/008-rhi-ABI.md` 一致；实现不依赖未在上游契约中声明的 API。
- **SC-005**（ABI TODO）：用户可将 CPU 数据写入已创建的 IBuffer（通过 UpdateBuffer 或 Map/Unmap），并可将该 IBuffer 绑定到 ICommandList 的指定 slot；后续 Draw/Dispatch 可使用该数据，满足下游 009 UniformBuffer 更新与绑定需求。
- **SC-006**（ABI TODO）：BufferDesc.usage 支持 Uniform 用途；009-RenderCore 产出的描述可与 008-RHI 的 CreateBuffer/CreateTexture 参数对接（转换或共用类型）。

## Interface Contracts *(multi-agent sync)*

- **本模块实现的契约**: `specs/_contracts/008-rhi-public-api.md`；ABI 见 `specs/_contracts/008-rhi-ABI.md`（实现含 TODO 转为正式条目后的符号）。
- **本模块依赖的契约**: `specs/_contracts/001-core-public-api.md`（上游 001-Core）；依赖的上游 API 见该契约中已声明的类型与能力；实现时只使用该契约已声明的类型与 API。
- **ABI/构建**：须实现 ABI 中全部符号（含原 TODO 实现后写入 ABI 的条目）；构建须引入真实子模块代码，禁止长期使用 stub 或代替方案。接口变更须在 ABI 文件中更新；下游所需接口须在上游 ABI 中以 TODO 登记（constitution §VI、`specs/_contracts/README.md`）。

## Dependencies

- **001-Core**: `specs/_contracts/001-core-public-api.md`（内存、平台等；008-RHI 仅依赖 Core，见 `specs/_contracts/000-module-dependency-map.md`）。实现时只使用该契约已声明的类型与 API。
- **009-RenderCore**（对接约定）：009 产出的 VertexFormat、IndexFormat、TextureDesc、BufferDesc 需与 008-RHI 类型可对接；具体约定可在 009 契约或联合文档中说明，本 feature 实现 008 侧接口与 BufferDesc.usage 扩展、Buffer 写入与 Uniform 绑定能力。
