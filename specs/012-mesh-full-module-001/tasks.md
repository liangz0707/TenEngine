# Tasks: 012-Mesh 完整模块

**Input**: Design documents from `specs/012-mesh-full-module-001/`  
**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/

**Tests**: 测试覆盖本模块对外接口及与上游（008/009/013）的调用链；测试可执行文件只 link 本模块 target，测试代码主动调用上游 API 以验证依赖集成。见 TenEngine 规约。

**Organization**: 按用户故事分组，便于独立实现与验证。

## Format: `[ID] [P?] [Story] Description`

- **[P]**: 可并行（不同文件、无未完成依赖）
- **[Story]**: 所属用户故事（US1～US5）
- 描述中包含具体文件路径

## Path Conventions

- 仓库根为 worktree 根（如 `G:\AIHUMAN\WorkSpaceSDD\TenEngine-012-mesh`）
- 头文件：`include/te/mesh/`
- 实现：`src/mesh/`
- 测试：`tests/unit/`、`tests/integration/`、`tests/contract/`

---

## Phase 1: Setup（共享基础设施）

**Purpose**: 工程初始化与目录结构；CMake 配置依赖 001/008/009/013 源码引入。

- [x] T001 Create directory structure per plan: `include/te/mesh/`, `src/mesh/`, `tests/unit/`, `tests/integration/`, `tests/contract/` at repo root
- [x] T002 Add top-level or module CMakeLists.txt for 012-mesh target; declare dependency on 001-core, 008-rhi, 009-rendercore, 013-resource via **源码**（add_subdirectory/FetchContent 或 TenEngineHelpers）。**构建前必须澄清**：在哪个目录执行构建（构建根目录）；未澄清前**禁止**直接执行 cmake，须先向用户确认。规约见 `docs/engine-build-module-convention.md` §3
- [x] T003 [P] Add include path for `include/te/mesh/` and link 001-core, 008-rhi, 009-rendercore, 013-resource in CMake; ensure no circular dependency. **cmake 生成之后须检查**：引入的头文件/源文件是否完整、是否存在循环依赖或缺失依赖；有问题须标注或先修复再继续

---

## Phase 2: Foundational（阻塞性前置）

**Purpose**: 全量 ABI 所需的基础类型与头文件，所有 User Story 依赖此阶段。

**⚠️ CRITICAL**: 未完成本阶段前不得开始任意 User Story 的实现

- [x] T004 [P] Define MeshHandle (opaque), SubmeshDesc, LODLevel, SkinningData in `include/te/mesh/Mesh.h` per plan 全量 ABI；仅声明类型与访问器签名，实现可留空
- [x] T005 [P] Define MeshAssetDesc (formatVersion, debugDescription, vertexLayout, vertexData, indexData, indexFormat, submeshes, optional LOD/skinning) in `include/te/mesh/MeshAssetDesc.h` per data-model.md and plan
- [x] T006 Declare CreateMesh, ReleaseMesh in `include/te/mesh/MeshFactory.h`; GetSubmeshCount, GetSubmesh, GetLODCount, SelectLOD, GetSkinningData in `include/te/mesh/Mesh.h`; EnsureDeviceResources, GetVertexBufferHandle, GetIndexBufferHandle in `include/te/mesh/MeshDevice.h` per plan 全量 ABI 表

**Checkpoint**: 类型与声明就绪，可按 User Story 实现

---

## Phase 3: User Story 1 - 从内存数据创建网格 (Priority: P1) 🎯 MVP

**Goal**: 013 解析 .mesh 后调用 012 CreateMesh 得到 MeshHandle；可查询 SubmeshCount、GetSubmesh、顶点/索引元数据。

**Independent Test**: 给定合法 MeshAssetDesc，调用 CreateMesh 得到非空 MeshHandle；GetSubmeshCount、GetSubmesh(i) 与入参一致；无效入参返回空句柄或错误且不崩溃。

### Implementation for User Story 1

- [x] T007 [US1] Implement CreateMesh(MeshAssetDesc const* desc) in `src/mesh/MeshFactory.cpp`; 仅接受内存数据，校验 desc 非空与 submeshes 范围，返回 MeshHandle（内部存储 submeshes 副本或引用）
- [x] T008 [US1] Implement ReleaseMesh(MeshHandle h) in `src/mesh/MeshFactory.cpp`; 释放句柄与内部资源，不调用 008 DestroyBuffer（DResource 由 US2 创建，此处仅释放 CPU 侧）
- [x] T009 [US1] Implement GetSubmeshCount(MeshHandle h), GetSubmesh(MeshHandle h, uint32_t index) in `src/mesh/Mesh.cpp` or MeshFactory.cpp；index >= SubmeshCount 时返回 nullptr 或契约约定行为
- [x] T010 [US1] Implement MeshDeserializer::Deserialize(void const* buffer, size_t size) in `src/mesh/MeshDeserializer.cpp`; 产出 MeshAssetDesc*（opaque payload），013 不解析；对应头文件 `include/te/mesh/MeshDeserializer.h`
- [x] T011 [US1] Implement MeshResourceLoader::CreateFromPayload(ResourceType type, void* payload, IResourceManager* manager) in `src/mesh/MeshLoader.cpp`; type==Mesh 时将 payload 转为 MeshAssetDesc*，调用 CreateMesh(desc)，包装为 IResource*（实现 IResource/IMeshResource）返回；对应头文件 `include/te/mesh/MeshLoader.h`

