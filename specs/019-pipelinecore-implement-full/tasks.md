# Tasks: 019-PipelineCore 完整模块实现

**Input**: Design documents from `specs/019-pipelinecore-implement-full/`  
**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/019-pipelinecore-ABI-full.md

**Organization**: Tasks grouped by user story for independent implementation and testing.

## Format: `[ID] [P?] [Story?] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: User story label (US1, US2, US3, US4)
- Include exact file paths in descriptions

## Path Conventions

- **Repository root**: `TenEngine-019-pipeline-core/` (worktree)
- **Headers**: `include/te/pipelinecore/*.h`
- **Sources**: `src/*.cpp`
- **Tests**: `tests/unit/`, `tests/integration/`

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Project initialization and directory structure

- [x] T001 Create directory structure per plan: `include/te/pipelinecore/`, `src/`, `tests/unit/`, `tests/integration/` at worktree root
- [x] T002 Create CMakeLists.txt skeleton: project(te_pipelinecore), C++17, add_library(te_pipelinecore), target_include_directories(PUBLIC include), target_link_libraries(te_rhi te_rendercore). **⚠️ 执行 cmake -B build 前须已澄清构建根目录**（worktree 根）；各依赖（008-RHI、009-RenderCore）按**源码**引入；规约见 `docs/engine-build-module-convention.md` §3。cmake 生成后须检查：头文件/源文件是否完整、是否存在循环依赖或缺失依赖。

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Config and shared types required by all user stories

**⚠️ CRITICAL**: No user story work can begin until this phase is complete

- [x] T003 [P] Implement Config.h: kMaxFramesInFlight, PipelineConfig, FrameSlotId in `include/te/pipelinecore/Config.h`
- [x] T004 [P] Implement FrameContext.h: FrameContext struct (scene, camera, viewport, frameSlotId), ViewportDesc in `include/te/pipelinecore/FrameContext.h`
- [x] T005 [P] Declare ISceneWorld minimal interface in `include/te/pipelinecore/FrameGraph.h` (virtual ~ISceneWorld = default; 020/004 实现)
- [x] T006 Add Config.h, FrameContext.h to build; verify include paths and te::pipelinecore namespace

**Checkpoint**: Foundational types ready - user story implementation can begin

---

## Phase 3: User Story 1 - FrameGraph 构建与 Pass 执行 (Priority: P1) 🎯 MVP

**Goal**: IFrameGraph::AddPass、IPassBuilder 配置、Compile、Pass 按拓扑顺序执行；PassContext::GetCollectedObjects 返回收集到的物体

**Independent Test**: 创建 FrameGraph、AddPass 若干、Compile，验证执行顺序与 PassContext；Mock IRenderObjectList 验证回调被正确调用

### Implementation for User Story 1

- [x] T007 [US1] Implement CullMode, RenderType, PassOutputDesc in `include/te/pipelinecore/FrameGraph.h`
- [x] T008 [US1] Implement IPassBuilder interface and SetScene/SetCullMode/SetObjectTypeFilter/SetRenderType/SetOutput/SetExecuteCallback in `include/te/pipelinecore/FrameGraph.h`
- [x] T009 [US1] Implement PassContext, PassExecuteCallback, IRenderObjectList in `include/te/pipelinecore/FrameGraph.h`
- [x] T010 [US1] Implement IFrameGraph interface (AddPass, Compile) in `include/te/pipelinecore/FrameGraph.h`
- [x] T011 [US1] Implement FrameGraphImpl and AddPass/Compile logic in `src/FrameGraph.cpp`; Compile 含环检测、拓扑排序
- [x] T012 [US1] Add FrameGraph.cpp to CMakeLists.txt; 验证 build 成功
- [x] T013 [US1] Unit test: AddPass returns IPassBuilder, Compile returns true/false (含环), PassContext.GetCollectedObjects in `tests/unit/test_framegraph.cpp`（调用本模块接口，可 Mock 009/RHI）

**Checkpoint**: User Story 1 complete - FrameGraph 可独立测试

---

## Phase 4: User Story 2 - 逻辑管线与多线程收集 RenderItem (Priority: P1)

**Goal**: BuildLogicalPipeline、CollectRenderItemsParallel、MergeRenderItems、PrepareRenderResources、ConvertToLogicalCommandBuffer；线程 D 约束

**Independent Test**: Mock FrameContext/场景，验证 BuildLogicalPipeline、Collect、Merge 产出；线程 D 验证 Prepare 与 Convert 仅用 RHI 已声明 API

### Implementation for User Story 2

- [x] T014 [US2] Implement RenderItem struct, IRenderItemList, CreateRenderItem in `include/te/pipelinecore/RenderItem.h` and `src/RenderItem.cpp`
- [x] T015 [US2] Implement ILogicalPipeline in `include/te/pipelinecore/LogicalPipeline.h`
- [x] T016 [US2] Implement BuildLogicalPipeline in `src/LogicalPipeline.cpp`; 产出 Pass 列表与每 Pass 收集配置
- [x] T017 [US2] Implement CollectRenderItemsParallel, MergeRenderItems in `include/te/pipelinecore/CollectPass.h` and `src/CollectPass.cpp`
- [x] T018 [US2] Implement PrepareRenderResources, PrepareRenderMaterial, PrepareRenderMesh (return te::rendercore::ResultCode) in `src/RenderItem.cpp`; 仅用 IDevice 已声明 API；文档化线程 D 约束
- [x] T019 [US2] Implement ILogicalCommandBuffer, ConvertToLogicalCommandBuffer (ResultCode + out param), CollectCommandBuffer alias in `include/te/pipelinecore/LogicalCommandBuffer.h` and `src/LogicalCommandBuffer.cpp`; 格式符合 pipeline-to-rci.md
- [x] T020 [US2] Add LogicalPipeline.cpp, CollectPass.cpp, RenderItem.cpp, LogicalCommandBuffer.cpp to CMakeLists.txt; 验证 build
- [x] T021 [US2] Integration test: BuildLogicalPipeline → CollectRenderItemsParallel → MergeRenderItems → PrepareRenderResources → ConvertToLogicalCommandBuffer in `tests/integration/test_logical_pipeline.cpp`；主动调用 te_rhi、te_rendercore API 验证依赖链

