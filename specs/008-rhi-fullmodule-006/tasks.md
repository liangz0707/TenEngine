# Tasks: 008-RHI 完整模块实现（含 ABI TODO）

**Input**: Design documents from `specs/008-rhi-fullmodule-006/`  
**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/, quickstart.md  

**ABI 基准**: 实现须基于**全量 ABI 内容**（`contracts/008-rhi-ABI-full.md` = 现有 `specs/_contracts/008-rhi-ABI.md` + 本次新增/修改）。本 feature 在 008-rhi-fullmodule-005 基础上**增补** ABI TODO 部分：BufferUsage、UpdateBuffer、SetUniformBuffer、BufferDesc.usage 语义。若 005 未完成，须先按全量 ABI 实现原有符号，再完成本任务列表。

**Tests**: 测试须覆盖上游 001-core 与第三方库调用（volk/Vulkan、D3D11/D3D12、Metal）；测试代码主动调用 UpdateBuffer、SetUniformBuffer、CreateBuffer(BufferUsage::Uniform) 及本模块对外接口。

**Organization**: Tasks grouped by user story (US7–US8 为本 feature 增量)；Phase 1–2 为前置与 ABI 头文件增补，Phase 3–4 对应 US7/US8，Phase 5 为 Polish。

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: [US7], [US8] per spec.md（本 feature 增量故事）
- Include exact file paths. **CMake 任务**：执行前须已澄清**构建根目录**（worktree 路径）；各子模块均使用**源码**引入；cmake 生成后须检查头文件/源文件完整、循环依赖或缺失依赖，有问题须标注或先修复。

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: 确认 feature 设计产物与构建前提；构建根目录须在执行 CMake 前与用户澄清。

- [X] T001 Verify feature dir and contracts: `specs/008-rhi-fullmodule-006/contracts/008-rhi-ABI-full.md` 存在且含 BufferUsage、UpdateBuffer、SetUniformBuffer、BufferDesc.usage 语义；实现时以该全量 ABI 为参考
- [X] T002 Verify build root clarified: 若本 feature 需执行 `cmake -B build` 或配置/构建工程，**执行前须已与用户确认构建根目录**（在哪个模块路径执行构建）；各子模块均使用**源码**引入，规约见 `docs/engine-build-module-convention.md` §3。未澄清根目录时**禁止**直接执行 cmake，须先向用户询问

---

## Phase 2: Foundational (ABI 头文件增补)

**Purpose**: 在全量 ABI 基础上增补本次新增符号声明；阻塞 US7 实现。

**⚠️ CRITICAL**: US7 实现依赖本阶段完成

- [X] T003 [P] Add BufferUsage enum in `include/te/rhi/resources.hpp`: `enum class BufferUsage : uint32_t { Vertex = 1u << 0, Index = 1u << 1, Uniform = 1u << 2, Storage = 1u << 3, CopySrc = 1u << 4, CopyDst = 1u << 5 };` 或等价位掩码常量；见 `contracts/008-rhi-ABI-full.md`
- [X] T004 [P] Update BufferDesc.usage semantic in `include/te/rhi/resources.hpp`: BufferDesc.usage 保持 `uint32_t`，文档注释或类型别名标明为 BufferUsage 位掩码；含 Uniform 时表示可用于 Uniform 缓冲；见 plan.md 契约更新
- [X] T005 [P] Add IDevice::UpdateBuffer declaration in `include/te/rhi/device.hpp`: `void UpdateBuffer(IBuffer* buf, size_t offset, void const* data, size_t size) = 0;` 见 contracts/008-rhi-ABI-full.md
- [X] T006 [P] Add ICommandList::SetUniformBuffer declaration in `include/te/rhi/command_list.hpp`: `void SetUniformBuffer(uint32_t slot, IBuffer* buffer, size_t offset) = 0;` 见 contracts/008-rhi-ABI-full.md

**Checkpoint**: ABI 头文件增补完成；各后端可实现 UpdateBuffer 与 SetUniformBuffer

---

## Phase 3: User Story 7 - Buffer CPU 写入与 Uniform 绑定（ABI TODO）(Priority: P1)

**Goal**: IDevice::UpdateBuffer、ICommandList::SetUniformBuffer、BufferUsage::Uniform 在各后端真实实现；CreateBuffer(BufferDesc{usage 含 Uniform}) 接受并创建可用于 Uniform 的缓冲。