**Checkpoint**: US1 可独立验证：Deserialize → CreateFromPayload → CreateMesh → GetSubmeshCount/GetSubmesh

---

## Phase 4: User Story 2 - 确保设备侧顶点/索引缓冲 (Priority: P1)

**Goal**: 对 MeshHandle 调用 EnsureDeviceResources(handle, device)；012 调用 008-RHI CreateBuffer 创建顶点/索引缓冲；幂等。

**Independent Test**: 在 008/009 已初始化前提下，CreateMesh 后调用 EnsureDeviceResources，再调用 GetVertexBufferHandle/GetIndexBufferHandle 得到有效句柄；重复 Ensure 不重复创建、不泄漏。

### Implementation for User Story 2

- [x] T012 [US2] Implement EnsureDeviceResources(MeshHandle h, IDevice* device) in `src/mesh/MeshDevice.cpp`; 对依赖链先 Ensure（若需），再调用 008 CreateBuffer 创建顶点/索引缓冲并填入 handle 内部；device 无效时返回 false 或不崩溃
- [x] T013 [US2] Implement GetVertexBufferHandle(MeshHandle h), GetIndexBufferHandle(MeshHandle h) in `src/mesh/MeshDevice.cpp`; EnsureDeviceResources 后可用；类型与 008 契约一致
- [x] T014 [US2] In ReleaseMesh (T008), 增加对已创建 DResource 的释放：调用 008 DestroyBuffer 释放顶点/索引缓冲（若 EnsureDeviceResources 曾被调用）

**Checkpoint**: US2 可独立验证：CreateMesh → EnsureDeviceResources → GetVertexBufferHandle/GetIndexBufferHandle → ReleaseMesh 无泄漏

---

## Phase 5: User Story 3 - LOD 选择与流式请求 (Priority: P2)

**Goal**: GetLODCount、SelectLOD(handle, distanceOrScreenSize)；可选 RequestStreaming 与 013 对接。

**Independent Test**: 多 LOD 网格 GetLODCount > 1；SelectLOD 返回与契约策略一致的 LOD 索引；StreamingRequest 与 013 句柄对接。

### Implementation for User Story 3

- [x] T015 [US3] Implement GetLODCount(MeshHandle h), SelectLOD(MeshHandle h, float distanceOrScreenSize) in `src/mesh/Mesh.cpp` or MeshLOD.cpp；LOD 数据来自 MeshAssetDesc 可选字段；策略与契约一致
- [x] T016 [US3] [P] Add optional RequestStreaming or LOD 流式接口 in `include/te/mesh/Mesh.h` and `src/mesh/Mesh.cpp`；与 013 RequestStreaming/StreamingHandle 对接（若 013 契约有明确接口）

**Checkpoint**: US3 可独立验证：多 LOD 网格 SelectLOD 与 GetLODCount 正确

---

## Phase 6: User Story 4 - 子网格与 DrawCall 批次 (Priority: P2)

**Goal**: SubmeshDesc 含 materialSlotIndex；Pipeline 可通过 GetSubmesh 得到偏移、数量、材质槽位以生成 DrawCall 批次。

**Independent Test**: 多子网格 MeshHandle，GetSubmesh(i) 返回的 SubmeshDesc 与 CreateMesh 入参一致，materialSlotIndex 可用；无效索引行为符合契约。

### Implementation for User Story 4

- [x] T017 [US4] Ensure SubmeshDesc in `include/te/mesh/Mesh.h` includes offset, count, materialSlotIndex per plan and data-model; 实现 GetSubmesh 已返回该结构（T009）；若无则补全并修正 MeshFactory 中 submeshes 的填充
- [x] T018 [US4] Add unit test in `tests/unit/test_mesh_submesh.cpp` that creates mesh with multiple submeshes, calls GetSubmesh for each index and validates SubmeshDesc and materialSlotIndex; 测试须调用本模块对外接口（可依赖 001 分配等上游能力）

**Checkpoint**: US4 可独立验证：多子网格 DrawCall 批次数据正确

---

## Phase 7: User Story 5 - 蒙皮数据与 Animation 对接 (Priority: P3)

**Goal**: MeshAssetDesc 可选蒙皮数据；GetSkinningData(handle) 返回 BoneIndices、Weights、BindPose；无蒙皮返回 nullptr。

**Independent Test**: 含蒙皮 CreateMesh 入参，GetSkinningData 返回非空且数据一致；无蒙皮网格 GetSkinningData 返回 nullptr，不崩溃。

