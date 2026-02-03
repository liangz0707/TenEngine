# Tasks: 009-RenderCore 完整模块与 Shader Reflection 对接

**Input**: Design documents from `specs/009-render-core-shader-reflection-abi/`  
**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/009-rendercore-ABI-full.md

**Organization**: Tasks grouped by user story. Implementation reference: `contracts/009-rendercore-ABI-full.md`（全量 ABI）。

## Format: `[ID] [P?] [Story] Description`

- **[P]**: 可并行执行（不同文件，无依赖）
- **[Story]**: 所属 User Story（US1, US2, US3, US4）

## Path Conventions

- **Headers**: `include/te/rendercore/`（对外 ABI 头）
- **Sources**: `src/render_core/`
- **Tests**: `tests/unit/`, `tests/contract/`

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: 项目结构与构建根目录澄清

- [x] T001 **澄清构建根目录**：确认 worktree 路径（如 `TenEngine-009-render-core`）为构建根；out-of-source 构建至 `build/`。**执行 cmake 前必须完成**。规约见 `docs/engine-build-module-convention.md` §3
- [x] T002 配置 CMake：更新根 `CMakeLists.txt`，project 名为 `te_rendercore`，target 名为 `te_rendercore`；`target_link_libraries` 使用 `te_rhi`（非 TenEngine_RHI）
- [x] T003 配置 008-RHI 依赖：通过 `add_subdirectory` 引入 008-RHI 源码；008-RHI 不存在时 `message(FATAL_ERROR ...)`，禁止占位 interface 库作为长期方案

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: 头文件布局、命名空间、基础类型；所有 User Story 依赖此阶段

**⚠️ CRITICAL**: 本阶段完成后才能开始 User Story 实现

- [x] T004 [P] 创建 `include/te/rendercore/types.hpp`：ResultCode、PassHandle、ResourceHandle、FrameSlotId、ResourceLifetime、BindSlot；命名空间 `te::rendercore`
- [x] T005 [P] 创建 `include/te/rendercore/resource_desc.hpp`：VertexAttributeFormat、VertexAttribute、VertexFormat、VertexFormatDesc、IndexType、IndexFormat、IndexFormatDesc、TextureFormat、TextureUsage、TextureDesc、TextureDescParams、BufferUsage、BufferDesc、BufferDescParams 及 Create* 函数声明
- [x] T006 [P] 创建 `include/te/rendercore/uniform_layout.hpp`：UniformMemberType、UniformMember、UniformLayoutDesc、IUniformLayout 接口（GetOffset、GetTotalSize）、CreateUniformLayout、ReleaseUniformLayout 声明
- [x] T007 [P] 创建 `include/te/rendercore/pass_protocol.hpp`：PassResourceDecl、DeclareRead、DeclareWrite、SetResourceLifetime 声明
- [x] T008 [P] 创建 `include/te/rendercore/uniform_buffer.hpp`：IUniformBuffer 接口（Update、Bind、GetRingBufferOffset、SetCurrentFrameSlot）、CreateUniformBuffer、ReleaseUniformBuffer 声明；依赖 `te/rhi/device.hpp`、`te/rhi/command_list.hpp`
- [x] T009 创建 `include/te/rendercore/api.hpp`：聚合包含以上所有头；供下游 `#include <te/rendercore/api.hpp>`
- [x] T010 更新 CMake：`target_include_directories` 暴露 `include/`；头文件加入 `add_library` 与 `source_group` 以便 IDE 可见；移除对不存在源文件的引用（如 ResourceDesc.cpp、UniformLayout.cpp）

**Checkpoint**: 头文件布局与 ABI 结构就绪

---

## Phase 3: User Story 1 - 资源描述创建 (Priority: P1) 🎯 MVP

**Goal**: 管线或材质系统创建顶点/索引/纹理/缓冲描述，传递给 RHI 创建接口

**Independent Test**: 调用 CreateVertexFormat/CreateIndexFormat/CreateTextureDesc/CreateBufferDesc，合法参数返回 IsValid 描述；非法参数返回 invalid

### Implementation for User Story 1

