# Tasks: 013-Resource 完整模块实现

**Input**: Design documents from `specs/013-resource-fullmodule-001/`  
**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/（ABI 全量见 plan.md）

**Tests**: Spec 未显式要求 TDD/测试任务；本 tasks 仅在 Polish 阶段包含 quickstart 验收。若后续增加测试，须覆盖上游模块能力与 ABI 符号调用（见 TenEngine 测试规约）。

**Organization**: 按 User Story 分组，便于按故事独立实现与验收。

## Format: `[ID] [P?] [Story] Description`

- **[P]**: 可并行（不同文件、无未完成依赖）
- **[Story]**: 所属用户故事（US1–US4）；Setup/Foundational/Polish 无 Story 标签
- 描述中须包含**具体文件路径**

## Path Conventions

- 本模块：`include/te/resource/`、`src/`、`tests/` 位于 worktree 根（即 `TenEngine-013-resource`）；与 plan.md Project Structure 一致。

---

## Phase 1: Setup（共享基础设施）

**Purpose**: 工程初始化与目录结构

- [x] T001 Create project structure per plan: `include/te/resource/`, `src/`, `tests/unit/`, `tests/integration/` under worktree root
- [x] T002 Configure CMake and dependencies in `CMakeLists.txt`: 001-core、002-object、028-texture 以**源码**方式引入（TenEngineHelpers / tenengine_resolve_my_dependencies）。**执行前须已与用户澄清构建根目录**（在哪个 worktree 执行构建）；未澄清时**禁止**直接执行 cmake。**cmake 生成之后须检查**：引入的头文件/源文件是否完整、是否存在循环依赖或缺失依赖；发现问题须在任务中标注或先修复再继续。规约见 `docs/engine-build-module-convention.md` §3。

---

## Phase 2: Foundational（阻塞性前置）

**Purpose**: 所有 User Story 依赖的核心类型与接口，必须先完成才能实现任一故事。

**⚠️ CRITICAL**: 本阶段未完成前，不得开始 User Story 实现。

- [x] T003 [P] Add `ResourceType` enum and related types in `include/te/resource/ResourceTypes.h` per plan 全量 ABI（Texture, Mesh, Material, Model, Effect, Terrain, Shader, Audio, Custom 等）
- [x] T004 [P] Add `ResourceId` type and API in `include/te/resource/ResourceId.h`; implement in `src/ResourceId.cpp`（与 002 GUID 对接、全局唯一；见 data-model）
- [x] T005 [P] Add `LoadRequestId`, `LoadStatus`, `LoadResult`, `LoadCompleteCallback` in `include/te/resource/ResourceManager.h` per ABI
- [x] T006 [P] Add `IResource` interface in `include/te/resource/Resource.h`: `GetResourceType()`, `Release()`; 声明 `EnsureDeviceResources`/`EnsureDeviceResourcesAsync` 转发（由下游触发，013 转发给实现体）
- [x] T007 [P] Add concept/type headers `include/te/resource/FResource.h`, `include/te/resource/RResource.h`, `include/te/resource/DResource.h` per ABI（仅概念/文档化类型，013 不创建 DResource）
- [x] T008 [P] Declare `IResourceManager` in `include/te/resource/ResourceManager.h` with all methods from plan 全量 ABI: RequestLoadAsync, GetLoadStatus, GetLoadProgress, CancelLoad, GetCached, LoadSync, Unload, RequestStreaming, SetStreamingPriority, RegisterResourceLoader, RegisterDeserializer, RegisterImporter, Import, Save, ResolvePath
- [x] T009 [P] Add `IResourceLoader` in `include/te/resource/ResourceLoader.h`: `CreateFromPayload(ResourceType, void* payload, IResourceManager*)` per ABI
- [x] T010 [P] Add `IResourceImporter` in `include/te/resource/ResourceImporter.h` (DetectFormat, Convert, 产出描述/数据、Metadata、Dependencies) per ABI
- [x] T011 [P] Add `IDeserializer` in `include/te/resource/Deserializer.h`: `Deserialize(void const* buffer, size_t size)` returning opaque payload per ABI
- [x] T012 [P] Add type-view headers for downstream: `include/te/resource/TextureResource.h`, `MeshResource.h`, `MaterialResource.h`, `EffectResource.h`, `TerrainResource.h`（抽象接口或前向声明，由 028/012/011 等实现；013 仅返回 IResource*）
- [x] T013 Add `GetResourceManager()` declaration in `include/te/resource/ResourceManager.h` and stub/singleton accessor in `src/ResourceManager.cpp` (implementation in US1)

