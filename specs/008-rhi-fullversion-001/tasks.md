# Tasks: 008-RHI 完整功能

**Input**: Design documents from `specs/008-rhi-fullversion-001/`  
**Prerequisites**: plan.md, spec.md, `specs/_contracts/008-rhi-public-api.md`  
**Authority**: plan.md, `specs/_contracts/008-rhi-public-api.md`（API 雏形）

**Organization**: Tasks grouped by user story (US1–US5) for independent implementation and testing. US1 blocks US2–US5; US2–US5 may proceed in parallel after US1.

## 契约约束（Contract Constraint）

**任务只暴露契约已声明的 API**。实现时：

- 对外暴露的类型与函数 **必须** 与 `specs/_contracts/008-rhi-public-api.md` 的 **「API 雏形」** 小节一致。
- 不得引入契约未声明的公开类型、函数或符号。
- 实现内部仅使用 `specs/_contracts/001-core-public-api.md` 已声明的类型与 API。

API 雏形涵盖：Backend, QueueType, DeviceFeatures, IDevice, IQueue, ICommandList, IBuffer, ITexture, ISampler, IPSO, IFence, ISemaphore, ViewHandle, *Desc 及 Create*/Destroy*/Begin/End/Draw/Dispatch/Copy/ResourceBarrier/Submit/Wait/Signal 等。

| Phase | 对应契约 API 雏形 节 |
|-------|----------------------|
| US1 (T007–T011) | §1 Device 与后端、§2 队列、§3 特性查询 |
| US2 (T012–T014) | §4 命令列表 |
| US3 (T015–T017) | §5 资源与视图 |
| US4 (T018–T020) | §6 PSO |
| US5 (T021–T023) | §7 同步 |
| Polish (T024–T025) | §8 错误与约束、全部 |

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: User story (US1–US5)
- Include exact file paths. **构建/CMake**：执行 cmake 前须已澄清 **构建方式**（各依赖 源码/DLL）与 **根目录**；未澄清时禁止直接执行。规约见 `docs/build-module-convention.md` §1.1。

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Project skeleton, CMake, 001-core dependency, include layout.

- [x] T001 Create directory structure per plan: `include/te/rhi/`, `src/device/`, `src/command_list/`, `src/resources/`, `src/pso/`, `src/sync/`, `tests/`.
- [x] T002 Create `CMakeLists.txt` with C++17, `TENENGINE_CMAKE_DIR` → `../TenEngine-001-core/cmake`, `include(TenEngineHelpers)`, `tenengine_resolve_my_dependencies("008-rhi" OUT_DEPS MY_DEPS)`, `add_library(te_rhi STATIC ...)`, `target_link_libraries(te_rhi PRIVATE ${MY_DEPS})`, `target_include_directories(te_rhi PUBLIC include)`, `tenengine_add_module_test(NAME te_rhi_test MODULE_TARGET te_rhi SOURCES tests/device_create.cpp ENABLE_CTEST)`. Add placeholder `tests/device_create.cpp` including `te/rhi/device.hpp` and calling `CreateDevice(Backend::Vulkan)`. **配置/构建**：执行 `cmake -B build` 等前须已澄清构建方式（001-core 源码）与根目录（`TenEngine-008-rhi`）；未澄清时禁止直接执行 cmake。规约见 `docs/build-module-convention.md` §1.1.
- [x] T003 [P] Add `include/te/rhi/types.hpp` with `Backend`, `QueueType`, `DeviceFeatures` (minimal: e.g. `maxTextureDimension2D`, `maxTextureDimension3D`), and forward declarations for `IDevice`, `IQueue`, `ICommandList`, `IBuffer`, `ITexture`, `ISampler`, `IPSO`, `IFence`, `ISemaphore` — 仅契约 API 雏形 §1–§7 已声明的类型。

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Declarations and stubs so `te_rhi` builds; all user stories depend on this.

**⚠️ CRITICAL**: No user story implementation can begin until this phase is complete.

- [x] T004 [P] Add `include/te/rhi/device.hpp` with `IDevice`, `CreateDevice`, `DestroyDevice`, `SelectBackend`, `GetSelectedBackend`, `GetQueue`, `GetFeatures`, `CreateCommandList`, `DestroyCommandList` — 仅契约 API 雏形 §1、§3、§4 已声明的接口。
- [x] T005 [P] Add `include/te/rhi/queue.hpp` with `IQueue` and `QueueType` — 仅契约 API 雏形 §2 已声明的类型。
- [x] T006 Add `src/device/device.cpp` with stub implementations: `SelectBackend` / `GetSelectedBackend` no-op or default; `CreateDevice` return `nullptr`; `DestroyDevice` no-op; `GetQueue` return `nullptr`; `GetFeatures` return static `DeviceFeatures`; `CreateCommandList` return `nullptr`; `DestroyCommandList` no-op. Ensure `te_rhi` builds and links.