- [x] T011 [US1] 实现 `src/render_core/resource_desc.cpp`：CreateVertexFormat、CreateIndexFormat、CreateTextureDesc、CreateBufferDesc；非法输入（nullptr、Unknown format、零尺寸）返回 invalid；命名空间 `te::rendercore`；include `<te/rendercore/resource_desc.hpp>`
- [x] T012 [US1] 确保 VertexFormat/IndexFormat/TextureDesc/BufferDesc 含 IsValid()；与 data-model 验证规则一致

**Checkpoint**: US1 可独立测试；Create* 返回合法或 invalid 描述

---

## Phase 4: User Story 2 - Uniform 布局与 Shader 反射对接 (Priority: P1)

**Goal**: 从手写或 010-Shader GetReflection 产出 UniformLayoutDesc 创建 IUniformLayout；GetOffset、GetTotalSize；与 010-Shader 格式对齐

**Independent Test**: 手写 UniformLayoutDesc 调用 CreateUniformLayout 得非空 IUniformLayout；GetOffset("memberName") 返回正确偏移；不存在的 name 返回 0；GetTotalSize 满足 std140

### Implementation for User Story 2

- [x] T013 [US2] 实现 `src/render_core/uniform_layout.cpp`（或合并自 shader_params.cpp）：CreateUniformLayout、ReleaseUniformLayout；IUniformLayout 实现类；命名空间 `te::rendercore`
- [x] T014 [US2] 实现 GetOffset(name)：按名称查找成员返回 offset；未找到返回 0
- [x] T015 [US2] 实现 GetTotalSize：按 std140 规则计算或使用 desc.totalSize（非 0 时）；研究见 `research.md`
- [x] T016 [US2] UniformMemberType 为 Unknown 时 CreateUniformLayout 返回 nullptr；members 为空或 nullptr 时返回 nullptr
- [x] T017 [US2] 文档化 010-Shader 反射格式约定：UniformMember 结构与 UniformMemberType 映射；写入 data-model 或 quickstart

**Checkpoint**: US2 可独立测试；手写 UniformLayoutDesc 创建 layout 成功；格式与 010-Shader 契约约定一致

---

## Phase 5: User Story 3 - Pass 资源声明 (Priority: P2)

**Goal**: DeclareRead、DeclareWrite、SetResourceLifetime；与 PipelineCore RDG 对接

**Independent Test**: DeclareRead(pass, resource) 产生 isRead=true 的 PassResourceDecl；SetResourceLifetime(decl, Persistent) 设置 decl.lifetime

### Implementation for User Story 3

- [x] T018 [US3] 实现 `src/render_core/pass_protocol.cpp`：DeclareRead、DeclareWrite、SetResourceLifetime；PassResourceDecl 填充 pass、resource、isRead、isWrite、lifetime；命名空间 `te::rendercore`

**Checkpoint**: US3 可独立测试

---

## Phase 6: User Story 4 - UniformBuffer 创建、更新与绑定 (Priority: P1)

**Goal**: CreateUniformBuffer、Update、Bind、GetRingBufferOffset、SetCurrentFrameSlot；直接调用 008-RHI CreateBuffer(Uniform)、UpdateBuffer、SetUniformBuffer

**Independent Test**: 有效 layout 与 te::rhi::IDevice* 调用 CreateUniformBuffer 得非空 IUniformBuffer；Update 写入数据；Bind 调用 RHI SetUniformBuffer；layout 或 device 为 nullptr 时返回 nullptr

### Implementation for User Story 4

- [x] T019 [US4] 实现 `src/render_core/uniform_buffer.cpp`：CreateUniformBuffer 调用 te::rhi::IDevice::CreateBuffer(BufferUsage::Uniform)；layout 或 device 为 nullptr 时返回 nullptr
- [x] T020 [US4] 实现 IUniformBuffer::Update：调用 008-RHI UpdateBuffer，写入当前帧 slot；禁止 no-op
- [x] T021 [US4] 实现 IUniformBuffer::Bind：调用 te::rhi::ICommandList::SetUniformBuffer；直接使用 te::rhi 类型，禁止 reinterpret_cast
- [x] T022 [US4] 实现 GetRingBufferOffset、SetCurrentFrameSlot、ReleaseUniformBuffer；ReleaseUniformBuffer(nullptr) 为 no-op