**Independent Test**: CreateBuffer(BufferDesc{usage 含 Uniform})；UpdateBuffer(buf, 0, data, size)；cmd->Begin()；cmd->SetUniformBuffer(slot, buffer, offset)；Draw/Dispatch 后数据对 Shader 可见（或由集成测试验证）。测试须调用上游/第三方 API。

### Implementation for User Story 7

- [X] T007 [P] [US7] Implement UpdateBuffer in `src/vulkan/device_vulkan.cpp`: 将 CPU 数据写入 VkBuffer（staging 或 host-visible 分配）；调用真实 Vulkan API（如 vkMapMemory/vkUnmapMemory 或 vkCmdUpdateBuffer）
- [X] T008 [P] [US7] Implement UpdateBuffer in `src/d3d12/device_d3d12.cpp`: 使用上传堆或 Map/Unmap 将 CPU 数据写入 ID3D12Resource；调用真实 D3D12 API
- [X] T009 [P] [US7] Implement UpdateBuffer in `src/d3d11/device_d3d11.cpp`: 使用 ID3D11DeviceContext::Map/Unmap 或 UpdateSubresource 将 CPU 数据写入 ID3D11Buffer；调用真实 D3D11 API
- [X] T010 [P] [US7] Implement UpdateBuffer in `src/metal/device_metal.mm`: 使用 MTLBuffer contents 或 MTLBlitCommandEncoder 将 CPU 数据写入 MTLBuffer；调用真实 Metal API
- [X] T011 [P] [US7] Implement SetUniformBuffer in `src/vulkan/device_vulkan.cpp`: 在录制中绑定 VkBuffer 到指定 slot（push constant 或 descriptor set）；供后续 Draw/Dispatch 使用
- [X] T012 [P] [US7] Implement SetUniformBuffer in `src/d3d12/device_d3d12.cpp`: 绑定 ID3D12Resource 到 root parameter slot（CBV）；供后续 Draw/Dispatch 使用
- [X] T013 [P] [US7] Implement SetUniformBuffer in `src/d3d11/device_d3d11.cpp`: 绑定 ID3D11Buffer 到 constant buffer slot（VSSetConstantBuffers/PSSetConstantBuffers/CSSetConstantBuffers）；供后续 Draw/Dispatch 使用
- [X] T014 [P] [US7] Implement SetUniformBuffer in `src/metal/device_metal.mm`: 绑定 MTLBuffer 到 vertex/fragment/compute 的 buffer 索引；供后续 draw/dispatch 使用
- [X] T015 [US7] Ensure CreateBuffer accepts BufferUsage::Uniform in all backends: 在 `src/vulkan/device_vulkan.cpp`、`src/d3d12/device_d3d12.cpp`、`src/d3d11/device_d3d11.cpp`、`src/metal/device_metal.mm` 中，当 BufferDesc.usage 含 Uniform 位时，创建可用于 constant buffer 的缓冲（host-visible 或 upload heap 等）；若当前 CreateBuffer 未区分 usage，须按 usage 选择内存类型或堆属性
- [X] T016 [US7] Add test `tests/buffer_update_uniform_bind.cpp`: CreateDevice；CreateBuffer(BufferDesc{size, BufferUsage::Uniform})；UpdateBuffer(buf, 0, data, size)；CreateCommandList；cmd->Begin()；cmd->SetUniformBuffer(0, buf, 0)；cmd->End()；Submit(cmd, queue)；验证不崩溃且可调用上游/第三方；覆盖已启用后端

**Checkpoint**: US7 independently testable；UpdateBuffer、SetUniformBuffer、BufferUsage::Uniform 在各后端可用

---

## Phase 4: User Story 8 - 描述符与 009-RenderCore 对接（ABI TODO）(Priority: P2)

**Goal**: 009 产出的 VertexFormat、IndexFormat、TextureDesc、BufferDesc 能与 008-RHI 的顶点/纹理/缓冲创建参数对接；类型可转换或字段一一对应。

**Independent Test**: 009 提供的 BufferDesc/TextureDesc 或等价结构可转换为 te::rhi::BufferDesc/TextureDesc 并成功调用 CreateBuffer/CreateTexture；或双方约定共用描述结构。

