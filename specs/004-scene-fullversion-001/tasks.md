# Tasks: 004-Scene 完整功能集

**Branch**: 004-scene-fullversion-001  
**Input**: plan.md, spec.md, specs/_contracts/004-scene-public-api.md  
**Prerequisites**: plan.md (required), spec.md (required), research.md, data-model.md  

**Constraint**: 任务只暴露契约（`specs/_contracts/004-scene-public-api.md`）与 plan.md「契约更新（API 雏形）」已声明的类型与 API；不新增契约外的对外接口。

**Organization**: 按用户故事分阶段；每阶段可独立验收。

## Format: `[ID] [P?] [Story?] Description`

- **[P]**: 可并行（不同文件、无未完成依赖）
- **[USn]**: 用户故事标签（US1–US5）
- 描述中含具体文件路径

---

## Phase 1: Setup（共享基础设施）

**Purpose**: 工程初始化与目录结构，C++17、CMake

- [ ] T001 Create CMake project for 004-Scene (C++17) at repository root or under engine, per plan.md structure
- [ ] T002 Add 001-Core and 002-Object as dependencies in CMake; ensure only contract-declared types/API are used
- [ ] T003 [P] Create directory layout: src/scene/, tests/scene/unit/, tests/scene/integration/ per plan.md

---

## Phase 2: Foundational（阻塞性前置）

**Purpose**: 契约已声明的类型与单 World 容器；所有用户故事依赖此阶段完成。

**Constraint**: 仅声明契约中的类型（WorldRef, NodeId, Transform, HierarchyIterator, Active, LevelHandle）；不暴露契约外 API。

- [ ] T004 [P] Declare WorldRef, NodeId, LevelHandle (opaque handles) in src/scene/types.hpp aligned with specs/_contracts/004-scene-public-api.md
- [ ] T005 [P] Declare Transform type (or forward-use Core.Math type per 001-core-public-api) in src/scene/types.hpp; no extra public fields beyond contract
- [ ] T006 [P] Declare HierarchyIterator (single-use, move-only) in src/scene/hierarchy.hpp; contract: single-use, invalid after traverse ends
- [ ] T007 Implement single World container and GetCurrentWorld() in src/scene/world.hpp/.cpp; no AddWorld/SetActiveWorld yet—Phase 5
- [ ] T008 Provide internal node storage and identity (NodeId) for one World in src/scene/scene_graph.hpp/.cpp; no public CreateNode until Phase 3

**Checkpoint**: WorldRef/NodeId/Transform/HierarchyIterator/Active/LevelHandle 类型就绪；GetCurrentWorld 可用；内部可创建节点但不暴露契约外 API。

---

## Phase 3: User Story 1 - 场景图与变换层级 (P1) — MVP

**Goal**: 在 World 下构建场景图、父子关系、Local/World 变换、SetDirty、UpdateTransforms；仅暴露契约中的场景图 API。

**Independent Test**: 创建 World、多级节点、设置 Parent/Children 与 LocalTransform，SetDirty 后 UpdateTransforms，验证 WorldTransform 与层级一致；SetParent 成环/非法层级返回错误。

**Constraint**: 对外仅暴露契约与 plan 雏形中的 API：CreateNode, SetParent, GetParent, GetChildren, GetLocalTransform, SetLocalTransform, GetWorldTransform, SetDirty, UpdateTransforms。

- [ ] T009 [US1] Implement CreateNode(WorldRef) in src/scene/scene_graph.hpp/.cpp; return NodeId; contract-only API
- [ ] T010 [US1] Implement SetParent(NodeId, NodeId) in src/scene/scene_graph.hpp/.cpp; return false on cycle/invalid hierarchy, no scene change; contract-only
- [ ] T011 [US1] Implement GetParent(NodeId), GetChildren(NodeId, ...) in src/scene/scene_graph.hpp/.cpp; contract-only API
- [ ] T012 [US1] Implement GetLocalTransform/SetLocalTransform, GetWorldTransform in src/scene/scene_graph.hpp/.cpp; Transform from Core.Math or types.hpp
- [ ] T013 [US1] Implement SetDirty(NodeId), UpdateTransforms(WorldRef) in src/scene/scene_graph.hpp/.cpp; dirty propagation and world transform update per contract
- [ ] T014 [US1] Add unit tests: create nodes, parent/children, local/world transform, SetDirty/UpdateTransforms, SetParent cycle rejection in tests/scene/unit/test_scene_graph.cpp