**Checkpoint**: 头文件与类型、接口声明就绪；可开始 US1 实现。

---

## Phase 3: User Story 1 - 通过统一接口按 ResourceType 加载资源 (Priority: P1) 🎯 MVP

**Goal**: 调用方通过 RequestLoadAsync(path, type, callback) 或 LoadSync(path, type) 加载任意类型资源；按 ResourceType 分发到已注册 Loader，返回 IResource* 并纳入缓存；同一 ResourceId 多次 Load 返回同一 IResource* 并增加引用计数。

**Independent Test**: 给定合法 path 与 ResourceType、已注册 Loader，LoadSync 或 RequestLoadAsync 得到有效 IResource*；GetCached(ResourceId) 可再次获取；无效 path 或未注册类型返回 nullptr/LoadResult::NotFound 或 Error；同一 ResourceId 再次 Load 返回同一指针。

### Implementation for User Story 1

- [x] T014 [US1] Implement resource cache (ResourceId → IResource* with refcount) in `src/ResourceManager.cpp`; ensure same ResourceId returns same pointer and increments refcount on Load
- [x] T015 [US1] Implement RegisterResourceLoader and RegisterDeserializer storage and dispatch by ResourceType in `src/ResourceManager.cpp`
- [x] T016 [US1] Implement LoadSync in `src/ResourceManager.cpp`: resolve path → read file (via 001-Core) → call registered Deserializer by ResourceType → get opaque payload → call registered Loader CreateFromPayload → insert cache → return IResource*; on cache hit return cached and addref; fail with nullptr on invalid path or unregistered type
- [x] T017 [US1] Implement dependency loading and cycle detection in Load path: build dependency set during load, detect cycle and return LoadResult::Error (or equivalent), no stub
- [x] T018 [US1] Implement RequestLoadAsync in `src/ResourceManager.cpp`: enqueue request, complete when root and recursive dependencies loaded; invoke LoadCompleteCallback once with IResource* and LoadResult; implement GetLoadStatus, GetLoadProgress, CancelLoad per ABI
- [x] T019 [US1] Implement GetResourceManager() singleton or subsystem-backed instance in `src/ResourceManager.cpp` and wire to cache/LoadSync/RequestLoadAsync
- [x] T020 [US1] Implement IResource::Release() refcount decrement and removal from cache when count reaches zero in `src/Resource.cpp` (or implementation class used by Loader-created resources)

**Checkpoint**: User Story 1 可独立验收：LoadSync/RequestLoadAsync、缓存、同一 ResourceId 同指针、引用计数、循环检测失败、回调一次。

---

## Phase 4: User Story 2 - 资源缓存与寻址 (Priority: P2)

**Goal**: GetCached(ResourceId) 仅查询缓存，未命中返回 nullptr 不触发加载；ResolvePath(ResourceId) 返回可解析路径或包内引用，支持多内容根与 Bundle。

**Independent Test**: 加载后 GetCached(ResourceId) 得同一 IResource*；未加载或已释放时 GetCached 返回 nullptr 且不触发加载；ResolvePath(ResourceId) 返回约定路径或包内引用。

### Implementation for User Story 2