**Checkpoint**: US4 可独立测试；与 008-RHI 契约对接

---

## Phase 7: Tests & Validation

**Purpose**: 单元测试与契约测试；覆盖上游 RHI 调用（TenEngine 测试规约）

- [x] T023 [P] 更新 `tests/unit/test_render_core.cpp`：include `<te/rendercore/api.hpp>`；`using namespace te::rendercore`；覆盖 ResourceDesc、UniformLayout、PassProtocol、UniformBuffer 的 Create* 与验证逻辑
- [x] T024 [P] 更新 `tests/contract/test_rhi_integration.cpp`：include `<te/rendercore/api.hpp>`；当 008-RHI 可用时，调用 te::rhi::IDevice 创建 Buffer/Texture，验证 ResourceDesc 产出的描述可被 RHI 接受；验证 CreateUniformBuffer + Update + Bind 链
- [x] T025 更新 `tests/CMakeLists.txt`：target_link_libraries 使用 `te_rendercore`；若存在 contract 测试，链接 `te_rhi` 以验证对接

---

## Phase 8: Polish & ABI TODO

**Purpose**: 契约更新、构建验证、文档

- [x] T026 更新 `specs/_contracts/009-rendercore-ABI.md`：将「TODO（010-Shader 反射对接）」中两复选框改为 - [x]；将该小节改为「已实现」或移除
- [x] T027 运行 `cmake -B build` 并构建；确认无循环依赖、缺失 include；头文件在 IDE 中可见
- [x] T028 [P] 更新 `README.md`：`#include <te/rendercore/api.hpp>`、`te::rendercore` 命名空间、`te_rendercore` target 说明
- [x] T029 运行 quickstart.md 中的示例验证流程

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 1 (Setup)**: 无依赖
- **Phase 2 (Foundational)**: 依赖 Phase 1；**阻塞所有 User Story**
- **Phase 3–6 (User Stories)**: 依赖 Phase 2；US1/US2/US4 可并行；US3 依赖较少
- **Phase 7 (Tests)**: 依赖 Phase 3–6
- **Phase 8 (Polish)**: 依赖 Phase 7

### User Story Dependencies

- **US1 (P1)**: 仅依赖 Phase 2
- **US2 (P1)**: 仅依赖 Phase 2
- **US3 (P2)**: 仅依赖 Phase 2
- **US4 (P1)**: 依赖 Phase 2；需 IUniformLayout（US2）

### Parallel Opportunities

- T004–T008 可并行（不同头文件）
- T011–T012 与 T013–T017 可并行（resource_desc vs uniform_layout）
- T023–T024 可并行
- T028 可与 T026、T027 并行

---

## Parallel Example: User Story 1 & 2

```bash
# US1 与 US2 实现可并行（不同源文件）：
Task: "实现 src/render_core/resource_desc.cpp"
Task: "实现 src/render_core/uniform_layout.cpp"
```

---

## Implementation Strategy

### MVP First (User Story 1)

1. Phase 1: Setup
2. Phase 2: Foundational
3. Phase 3: US1 ResourceDesc
4. **STOP & VALIDATE**: 单元测试 US1

### Incremental Delivery

1. Setup + Foundational → 头文件与 CMake 就绪
2. US1 → 独立测试 → MVP
3. US2 UniformLayout → Shader Reflection 对接
4. US4 UniformBuffer → 与 RHI 对接
5. US3 PassProtocol → RDG 协议
6. Tests + Polish → 完成

### CMake 注意事项

- 执行 `cmake -B build` **前**必须确认**构建根目录**（worktree 路径）
- 各子模块均使用**源码方式**引入；008-RHI 缺失时须 `FATAL_ERROR`
- cmake 生成后检查：include 路径、source_group、依赖链是否完整

---

## Notes

- 实现以 `contracts/009-rendercore-ABI-full.md` 全量 ABI 为准
- 仅使用 001-Core、008-RHI 契约已声明类型与 API
- 无第三方依赖，无 7 步第三方任务
