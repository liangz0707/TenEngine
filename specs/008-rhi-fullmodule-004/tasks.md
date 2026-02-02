# Tasks: 008-RHI 完整模块（含 DX11、DXR、ABI TODO）

**Input**: Design documents from `specs/008-rhi-fullmodule-004/`  
**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/008-rhi-ABI-delta.md  
**Organization**: Tasks grouped by user story and ABI delta; implementation based on **full ABI** (existing `specs/_contracts/008-rhi-ABI.md` + delta).

## Format: `[ID] [P?] [Story?] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: User story (US1–US5) or P2 for ABI TODO P2 items
- Include exact file paths in descriptions

## Path Conventions

- Repository root: `g:\AIHUMAN\WorkSpaceSDD\TenEngine-008-rhi` (or current worktree root)
- Headers: `include/te/rhi/`
- Sources: `src/`, `src/d3d11/`, `src/d3d12/`, `src/vulkan/`, `src/metal/`
- Tests: `tests/`

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: CMake and build configuration for D3D11 backend; build root clarification.

- [x] T001 Add CMake option `TE_RHI_D3D11` and conditional compilation for D3D11 backend in `CMakeLists.txt`: when `TE_RHI_D3D11` and `WIN32`, add `src/d3d11/device_d3d11.cpp` to `TE_RHI_SOURCES`, `target_compile_definitions(te_rhi PRIVATE TE_RHI_D3D11=1)`, and `target_link_libraries(te_rhi PRIVATE d3d11 dxgi)`. **Build note**: Before running cmake, **building root must be clarified** (worktree root); all submodules use **source** inclusion per `docs/engine-build-module-convention.md` §3. Do not run cmake without confirming build root with user.
- [x] T002 [P] Confirm `Backend::D3D11 = 3` in `include/te/rhi/types.hpp`; add if missing.
- [x] T003 [P] Confirm `include/te/rhi/backend_d3d11.hpp` exists with `CreateDeviceD3D11()` and `DestroyDeviceD3D11(IDevice*)` declarations; create if missing.

---

## Phase 2: Foundational (D3D11 Backend Integration)

**Purpose**: Wire D3D11 backend so CreateDevice(Backend::D3D11) and DestroyDevice work; all backends remain real API (no stub for D3D11 path).

**⚠️ CRITICAL**: D3D11 backend must use real ID3D11* / IDXGI* API only.

- [x] T004 Ensure `src/d3d11/device_d3d11.cpp` implements DeviceD3D11, QueueD3D11, CommandListD3D11 (deferred context + FinishCommandList/ExecuteCommandList), BufferD3D11, TextureD3D11, SamplerD3D11, PSOD3D11, FenceD3D11 (HANDLE + CreateEventEx), SemaphoreD3D11, SwapChainD3D11 with real D3D11/DXGI calls; add file if missing.
- [x] T005 In `src/device/device.cpp`: add `#if defined(TE_RHI_D3D11) && TE_RHI_D3D11` `#include "te/rhi/backend_d3d11.hpp"`; in `CreateDevice(Backend backend)` add branch for `backend == Backend::D3D11` returning `CreateDeviceD3D11()`; in `DestroyDevice(IDevice* device)` add branch for `device->GetBackend() == Backend::D3D11` calling `DestroyDeviceD3D11(device)`.
- [x] T006 Extend `tests/device_create.cpp` to test `CreateDevice(Backend::D3D11)` when `TE_RHI_D3D11` and WIN32, and verify GetQueue(Graphics, 0) and GetFeatures() return valid data; ensure test links and calls real RHI API (covers upstream/backend).

**Checkpoint**: CreateDevice(D3D11) and DestroyDevice work; tests pass for D3D11 when enabled.

---

## Phase 3: User Story 1 - 创建设备、队列与后端选择 (Priority: P1) 🎯 MVP

**Goal**: Device creation, queue retrieval, feature/limits query, backend selection including D3D11; GetLimits and CreateFence(initialSignaled) available.

**Independent Test**: CreateDevice(Backend), GetQueue, GetFeatures, GetLimits, CreateFence(true/false); verify handles and readable limits; unsupported backend returns nullptr.

### Implementation for User Story 1