- [x] T021 [US2] Ensure GetCached(ResourceId) in `src/ResourceManager.cpp` only queries cache and returns nullptr when miss without triggering load (behavior already implied by T014; verify and document)
- [x] T022 [US2] Implement ResolvePath(ResourceId) in `src/ResourceManager.cpp` (or dedicated addressing module): GUID → path; support content roots and BundleMapping; return nullptr when unresolved per ABI
- [x] T023 [US2] Implement RequestStreaming and SetStreamingPriority with StreamingHandle in `src/ResourceManager.cpp` per plan ABI (stub or minimal implementation if no streaming backend yet; interface present)

**Checkpoint**: User Story 2 可独立验收：GetCached 仅查缓存、ResolvePath 返回路径、Streaming 接口可用。

---

## Phase 5: User Story 3 - 卸载、释放与生命周期 (Priority: P2)

**Goal**: Unload(IResource*) 与 IResource::Release() 正确更新引用计数并回收；对已释放或无效句柄再次调用幂等；EnsureDeviceResources 由下游触发并转发给 IResource 实现，013 不调用 008。

**Independent Test**: LoadSync 得到 IResource* 后调用 Release/Unload，资源可回收；对同一句柄多次 Release 幂等；无悬空引用。

### Implementation for User Story 3

- [x] T024 [US3] Implement IResourceManager::Unload(IResource*) in `src/ResourceManager.cpp`: decrement refcount or remove from cache; coordinate with IResource::Release() so that Release and Unload are both idempotent when handle already released
- [x] T025 [US3] Implement IResource::EnsureDeviceResources / EnsureDeviceResourcesAsync forwarding in `include/te/resource/Resource.h` and implementation path: 013 forwards to IResource implementation only, 013 does not call 008-RHI or create DResource

**Checkpoint**: User Story 3 可独立验收：Release/Unload 回收、幂等、EnsureDeviceResources 仅转发。

---

## Phase 6: User Story 4 - 导入、序列化、Save 统一接口 (Priority: P3)

**Goal**: RegisterImporter、Import(path, type, out_metadata)；Save(IResource*, path)：各模块产出内存内容，013 调用统一写接口落盘；反序列化已通过 RegisterDeserializer 在 US1 接入。

**Independent Test**: Save(IResource*, path) 按 GetResourceType() 分发到对应模块取得内存内容，013 写盘；Import(path, type) 按 type 分发到已注册 IResourceImporter，产出描述/数据与元数据。

### Implementation for User Story 4

- [x] T026 [US4] Implement RegisterImporter storage and Import(path, type, out_metadata_or_null) dispatch by ResourceType in `src/ResourceManager.cpp` per ABI
- [x] T027 [US4] Implement Save(IResource*, path) in `src/ResourceManager.cpp`: call GetResourceType(), dispatch to module to get serializable memory content, 013 writes to path via 001-Core or unified write API; 013 does not parse *Desc; modules do not write files directly
- [x] T028 [US4] Add unified Serialize entry point if required by ABI (Deserialize path already covered by RegisterDeserializer in US1); document in `include/te/resource/ResourceManager.h` or `Deserializer.h`

**Checkpoint**: User Story 4 可独立验收：Import 按 type 分发、Save 各模块出内容 013 写盘。

---

## Phase 7: Polish & Cross-Cutting Concerns

**Purpose**: 文档、全量 ABI 核对与 quickstart 验收。