**Checkpoint**: US1 可独立验收；场景图与变换 API 仅限契约声明。

---

## Phase 4: User Story 2 - 层级遍历与按名/按类型查找 (P2)

**Goal**: Traverse, FindByName, FindByType, GetPath, GetId, SetActive, GetActive；HierarchyIterator 单次有效；仅暴露契约中的层级 API。

**Independent Test**: 在已有场景图上 Traverse/FindByName/FindByType，GetPath/GetId，SetActive/GetActive；验证迭代器单次有效。

**Constraint**: 对外仅暴露契约与 plan 雏形中的 API；HierarchyIterator 单次有效、不可复用。

- [ ] T015 [P] [US2] Implement Traverse(WorldRef, NodeId) returning HierarchyIterator in src/scene/hierarchy.hpp/.cpp; single-use semantics per contract
- [ ] T016 [P] [US2] Implement FindByName(WorldRef, NodeId, char const*), FindByType(WorldRef, NodeId, typeFilter) in src/scene/hierarchy.hpp/.cpp; return single-use iterator
- [ ] T017 [US2] Implement GetPath(NodeId, ...), GetId(HierarchyIterator) in src/scene/hierarchy.hpp/.cpp; contract-only; iterator invalid after traverse end
- [ ] T018 [US2] Implement SetActive(NodeId, bool), GetActive(NodeId) in src/scene/hierarchy.hpp/.cpp or scene_graph; contract-only API
- [ ] T019 [US2] Add unit tests: Traverse order, FindByName/FindByType, GetPath/GetId, SetActive/GetActive, iterator single-use in tests/scene/unit/test_hierarchy.cpp

**Checkpoint**: US2 可独立验收；层级 API 仅限契约声明；HierarchyIterator 单次有效。

---

## Phase 5: User Story 3 - 多 World 与当前活动场景 (P2)

**Goal**: AddWorld, SetActiveWorld, GetCurrentWorld；SetActiveWorld 下一帧或下一次 UpdateTransforms 生效；仅暴露契约中的 World API。

**Independent Test**: AddWorld、SetActiveWorld 后本帧 GetCurrentWorld 仍为旧 WorldRef，下一帧或下一次 UpdateTransforms 后为新 WorldRef。

**Constraint**: 对外仅暴露 GetCurrentWorld, SetActiveWorld, AddWorld；生效时机符合 spec 澄清。

- [ ] T020 [US3] Implement AddWorld() in src/scene/world.hpp/.cpp; return WorldRef; contract-only API
- [ ] T021 [US3] Implement SetActiveWorld(WorldRef) in src/scene/world.hpp/.cpp; pending switch applied at next frame or next UpdateTransforms per spec
- [ ] T022 [US3] Ensure GetCurrentWorld() returns pending-active World only after next UpdateTransforms (or frame boundary); update world.cpp logic
- [ ] T023 [US3] Add unit/integration tests: AddWorld, SetActiveWorld timing, GetCurrentWorld in tests/scene/unit/test_world.cpp or tests/scene/integration/

**Checkpoint**: US3 可独立验收；多 World 与当前活动场景语义符合契约与 spec。

---

## Phase 6: User Story 4 - Level 加载/卸载与 Resource 对接 (P3)

**Goal**: LoadLevel, UnloadLevel, Level 资源句柄；与 013-Resource 约定句柄与加载时机；仅暴露契约与 plan 中约定的 Level API。

**Independent Test**: 在与 013-Resource 约定后，LoadLevel/UnloadLevel 验证关卡边界与句柄生命周期。

**Constraint**: 对外仅暴露契约中的 LoadLevel、UnloadLevel、LevelHandle；签名与失败语义由 plan 与 013-Resource 对接约定；若尚未对接则实现占位或最小签名。

- [ ] T024 [US4] Declare LevelHandle and LoadLevel(WorldRef, ...), UnloadLevel(WorldRef, LevelHandle) in src/scene/level.hpp per contract and plan 雏形
- [ ] T025 [US4] Implement LoadLevel/UnloadLevel stub or minimal implementation in src/scene/level.cpp; integrate with 013-Resource when contract is agreed
- [ ] T026 [US4] Add tests for LoadLevel/UnloadLevel boundary and LevelHandle lifecycle in tests/scene/integration/ when 013-Resource interface is fixed