- [x] T007 [P] [US1] Add `DeviceLimits` struct in `include/te/rhi/types.hpp` (e.g. `maxBufferSize`, `maxTextureDimension2D`, `maxTextureDimension3D`, `minUniformBufferOffsetAlignment`); document defaults.
- [x] T008 [US1] Add `DeviceLimits const& GetLimits() const = 0` to `IDevice` in `include/te/rhi/device.hpp`; implement in stub (device.cpp), Vulkan, D3D12, Metal, D3D11 backends (query from underlying API where applicable).
- [x] T009 [US1] Change `IDevice::CreateFence()` to `IDevice::CreateFence(bool initialSignaled = false) = 0` in `include/te/rhi/device.hpp`; implement in stub, Vulkan (VK_FENCE_CREATE_SIGNALED_BIT), D3D12 (fence value 1 + Signal), Metal, D3D11 (CreateEvent + SetEvent when initialSignaled) in `src/device/device.cpp`, `src/vulkan/device_vulkan.cpp`, `src/d3d12/device_d3d12.cpp`, `src/metal/device_metal.mm`, `src/d3d11/device_d3d11.cpp`.
- [x] T010 [US1] Extend `tests/device_create.cpp` to call `GetLimits()` and `CreateFence(true)` / `CreateFence(false)` and verify behavior; ensure tests exercise RHI API and backend (no stub-only path when backend enabled).

**Checkpoint**: GetLimits() and CreateFence(bool) work on all backends; US1 test passes.

---

## Phase 4: User Story 2 - 录制并提交命令 (Priority: P1)

**Goal**: Submit with full parameters (signalFence, waitSemaphore, signalSemaphore); DrawIndexed; SetViewport; SetScissor.

**Independent Test**: Begin/End, DrawIndexed, SetViewport, SetScissor, ResourceBarrier, Submit(cmd, queue, signalFence, waitSem, signalSem); verify submission and sync semantics.

### Implementation for User Story 2

- [x] T011 [US2] Add free function overload `void Submit(ICommandList* cmd, IQueue* queue, IFence* signalFence, ISemaphore* waitSem, ISemaphore* signalSem)` in `include/te/rhi/command_list.hpp`; implement in `src/command_list/command_list.cpp` by calling `queue->Submit(cmd, signalFence, waitSem, signalSem)`.
- [x] T012 [P] [US2] Add `Viewport` struct (x, y, width, height, minDepth, maxDepth, float) and `ScissorRect` struct (x, y, width, height, int32_t or uint32_t) in `include/te/rhi/command_list.hpp` or `include/te/rhi/types.hpp`.
- [x] T013 [US2] Add to `ICommandList` in `include/te/rhi/command_list.hpp`: `void SetViewport(uint32_t first, uint32_t count, Viewport const* viewports) = 0;` and `void SetScissor(uint32_t first, uint32_t count, ScissorRect const* scissors) = 0;` and `void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) = 0;`.
- [x] T014 [US2] Implement SetViewport, SetScissor, DrawIndexed in stub in `src/device/device.cpp` (CommandListStub).
- [x] T015 [US2] Implement SetViewport, SetScissor, DrawIndexed in Vulkan backend in `src/vulkan/device_vulkan.cpp` (vkCmdSetViewport, vkCmdSetScissor, vkCmdDrawIndexed).
- [x] T016 [US2] Implement SetViewport, SetScissor, DrawIndexed in D3D12 backend in `src/d3d12/device_d3d12.cpp` (RSSetViewports, RSSetScissorRects, DrawIndexedInstanced).
- [x] T017 [US2] Implement SetViewport, SetScissor, DrawIndexed in Metal backend in `src/metal/device_metal.mm` (setViewport, setScissorRect, drawIndexedPrimitives).
- [x] T018 [US2] Implement SetViewport, SetScissor, DrawIndexed in D3D11 backend in `src/d3d11/device_d3d11.cpp` (RSSetViewports, RSSetScissorRects, DrawIndexedInstanced on deferred context).
- [x] T019 [US2] Extend `tests/command_list_submit.cpp` to test Submit(cmd, queue, fence, nullptr, nullptr) and DrawIndexed/SetViewport/SetScissor where applicable; validate against RHI contract.

**Checkpoint**: Submit full params, DrawIndexed, SetViewport, SetScissor work on all backends; US2 test passes.

---

## Phase 5: User Story 3 - 创建与管理资源 (Priority: P1)

**Goal**: Existing CreateBuffer/CreateTexture/CreateSampler/CreateView/Destroy; no new ABI for US3 in this feature (CopyBuffer* is P2).