**Checkpoint**: Foundation ready — User Story 1 implementation can begin.

---

## Phase 3: User Story 1 — 创建设备、队列与后端选择 (P1) 🎯 MVP

**Goal**: CreateDevice, GetQueue, GetFeatures, SelectBackend; valid IDevice and queue handles for downstream stories.

**Independent Test**: Call CreateDevice, GetQueue, GetFeatures, SelectBackend; verify device creation, queue retrieval, and readable device features. Spec: `specs/008-rhi-fullversion-001/spec.md` User Story 1.

### Implementation for User Story 1

- [x] T007 [US1] Implement `SelectBackend`, `GetSelectedBackend` in `src/device/device.cpp` (global or device-scoped default backend state per research).
- [x] T008 [US1] Implement `CreateDevice(Backend)`, `CreateDevice()`, `DestroyDevice` in `src/device/`；仅契约 API 雏形 §1、§8；使用 001-core 契约 Alloc/Free、Log。后端不可用返回 `nullptr`，不自动回退。
- [x] T009 [US1] Implement `IDevice::GetQueue(QueueType, uint32_t)` in `src/device/`; return `nullptr` for invalid type or out-of-range index.
- [x] T010 [US1] Implement `DeviceFeatures` and `IDevice::GetFeatures` in `src/device/`; minimal fields (`maxTextureDimension2D`, `maxTextureDimension3D`) per plan; return const reference to device-owned features.
- [x] T011 [US1] Extend `tests/device_create.cpp` to validate CreateDevice, GetQueue, GetFeatures, SelectBackend per spec Independent Test and acceptance scenarios; use stub backend if no Vulkan/D3D12/Metal available.

**Checkpoint**: User Story 1 independently testable (device creation, queue, features, backend selection).

---

## Phase 4: User Story 2 — 录制并提交命令 (P1)

**Goal**: Begin/End command list, Draw/Dispatch/Copy, ResourceBarrier, Submit to queue.

**Independent Test**: Begin/End, record Draw/Dispatch/Copy, ResourceBarrier, Submit; verify submission semantics per contract. Spec User Story 2.

### Implementation for User Story 2

- [x] T012 [P] [US2] Add `include/te/rhi/command_list.hpp` with `ICommandList`, `Begin`, `End`, `Draw`, `Dispatch`, `Copy`, `ResourceBarrier`, `Submit` — 仅契约 API 雏形 §4 已声明的接口。
- [x] T013 [US2] Implement `IDevice::CreateCommandList`, `DestroyCommandList`, `Begin`, `End`, `ICommandList::Draw`, `Dispatch`, `Copy`, `ResourceBarrier`, `Submit(ICommandList*, IQueue*)` in `src/command_list/` — 仅契约 API 雏形 §4；单次录制周期语义与 Submit 行为。
- [x] T014 [US2] Add `tests/command_list_submit.cpp` (and register in CMakeLists via `tenengine_add_module_test` or extend existing test target) to validate Begin/End, Draw/Dispatch/Copy, ResourceBarrier, Submit per spec Independent Test.

**Checkpoint**: User Story 2 independently testable.

---

## Phase 5: User Story 3 — 创建与管理资源 (P1)

**Goal**: CreateBuffer, CreateTexture, CreateSampler, CreateView, Destroy; lifecycle and view binding per contract.

**Independent Test**: Create buffers, textures, samplers, views; verify creation success and destroy order. Spec User Story 3.

### Implementation for User Story 3

- [x] T015 [P] [US3] Add `include/te/rhi/resources.hpp` with `IBuffer`, `ITexture`, `ISampler`, `CreateBuffer`, `CreateTexture`, `CreateSampler`, `CreateView`, `Destroy*`, `BufferDesc` / `TextureDesc` / `SamplerDesc` / `ViewDesc`, `ViewHandle` — 仅契约 API 雏形 §5 已声明的接口与类型。
- [x] T016 [US3] Implement `IDevice::CreateBuffer`, `CreateTexture`, `CreateSampler`, `CreateView`, `DestroyBuffer`, `DestroyTexture`, `DestroySampler` in `src/resources/` — 仅契约 API 雏形 §5；使用 001-core 契约 Alloc/Free；失败返回 `nullptr`。
- [x] T017 [US3] Add `tests/resources_create.cpp` to validate Create* and Destroy per spec Independent Test.

**Checkpoint**: User Story 3 independently testable.

---

## Phase 6: User Story 4 — 创建与绑定 PSO (P1)

**Goal**: CreateGraphicsPSO, CreateComputePSO, SetShader, Cache, DestroyPSO; PSO usable for command recording.