**Checkpoint**: US4 Level API 仅限契约声明；与 013-Resource 对接后补全实现与测试。

---

## Phase 7: User Story 5 - 与 Entity 根实体挂接 (P3)

**Goal**: 在当前（或指定）World 上挂接 Entity 根实体；接口归属与签名由 plan 与 005-Entity 约定；仅暴露契约与约定后的 API。

**Independent Test**: 在 plan 与 005-Entity 约定接口后，挂接根实体，下游可基于 WorldRef/NodeId 与 Entity 协作。

**Constraint**: 不新增契约与 plan 约定之外的挂接 API；若约定为 Scene 提供 AttachEntityRoot，则实现之；否则预留占位。

- [ ] T027 [US5] Define Entity root attach API in src/scene/world.hpp or entity_root.hpp per plan–005-Entity agreement; contract-only
- [ ] T028 [US5] Implement Entity root attach (stub or full) in src/scene/world.cpp or entity_root.cpp when 005-Entity contract is agreed
- [ ] T029 [US5] Add integration test for Entity root attach in tests/scene/integration/ when 005-Entity interface is fixed

**Checkpoint**: US5 挂接 API 仅限契约与 005-Entity 约定；无约定则占位不暴露额外 API。

---

## Phase 8: Polish & Cross-Cutting

**Purpose**: 对外头文件仅包含契约已声明的类型与 API；无契约外泄漏。

- [ ] T030 [P] Audit all public headers under src/scene/; remove or hide any API not in specs/_contracts/004-scene-public-api.md and plan.md API 雏形
- [ ] T031 Add quickstart validation: build, run minimal CreateNode/UpdateTransforms/GetCurrentWorld flow per specs/004-scene-fullversion-001/quickstart.md
- [ ] T032 [P] Update README or docs for 004-Scene build and usage, referencing contract and quickstart

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 1 (Setup)**: 无依赖，可立即开始。
- **Phase 2 (Foundational)**: 依赖 Phase 1；**阻塞**所有用户故事。
- **Phase 3–7 (US1–US5)**: 依赖 Phase 2；US1 先做（MVP），US2/US3 可并行于 US1 之后，US4/US5 可并行于 US2/US3 之后（注意 US3 依赖 World 多实例）。
- **Phase 8 (Polish)**: 依赖 Phase 3–7 完成。

### User Story Dependencies

- **US1 (P1)**: 仅依赖 Phase 2；无其他故事依赖。
- **US2 (P2)**: 依赖 Phase 2、US1（场景图存在才有遍历/查找）。
- **US3 (P2)**: 依赖 Phase 2；与 US1 共享 World 类型，逻辑上可与 US1 并行但建议 US1 先完成。
- **US4 (P3)**: 依赖 Phase 2、US3（多 World）；与 013-Resource 约定后实现。
- **US5 (P3)**: 依赖 Phase 2、US3；与 005-Entity 约定后实现。

### Parallel Opportunities

- T004–T006、T015–T016、T030 与 T032 可并行（不同文件或独立审计）。
- Phase 4 与 Phase 5 在 Phase 3 完成后可由不同人并行（Hierarchy vs World）。
- 同一 Phase 内标 [P] 的任务可并行。

---

## Implementation Strategy

### MVP First (US1 only)

1. Phase 1 → Phase 2 → Phase 3  
2. 验收：CreateNode、SetParent、GetParent/GetChildren、Local/WorldTransform、SetDirty、UpdateTransforms、成环拒绝。  
3. 对外仅契约 API；可在此停步交付 MVP。

### Incremental Delivery

1. Setup + Foundational → 类型与单 World 就绪。  
2. US1 → 场景图与变换可测。  
3. US2 → 层级与激活可测。  
4. US3 → 多 World 与当前活动场景可测。  
5. US4/US5 → 与 013/005 对接后补全 Level 与 Entity 根挂接。  
6. Polish → 契约审计与 quickstart 验证。

---

## Notes

- 所有实现任务以 `specs/_contracts/004-scene-public-api.md` 与 plan.md「契约更新（API 雏形）」为准；不新增对外类型或函数。  
- [P] 表示可并行；[USn] 表示归属用户故事。  
- 每阶段完成后可单独验收该故事；Phase 8 确认无契约外 API 泄漏。