- [x] T020 [US3] Confirm D3D11 backend CreateBuffer, CreateTexture, CreateSampler, CreateView, Destroy* in `src/d3d11/device_d3d11.cpp` match contract (real API, nullptr on failure); extend `tests/resources_create.cpp` to cover D3D11 backend when TE_RHI_D3D11 and WIN32.

**Checkpoint**: Resource creation/destruction on D3D11 consistent with other backends.

---

## Phase 6: User Story 4 - 创建与绑定 PSO (Priority: P1)

**Goal**: Existing CreateGraphicsPSO/CreateComputePSO/SetShader/Cache/DestroyPSO; D3D11 already implements PSO stubs.

- [x] T021 [US4] Confirm D3D11 CreateGraphicsPSO/CreateComputePSO in `src/d3d11/device_d3d11.cpp` return non-null for valid desc and use real D3D11 path where applicable; extend `tests/pso_create.cpp` for Backend::D3D11 when enabled.

**Checkpoint**: PSO creation on D3D11 testable.

---

## Phase 7: User Story 5 - 同步与多队列 (Priority: P1)

**Goal**: CreateFence(bool), Submit with fence/semaphore; covered by Phase 3 and Phase 4.

- [x] T022 [US5] Extend `tests/sync_fence_semaphore.cpp` to test CreateFence(true), CreateFence(false), and Submit(cmd, queue, signalFence, waitSem, signalSem) for at least one backend (e.g. Vulkan or D3D12); verify Wait/Signal/Reset semantics.

**Checkpoint**: Sync and multi-queue submission test coverage.

---

## Phase 8: ABI TODO P2 (RenderPass, CopyBuffer*, DescriptorSet)

**Purpose**: Implement P2 items from contracts/008-rhi-ABI-delta.md when scoped in this feature.

- [x] T023 [P2] Add `RenderPassDesc` (color/depth attachments, loadOp/storeOp) and `ICommandList::BeginRenderPass(RenderPassDesc const&)`, `EndRenderPass()` in `include/te/rhi/command_list.hpp`; implement in stub and all backends (Vulkan: vkCmdBeginRenderPass/vkCmdEndRenderPass; D3D12/D3D11: OMSetRenderTargets + clear; Metal: renderPassDescriptor).
- [x] T024 [P2] Add `ICommandList::CopyBuffer(IBuffer* src, size_t srcOffset, IBuffer* dst, size_t dstOffset, size_t size)`, `CopyBufferToTexture`, `CopyTextureToBuffer` (with region structs) in `include/te/rhi/command_list.hpp`; implement in Vulkan (vkCmdCopyBuffer, vkCmdCopyBufferToImage, vkCmdCopyImageToBuffer), D3D12 (CopyBufferRegion, CopyTextureRegion), Metal, D3D11 (CopyResource/CopySubresourceRegion as needed).
- [x] T025 [P2] Add `include/te/rhi/descriptor_set.hpp` (or extend resources.hpp) with `DescriptorSetLayoutDesc`, `DescriptorWrite`, `IDescriptorSetLayout`, `IDescriptorSet`; add `IDevice::CreateDescriptorSetLayout`, `AllocateDescriptorSet`, `UpdateDescriptorSet` in `include/te/rhi/device.hpp`; implement in Vulkan (VkDescriptorSetLayout/VkDescriptorSet), D3D12 (DescriptorHeap + root signature), Metal (MTLArgumentEncoder), D3D11 (simplified: SRV array or no-op documented).

**Checkpoint**: RenderPass, CopyBuffer*, and DescriptorSet APIs implemented per ABI delta (or deferred to next iteration with tasks marked).

---

## Phase 9: DXR (D3D12 Ray Tracing)

**Purpose**: RHI-level ray tracing abstraction; D3D12 only implementation.

- [x] T026 Add `include/te/rhi/raytracing.hpp` with `RaytracingAccelerationStructureDesc`, `DispatchRaysDesc` and extend `ICommandList` (or optional interface) with `BuildAccelerationStructure(...)` and `DispatchRays(DispatchRaysDesc const&)`; default implementations no-op / return.
- [x] T027 Implement BuildAccelerationStructure and DispatchRays in D3D12 backend in `src/d3d12/device_d3d12.cpp`: obtain `ID3D12GraphicsCommandList4` via QueryInterface, call `BuildRaytracingAccelerationStructure` and `DispatchRays`; Vulkan, Metal, D3D11 backends leave as no-op or return without error as per research.md.

**Checkpoint**: DXR types and D3D12 implementation available; other backends no-op.

---

## Phase 10: Polish & Cross-Cutting