### Implementation for User Story 5

- [x] T019 [US5] [P] Ensure SkinningData and optional skinning in MeshAssetDesc are defined in `include/te/mesh/Mesh.h` and `include/te/mesh/MeshAssetDesc.h` per plan; CreateMesh 接受并存储可选蒙皮数据
- [x] T020 [US5] Implement GetSkinningData(MeshHandle h) in `src/mesh/Mesh.cpp`; 无蒙皮返回 nullptr；有蒙皮返回与 015-Animation 骨骼约定一致的数据视图

**Checkpoint**: US5 可独立验证：带蒙皮/无蒙皮网格行为符合契约

---

## Phase 8: Polish & Cross-Cutting

**Purpose**: Save 产出、002 注册、测试与文档。

- [x] T021 [P] Implement SerializeMeshToBuffer(MeshHandle h, void* buffer, size_t* size) (or equivalent) in `src/mesh/MeshSerialize.cpp`; 从 handle 产出 .mesh 布局内存供 013 Save 写盘；头文件 `include/te/mesh/MeshSerialize.h`
- [x] T022 Register MeshAssetDesc and .mesh format with 002-Object per data-model（与 002 注册描述类型）；一目录一资源（.mesh + .meshdata）在文档或注册中说明
- [x] T023 Add unit tests in `tests/unit/` for CreateMesh, ReleaseMesh, GetSubmeshCount, GetSubmesh, GetLODCount, SelectLOD, GetSkinningData; 测试须通过调用本模块公开 API 并（若可能）调用上游 001/008/009 类型以验证依赖链，见 `docs/engine-build-module-convention.md` 与 speckit.tasks 测试逻辑
- [x] T024 Add integration test in `tests/integration/` for 013 Load path: 013 反序列化 .mesh 后调 012 Loader CreateFromPayload，再 EnsureDeviceResources；验证与 013 的对接
- [x] T025 Run quickstart.md validation: 按 `specs/012-mesh-full-module-001/quickstart.md` 步骤验证「通过 013 加载 Mesh」与「直接使用 012 API」两条路径

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 1 (Setup)**: 无依赖；可立即开始
- **Phase 2 (Foundational)**: 依赖 Phase 1；**阻塞**所有 User Story
- **Phase 3～7 (US1～US5)**: 依赖 Phase 2；US1/US2 为 P1 建议先做，US3/US4 可并行或按 P2 顺序，US5 为 P3
- **Phase 8 (Polish)**: 依赖 Phase 3～7 中需交付的 Story 完成

### User Story Dependencies

- **US1 (P1)**: 仅依赖 Foundational；无其他 Story 依赖
- **US2 (P1)**: 依赖 US1（CreateMesh/ReleaseMesh 存在后再做 EnsureDeviceResources）
- **US3 (P2)**: 依赖 US1（MeshHandle 与 LOD 数据）
- **US4 (P2)**: 依赖 US1（GetSubmesh 已在 US1）
- **US5 (P3)**: 依赖 US1（MeshAssetDesc 与 CreateMesh 支持蒙皮字段）

### Parallel Opportunities

- T004, T005 可并行；T003 与 T001 无冲突时可与 T002 错开
- 同一 Phase 内标 [P] 的任务可并行
- Phase 2 完成后，US1 与 US2 顺序执行（US2 依赖 US1）；US3、US4、US5 可在 US1 完成后视情况并行
- T021、T023 在 Phase 8 内可并行

---

## Parallel Example: Phase 2

```text
T004: Define MeshHandle, SubmeshDesc, LODLevel, SkinningData in include/te/mesh/Mesh.h
T005: Define MeshAssetDesc in include/te/mesh/MeshAssetDesc.h
```

---

## Implementation Strategy

### MVP First (US1 + US2)

1. Phase 1: Setup（含 CMake 与依赖澄清）
2. Phase 2: Foundational（类型与声明）
3. Phase 3: US1（CreateMesh, ReleaseMesh, GetSubmesh, Loader, Deserializer）
4. Phase 4: US2（EnsureDeviceResources, GetVertexBufferHandle/GetIndexBufferHandle）
5. **STOP and VALIDATE**: 单元测试与简单集成（013 Load → CreateMesh → Ensure → Release）

### Incremental Delivery

1. Setup + Foundational → 基础就绪
2. US1 → 可独立测试「从内存创建 + 子网格查询」
3. US2 → 可独立测试「设备缓冲创建与获取」
4. US3 → LOD；US4 → 子网格/DrawCall 完善；US5 → 蒙皮
5. Polish → SerializeMeshToBuffer、002 注册、quickstart 验证

### Notes

- 构建根目录未澄清前**禁止**直接执行 cmake；执行后须检查头文件/依赖完整性。
- 本 feature **无第三方依赖**，无需 7 步第三方任务。
- 全量 ABI 见 `specs/012-mesh-full-module-001/plan.md`「全量 ABI 内容（实现参考）」；实现须覆盖该表全部符号与能力。