- [X] T017 [P] [US8] Document 009–008 descriptor convention: 在 `specs/008-rhi-fullmodule-006/` 或 `docs/` 中新增或更新文档，说明 009-RenderCore 产出的 BufferDesc/TextureDesc、VertexFormat、IndexFormat 与 008-RHI 的 te::rhi::BufferDesc、TextureDesc 的对应关系（字段一一对应或转换规则）；若 009 使用同一类型则注明共用头文件或命名空间
- [X] T018 [US8] Verify BufferDesc/TextureDesc ABI compatibility: 确认 `include/te/rhi/resources.hpp` 中 BufferDesc、TextureDesc 字段与 009 契约或约定一致（size、usage、width、height、depth、format 等）；若 009 需额外字段，在文档中标注扩展点或与 009 契约同步

**Checkpoint**: US8 对接约定明确；009 可基于文档或类型对接 008

---

## Phase 5: Polish & Cross-Cutting

**Purpose**: 契约写回、全量 ABI 校验、quickstart 验证

- [X] T019 Write back plan.md 契约更新 to `specs/_contracts/008-rhi-ABI.md`: 将 plan.md「契约更新」小节中的新增/修改条目增补到 ABI 对应节（BufferUsage、BufferDesc.usage 语义、IDevice::UpdateBuffer、ICommandList::SetUniformBuffer）；将原「TODO」节移除或改为“已实现”说明；见 plan.md 与 specs/_contracts/README.md
- [X] T020 [P] Verify full ABI coverage: 对照 `specs/008-rhi-fullmodule-006/contracts/008-rhi-ABI-full.md` 检查所有符号已实现（含原有 ABI + 本次新增）；缺失则标注或补实现
- [X] T021 Run quickstart.md validation: 在已澄清的构建根目录执行构建与测试；验证 tests/buffer_update_uniform_bind.cpp 及现有测试通过；验证上游 001-core 与第三方库调用有效

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 1 (Setup)**: 无依赖；T002 若涉及 cmake 须先澄清构建根目录
- **Phase 2 (Foundational)**: 依赖 Phase 1；阻塞 Phase 3
- **Phase 3 (US7)**: 依赖 Phase 2；可独立于 Phase 4
- **Phase 4 (US8)**: 可与 Phase 3 并行或在其后；主要为文档与约定
- **Phase 5 (Polish)**: 依赖 Phase 3、Phase 4 完成

### User Story Dependencies

- **US7 (P1)**: 依赖 Phase 2；无其他故事依赖 — **本 feature MVP 增量**
- **US8 (P2)**: 可与 US7 并行；依赖 Phase 2（类型已存在）

### Parallel Opportunities

- T003–T006（头文件增补）可并行
- T007–T010（UpdateBuffer 各后端）、T011–T014（SetUniformBuffer 各后端）可并行
- T017–T018（US8 文档）可与部分 US7 实现并行
- T019–T021 中 T020 可与 T019 并行（验证与写回）

---

## Implementation Strategy

### MVP First (User Story 7)

1. Complete Phase 1（确认 contracts、构建根目录）
2. Complete Phase 2（BufferUsage、UpdateBuffer、SetUniformBuffer 声明）
3. Complete Phase 3（UpdateBuffer/SetUniformBuffer 四后端 + CreateBuffer Uniform + 测试）
4. **STOP and VALIDATE**: 运行 tests/buffer_update_uniform_bind.cpp；验证 UpdateBuffer、SetUniformBuffer、BufferUsage::Uniform

### Incremental Delivery

1. Phase 1 + Phase 2 → ABI 头文件就绪  
2. Phase 3 (US7) → buffer_update_uniform_bind 测试通过（本 feature 核心）  
3. Phase 4 (US8) → 009–008 对接约定文档就绪  
4. Phase 5 → ABI 写回、全量校验、quickstart 通过  

### Notes

- 每项任务格式：`- [ ] Txxx [P?] [USn?] Description with file path`
- CMake/构建：执行前须已澄清构建根目录；各子模块源码引入；生成后检查头文件/源文件与依赖
- 测试须覆盖上游 001-core 与第三方（volk/Vulkan、D3D11/D3D12、Metal）实际调用
- 实现禁止长期 stub 或静默 no-op；UpdateBuffer/SetUniformBuffer 须调用各后端真实 API