**Purpose**: ABI writeback, quickstart validation, documentation.

- [x] T028 Update `specs/_contracts/008-rhi-ABI.md` with all delta entries from `specs/008-rhi-fullmodule-004/contracts/008-rhi-ABI-delta.md` (Backend D3D11, DeviceLimits, GetLimits, CreateFence(bool), Submit overload, DrawIndexed, Viewport, Scissor, SetViewport, SetScissor; P2: RenderPass, CopyBuffer*, DescriptorSet; DXR types and methods); remove or resolve TODO section items that are now implemented.
- [x] T029 Run quickstart validation: from build root, `cmake -B build` (after build root confirmed with user), `cmake --build build`, `ctest --test-dir build`; update `specs/008-rhi-fullmodule-004/quickstart.md` with TE_RHI_D3D11 option and any new test steps.
- [x] T030 [P] Document in `specs/008-rhi-fullmodule-004/spec.md` or `docs/module-specs/008-rhi.md`: D3D11 backend behavior (single queue, ResourceBarrier no-op); threading constraints per FR-007; DXR availability (D3D12 only).

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 1 (Setup)**: No dependencies; can start immediately.
- **Phase 2 (Foundational)**: Depends on Phase 1; blocks US1–US5 for D3D11 path.
- **Phase 3 (US1)**: Depends on Phase 2; GetLimits/CreateFence(bool) used by later stories.
- **Phase 4 (US2)**: Depends on Phase 2; Submit full params and DrawIndexed/Viewport/Scissor.
- **Phase 5–7 (US3–US5)**: Depend on Phase 2; confirm D3D11 and extend tests.
- **Phase 8 (P2 ABI)**: Depends on Phases 2–4; can follow Phase 7.
- **Phase 9 (DXR)**: Depends on Phase 2 (D3D12 backend present); can run after Phase 4.
- **Phase 10 (Polish)**: Depends on completion of targeted phases (at least 1–4, optionally 8–9).

### User Story Dependencies

- **US1 (P1)**: After Phase 2; no dependency on US2–US5.
- **US2 (P1)**: After Phase 2; uses Submit/Fence from US1.
- **US3–US5 (P1)**: After Phase 2; independent of each other.
- **P2 (Phase 8)**: After Phase 4; optional in same release.

### Parallel Opportunities

- T002, T003 can run in parallel (Phase 1).
- T007 can run in parallel with T008 (Phase 3).
- T015–T018 (SetViewport/SetScissor/DrawIndexed per backend) can be parallelized by backend.
- T023–T025 (P2) can be split by feature (RenderPass, Copy*, DescriptorSet).
- T030 can run in parallel with T028, T029.

---

## Implementation Strategy

### MVP First (User Story 1 + D3D11 + ABI P1)

1. Complete Phase 1 (Setup) and Phase 2 (D3D11 integration).
2. Complete Phase 3 (US1: GetLimits, CreateFence(bool)).
3. Complete Phase 4 (US2: Submit full params, DrawIndexed, SetViewport, SetScissor).
4. **STOP and VALIDATE**: Run device_create, command_list_submit, sync tests; confirm D3D11 and new APIs.
5. Optionally add Phase 9 (DXR) and Phase 10 (Polish).

### Incremental Delivery

1. Phase 1 + 2 → D3D11 backend usable.
2. Phase 3 + 4 → ABI TODO P1 (CreateFence(bool), GetLimits, Submit, DrawIndexed, Viewport, Scissor) done.
3. Phase 5–7 → All user stories covered for D3D11 and new interfaces.
4. Phase 8 → P2 (RenderPass, CopyBuffer*, DescriptorSet) if in scope.
5. Phase 9 → DXR (D3D12).
6. Phase 10 → ABI writeback and quickstart.

---

## Notes

- **Build root**: Before any `cmake -B build` or configure step, **confirm with user** the build root (worktree root). All submodules use **source** inclusion; see `docs/engine-build-module-convention.md` §3.
- **Third-party**: volk, Vulkan-Headers, d3d11, d3d12, dxgi, Metal are already referenced in plan; D3D11 uses system SDK (no extra FetchContent). No new third-party 7-step tasks added unless a new library is introduced.
- **Tests**: Test tasks (T006, T010, T019, T020–T022) should invoke **real RHI and backend API**; tests link only `te_rhi` target (dependencies via target_link_libraries).
- **Format**: All tasks use checklist format `- [ ] Tnnn [P?] [USn?] Description with file path`.