**Checkpoint**: User Story 2 complete - 多线程管线阶段可独立测试

---

## Phase 5: User Story 3 - 流水线在途帧与 Slot 配置 (Priority: P2)

**Goal**: kMaxFramesInFlight、PipelineConfig、FrameSlotId 符合 ABI；FrameSlotId 有效范围 [0, frameInFlightCount)

**Independent Test**: 引用 kMaxFramesInFlight、PipelineConfig；验证 FrameSlotId 范围

### Implementation for User Story 3

- [x] T022 [US3] Validate Config.h 中 kMaxFramesInFlight、PipelineConfig、FrameSlotId 符合 ABI；确保 FrameContext.frameSlotId 类型与范围正确
- [x] T023 [US3] Document slot 语义、与 RHI waitForSlot/getCommandListForSlot 的协同在 `quickstart.md` 或注释中

**Checkpoint**: User Story 3 complete - Slot 配置可验证

---

## Phase 6: User Story 4 - RDG 风格资源声明与生命周期 (Priority: P2)

**Goal**: DeclareRead/DeclareWrite 与 009 PassProtocol 对接；Compile 后执行顺序与资源声明一致；瞬态资源 ReleaseAfterPass

**Independent Test**: DeclareRead/DeclareWrite 记录读边；Compile 后执行顺序正确；瞬态资源屏障符合 RHI

### Implementation for User Story 4

- [x] T024 [US4] Integrate 009-RenderCore DeclareRead、DeclareWrite、PassHandle、ResourceHandle 到 Pass 图构建；在 IPassBuilder 或扩展 API 中支持资源声明
- [x] T025 [US4] Extend Compile() to resolve dependency graph from DeclareRead/DeclareWrite；拓扑排序考虑资源读写边
- [x] T026 [US4] Implement resource lifetime (Transient, ReleaseAfterPass) 与 RHI ResourceBarrier 协同；首版可简化

**Checkpoint**: User Story 4 complete - RDG 资源声明可验证

---

## Phase 7: Polish & Cross-Cutting Concerns

**Purpose**: Profiling, documentation, validation

- [x] T027 [P] Implement Profiling.h: TE_PIPELINECORE_PROFILING, PassProfilingScope, OnCompileProfiling in `include/te/pipelinecore/Profiling.h`
- [x] T028 Integrate profiling hooks into Compile() and Pass execution (macro/config controlled)
- [x] T029 [P] Update quickstart.md with complete example; add thread D constraint documentation
- [x] T030 Run quickstart.md validation; verify all ABI symbols compile and link

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 1 (Setup)**: No dependencies - start immediately
- **Phase 2 (Foundational)**: Depends on Phase 1 - BLOCKS all user stories
- **Phase 3 (US1)**: Depends on Phase 2
- **Phase 4 (US2)**: Depends on Phase 2, US1 (BuildLogicalPipeline 需要 IFrameGraph)
- **Phase 5 (US3)**: Depends on Phase 2
- **Phase 6 (US4)**: Depends on Phase 3 (Compile 扩展)
- **Phase 7 (Polish)**: Depends on Phase 3, 4

### User Story Dependencies

| Story | Depends On | Can Run Parallel With |
|-------|------------|------------------------|
| US1 | Foundational | — |
| US2 | Foundational, US1 | US3 |
| US3 | Foundational | US2 |
| US4 | US1 | — |

### Parallel Opportunities

- T003, T004, T005 can run in parallel (Phase 2)
- T007–T010 can run in parallel within US1 (same header, sequential for impl)
- T014, T015 can run in parallel (US2)
- T027, T029 can run in parallel (Phase 7)

---

## Implementation Strategy

### MVP First (US1 Only)

1. Phase 1: Setup
2. Phase 2: Foundational
3. Phase 3: User Story 1
4. **STOP and VALIDATE**: Test FrameGraph AddPass/Compile/PassContext
5. Deploy/demo if ready

### Incremental Delivery

1. Setup + Foundational → Foundation ready
2. US1 → FrameGraph 可独立测试 (MVP)
3. US2 → 多线程管线完整
4. US3 → Slot 配置验证
5. US4 → RDG 资源声明
6. Polish → Profiling、文档

---

## Notes

- 所有实现仅使用 008-rhi-public-api、009-rendercore-public-api 已声明类型与 API
- IMaterialHandle、IMeshHandle 为前向声明；020/011/012 提供具体类型
- PrepareRenderResources、ConvertToLogicalCommandBuffer 遇 RHI 失败须返回 ResultCode
- 本 feature 无第三方依赖；无需 FetchContent 或第三方集成任务