- [x] T029 [P] Verify all ABI symbols from plan.md「全量 ABI 内容」are present in `include/te/resource/` and `src/`: ResourceTypes.h, ResourceId.h, Resource.h, ResourceManager.h, ResourceLoader.h, ResourceImporter.h, Deserializer.h, FResource.h, RResource.h, DResource.h, type-view headers, GetResourceManager, LoadRequestId/LoadStatus/LoadResult/LoadCompleteCallback
- [x] T030 Run quickstart.md validation: 按 `specs/013-resource-fullmodule-001/quickstart.md` 的 5 分钟上手步骤与关键约定验证实现（GetResourceManager, LoadSync, RequestLoadAsync, GetCached, ResolvePath, Save）
- [x] T031 [P] Update module-level docs in `docs/` if needed to reflect 013 API (e.g. reference to 013-resource from pipeline/editor docs)

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 1 (Setup)**: 无依赖，可立即开始。
- **Phase 2 (Foundational)**: 依赖 Phase 1 完成；**阻塞**所有 User Story。
- **Phase 3 (US1)**: 依赖 Phase 2；无其他 Story 依赖。
- **Phase 4 (US2)**: 依赖 Phase 2；与 US1 共享缓存与 Manager，建议在 US1 完成后实现。
- **Phase 5 (US3)**: 依赖 Phase 2；与 US1 共享 Release/Unload，建议在 US1 完成后实现。
- **Phase 6 (US4)**: 依赖 Phase 2；Import/Save 依赖 Manager 与类型分发，建议在 US1 后实现。
- **Phase 7 (Polish)**: 依赖所有欲交付的 User Story 完成。

### User Story Dependencies

- **US1 (P1)**: 仅依赖 Foundational；MVP 范围。
- **US2 (P2)**: 依赖 Foundational；与 US1 共享缓存接口，建议 US1 完成后做。
- **US3 (P2)**: 依赖 Foundational；与 US1 共享 Release/Unload，建议 US1 完成后做。
- **US4 (P3)**: 依赖 Foundational；Import/Save 依赖 Manager，建议 US1 后做。

### Parallel Opportunities

- Phase 1: T001 与 T002 可顺序（结构先于 CMake）。
- Phase 2: T003–T012 均为 [P]，可并行；T013 依赖 T008 声明。
- Phase 3: T014–T020 存在顺序（缓存 → LoadSync → RequestLoadAsync → Release）。
- Phase 7: T029、T031 可并行；T030 建议最后执行。

### Parallel Example: User Story 1

Phase 2 完成后，US1 内部可分组并行意向（实际存在依赖则顺序执行）：
- 先 T014（缓存）→ T015（注册）→ T016（LoadSync）→ T017（循环检测）→ T018（RequestLoadAsync）→ T019（GetResourceManager 接线）→ T020（Release 实现）。

### Parallel Example: Phase 2 (Foundational)

可并行执行（不同头文件）：
- T003 ResourceTypes.h
- T004 ResourceId.h + ResourceId.cpp
- T005 LoadRequestId/LoadStatus/LoadResult/LoadCompleteCallback in ResourceManager.h
- T006 Resource.h (IResource)
- T007 FResource.h, RResource.h, DResource.h
- T008 IResourceManager 声明
- T009 ResourceLoader.h, T010 ResourceImporter.h, T011 Deserializer.h
- T012 TextureResource.h, MeshResource.h, MaterialResource.h, EffectResource.h, TerrainResource.h

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. 完成 Phase 1 Setup。
2. 完成 Phase 2 Foundational。
3. 完成 Phase 3 User Story 1。
4. **STOP and VALIDATE**: 按 spec 的 US1 Independent Test 与 Acceptance Scenarios 验收。
5. 可交付「统一加载 + 缓存 + 引用计数 + 循环检测 + 异步回调」能力。

### Incremental Delivery

1. Setup + Foundational → 基础就绪。
2. 完成 US1 → 独立验收 → MVP。
3. 完成 US2 → 寻址与 GetCached 行为、Streaming 接口。
4. 完成 US3 → 卸载与 EnsureDeviceResources 转发。
5. 完成 US4 → Import/Save 统一接口。
6. Polish → ABI 核对与 quickstart 验收。

### Notes

- 所有任务均基于 **plan.md 全量 ABI 内容**（原始 + 新增/修改），不仅实现契约更新小节。
- CMake 任务（T002）执行前**必须**已澄清构建根目录；各子模块**源码**引入；cmake 生成后须检查依赖与头文件完整性。
- 本 feature 无第三方依赖，无 7 步第三方集成任务。
- [P] 表示可与其他同阶段 [P] 任务并行；[USn] 仅用于 Phase 3–6 的故事归属。