**Independent Test**: Create graphics/compute PSO, bind shader, optionally cache; verify PSO creation. Spec User Story 4.

### Implementation for User Story 4

- [x] T018 [P] [US4] Add `include/te/rhi/pso.hpp` with `IPSO`, `CreateGraphicsPSO`, `CreateComputePSO`, `SetShader`, `Cache`, `DestroyPSO`, `GraphicsPSODesc` / `ComputePSODesc` — 仅契约 API 雏形 §6 已声明的接口与类型。
- [x] T019 [US4] Implement `IDevice::CreateGraphicsPSO`, `CreateComputePSO`, `SetShader`, `Cache`, `DestroyPSO` in `src/pso/` — 仅契约 API 雏形 §6；描述符含 Shader 字节码或模块引用；失败返回 `nullptr`。
- [x] T020 [US4] Add `tests/pso_create.cpp` to validate PSO creation and binding per spec Independent Test.

**Checkpoint**: User Story 4 independently testable.

---

## Phase 7: User Story 5 — 同步与多队列 (P1)

**Goal**: CreateFence, CreateSemaphore, Wait, Signal; multi-queue and cross-frame sync per contract.

**Independent Test**: CreateFence/CreateSemaphore, Wait/Signal; verify sync semantics. Spec User Story 5.

### Implementation for User Story 5

- [x] T021 [P] [US5] Add `include/te/rhi/sync.hpp` with `IFence`, `ISemaphore`, `CreateFence`, `CreateSemaphore`, `Wait`, `Signal`, `DestroyFence`, `DestroySemaphore` — 仅契约 API 雏形 §7 已声明的接口与类型。
- [x] T022 [US5] Implement `IDevice::CreateFence`, `CreateSemaphore`, `Wait`, `Signal`, `DestroyFence`, `DestroySemaphore` in `src/sync/` — 仅契约 API 雏形 §7。
- [x] T023 [US5] Add `tests/sync_fence_semaphore.cpp` to validate Fence/Semaphore and multi-queue sync per spec Independent Test.

**Checkpoint**: User Story 5 independently testable.

---

## Phase 8: Polish & Cross-Cutting

**Purpose**: Quickstart validation, cleanup, contract alignment.

- [x] T024 [P] Run `quickstart.md` validation (build, ctest); update `quickstart.md` if paths or steps differ.
- [x] T025 Review all Create*/Destroy* and error paths; ensure `nullptr` on failure, no auto-fallback; 仅使用 001-core 契约 API；对齐 `specs/_contracts/008-rhi-public-api.md` API 雏形 §8 与 clarifications。

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 1 (Setup)**: No dependencies.
- **Phase 2 (Foundational)**: Depends on Phase 1; **blocks** all user stories.
- **Phase 3 (US1)**: Depends on Phase 2. **US1 blocks US2–US5.**
- **Phases 4–7 (US2–US5)**: Depend on Phase 2 and US1; can proceed **in parallel** after US1.
- **Phase 8 (Polish)**: Depends on completion of desired user stories.

### User Story Dependencies

- **US1**: After Foundational only. Delivers Device, Queue, GetFeatures, SelectBackend.
- **US2**: After US1. Depends on IDevice, IQueue.
- **US3**: After US1. Depends on IDevice.
- **US4**: After US1 (and optionally US3 for views). Depends on IDevice.
- **US5**: After US1 (and optionally US2 for barriers). Depends on IDevice.

### Parallel Opportunities

- T003, T004, T005 within Phase 2.
- T012, T015, T018, T021 (header adds for US2–US5) can run in parallel after US1.
- US2–US5 implementation phases can be parallelized across developers.

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Phase 1 (Setup) and Phase 2 (Foundational).
2. Complete Phase 3 (US1). Validate via `tests/device_create.cpp` and quickstart.
3. **STOP and VALIDATE**: Device creation, GetQueue, GetFeatures, SelectBackend work independently.

### Incremental Delivery

1. Setup + Foundational → buildable skeleton.
2. US1 → Device/Queue/Features/Backend → validate.
3. US2 → CommandList/Submit → validate.
4. US3 → Resources/Views → validate.
5. US4 → PSO → validate.
6. US5 → Sync → validate.
7. Polish → quickstart + contract alignment.

---

## Notes

- **契约约束**：任务只暴露 `specs/_contracts/008-rhi-public-api.md` API 雏形已声明的类型与 API；实现仅使用 `001-core-public-api.md` 已声明的接口。不得新增契约未列出的公开符号。
- **[P]** = different files, no task dependencies; **[Story]** = maps to User Story for traceability.
- Build root: `TenEngine-008-rhi`. Dependency: 001-core **源码**. Clarify with user before running cmake if not already confirmed.
