# Tasks: 009-render-core full

**Input**: Design documents from `specs/009-render-core-full/`  
**Prerequisites**: plan.md, spec.md, data-model.md, research.md, quickstart.md, contracts/ (API 雏形 in plan.md)  
**Constraint**: 任务只暴露契约已声明的 API；以 `specs/_contracts/009-rendercore-public-api.md` 与 plan.md 为准。

**Organization**: Tasks are grouped by user story (US1–US4) to enable independent implementation and testing.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: US1=ResourceDesc, US2=ShaderParams, US3=PassProtocol, US4=UniformBuffer
- Include exact file paths; only expose types/APIs from contract API 雏形

## Path Conventions

- **Source**: `src/render_core/` (or per main repo CMake; see plan.md)
- **Tests**: `tests/unit/`, `tests/contract/`, `tests/integration/`
- 构建方式与根目录须在执行 CMake 任务前澄清；规约见 `docs/engine-build-module-convention.md` §1.1。

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Project initialization and module structure for 009-RenderCore.

- [X] T001 Create directory structure per plan.md: `src/render_core/` (types.hpp, shader_params, resource_desc, pass_protocol, uniform_buffer, api.hpp), `tests/unit/`, `tests/contract/`, `tests/integration/`
- [X] T002 Add CMakeLists.txt for 009-render-core library (link 001-Core, 008-RHI per plan 依赖引入方式). **执行前须已澄清构建方式（各依赖 源码/DLL）与根目录（构建所在路径）；未澄清时禁止直接执行 cmake，须先向用户确认。**

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Contract types and shared definitions; all user stories depend on these. Only contract-declared types.

**⚠️ CRITICAL**: No user story implementation until this phase is complete.

- [X] T003 [P] Implement contract types in `src/render_core/types.hpp`: UniformLayout, VertexFormat, IndexFormat, TextureDesc, BufferDesc, PassResourceDecl, UniformBufferHandle (opaque handles/structs per API 雏形); add descriptor/param types used only by contract (UniformLayoutDesc, VertexFormatDesc, IndexFormatDesc, TextureDescParams, BufferDescParams, PassHandle, ResourceHandle, ResourceLifetime, BindSlot) without exposing RHI internals
- [X] T004 [P] Add any shared error/result type used by contract API (e.g. invalid handle or error code for reject-at-call semantics) in `src/render_core/types.hpp` or minimal shared header, consistent with contract

**Checkpoint**: Foundation ready — user story implementation can begin

---

## Phase 3: User Story 1 — ResourceDesc (Priority: P1) — MVP

**Goal**: VertexFormat, IndexFormat, TextureDesc, BufferDesc; descriptors accepted by RHI creation (contract 2. ResourceDesc).

**Independent Test**: Given vertex/index layout and texture/buffer params, produce descriptors; verify they are accepted by RHI creation API (per 008-RHI contract). Unsupported format/size → reject at API call (error or invalid handle).

### Implementation for User Story 1

- [X] T005 [P] [US1] Implement CreateVertexFormat(VertexFormatDesc const&) and VertexFormat in `src/render_core/resource_desc.hpp` / `.cpp`; reject unsupported format/size at call (contract API only)
- [X] T006 [P] [US1] Implement CreateIndexFormat(IndexFormatDesc const&) and IndexFormat in `src/render_core/resource_desc.hpp` / `.cpp`; reject unsupported at call (contract API only)
- [X] T007 [P] [US1] Implement CreateTextureDesc(TextureDescParams const&) and TextureDesc in `src/render_core/resource_desc.hpp` / `.cpp`; reject unsupported at call (contract API only)
- [X] T008 [P] [US1] Implement CreateBufferDesc(BufferDescParams const&) and BufferDesc in `src/render_core/resource_desc.hpp` / `.cpp`; reject unsupported at call (contract API only)

**Checkpoint**: US1 complete — ResourceDesc API usable for RHI creation

---

## Phase 4: User Story 2 — ShaderParams (Priority: P1)

**Goal**: DefineLayout, GetOffset; UniformLayout aligned with Shader reflection or hand-written layout (contract 1. ShaderParams).

**Independent Test**: Given layout definition, produce UniformLayout; GetOffset returns correct byte offset per member; layout matches Shader name/type convention.

### Implementation for User Story 2

- [X] T009 [P] [US2] Implement DefineLayout(UniformLayoutDesc const&) returning UniformLayout in `src/render_core/shader_params.hpp` / `.cpp`; failure returns empty/invalid handle (contract API only)
- [X] T010 [US2] Implement GetOffset(UniformLayout, char const* memberName) returning size_t in `src/render_core/shader_params.hpp` / `.cpp`; invalid/missing member per contract (contract API only)

**Checkpoint**: US2 complete — ShaderParams API usable for Shader/Material

---

## Phase 5: User Story 3 — PassProtocol (Priority: P1)

**Goal**: DeclareRead, DeclareWrite, SetResourceLifetime; PassResourceDecl consumable by PipelineCore RDG (contract 3. PassProtocol).

**Independent Test**: Declare read/write and lifetime for a pass; declarations consistent with PipelineCore RDG and RHI constraints; same-resource read+write in one pass defined by PipelineCore only.

### Implementation for User Story 3

- [X] T011 [P] [US3] Implement DeclareRead(PassHandle, ResourceHandle) returning PassResourceDecl in `src/render_core/pass_protocol.hpp` / `.cpp` (contract API only)
- [X] T012 [P] [US3] Implement DeclareWrite(PassHandle, ResourceHandle) returning PassResourceDecl in `src/render_core/pass_protocol.hpp` / `.cpp` (contract API only)
- [X] T013 [US3] Implement SetResourceLifetime(PassResourceDecl, ResourceLifetime) in `src/render_core/pass_protocol.hpp` / `.cpp`; align with PipelineCore RDG and RHI (contract API only)

**Checkpoint**: US3 complete — PassProtocol API usable for PipelineCore

---

## Phase 6: User Story 4 — UniformBuffer (Priority: P1)

**Goal**: CreateLayout (create buffer from UniformLayout), Update, RingBufferAdvance/RingBufferAllocSlot, Bind; RingBuffer exhaustion → Block (contract 4. UniformBuffer).

**Independent Test**: Create Uniform buffer from layout; Update and RingBuffer; Bind to RHI; no silent overwrite of in-flight data; optional validation at CreateLayout/Bind for layout vs Shader mismatch.

### Implementation for User Story 4

- [X] T014 [US4] Implement CreateUniformBuffer(UniformLayout) returning UniformBufferHandle (create Uniform buffer from layout) in `src/render_core/uniform_buffer.hpp` / `.cpp`; optional validation vs Shader convention, fail on mismatch (contract API only)
- [X] T015 [US4] Implement Update(UniformBufferHandle, void const* data, size_t size) in `src/render_core/uniform_buffer.hpp` / `.cpp` (contract API only)
- [X] T016 [US4] Implement RingBufferAdvance(UniformBufferHandle) or RingBufferAllocSlot(UniformBufferHandle); on exhaustion **Block** (block or require caller wait/retry until slot free) in `src/render_core/uniform_buffer.hpp` / `.cpp` (contract API only)
- [X] T017 [US4] Implement Bind(UniformBufferHandle, BindSlot) in `src/render_core/uniform_buffer.hpp` / `.cpp`; align with Shader module and RHI binding (contract API only)

**Checkpoint**: US4 complete — UniformBuffer API usable for Shader/Material and multi-Pass

---

## Phase 7: Polish & Cross-Cutting Concerns

**Purpose**: Public API surface and quality; only contract-exposed API.

- [X] T018 [P] Add public header `src/render_core/api.hpp` (or equivalent) aggregating only contract-declared API: ShaderParams (DefineLayout, GetOffset), ResourceDesc (CreateVertexFormat, CreateIndexFormat, CreateTextureDesc, CreateBufferDesc), PassProtocol (DeclareRead, DeclareWrite, SetResourceLifetime), UniformBuffer (CreateUniformBuffer, Update, RingBufferAdvance or RingBufferAllocSlot, Bind) and contract types; no RHI internals
- [X] T019 [P] Document validation behavior (optional validation at CreateUniformBuffer/Bind for Uniform vs Shader mismatch) per spec clarification
- [X] T020 [P] Add unit tests for descriptors and layout (e.g. GetOffset, CreateVertexFormat/CreateIndexFormat) in `tests/unit/` using only contract API
- [X] T021 [P] Add contract tests verifying descriptors are accepted by RHI creation (per 008-RHI contract) in `tests/contract/` if RHI test harness available
- [X] T022 Run quickstart.md validation (build and minimal usage example); ensure quickstart documents initialization order (Core/RHI before use) and upstream-only API usage per FR-006

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 1 (Setup)**: No dependencies — start first
- **Phase 2 (Foundational)**: Depends on Phase 1 — BLOCKS all user stories
- **Phase 3–6 (US1–US4)**: Depend on Phase 2; US1–US4 can proceed in parallel after Phase 2 (or sequentially P1→P2→P3→P4)
- **Phase 7 (Polish)**: Depends on Phases 3–6 complete

### User Story Dependencies

- **US1 (ResourceDesc)**: After Phase 2 only — no other story required
- **US2 (ShaderParams)**: After Phase 2 only — no other story required
- **US3 (PassProtocol)**: After Phase 2 only — no other story required
- **US4 (UniformBuffer)**: After Phase 2; uses UniformLayout from US2 — implement US2 before or with US4

### Parallel Opportunities

- T003, T004 can run in parallel (Phase 2)
- T005–T008 can run in parallel (US1)
- T009, T010 (US2) — T010 may depend on T009 if GetOffset uses layout internals
- T011, T012 can run in parallel; T013 after (US3)
- T014–T017 (US4) — CreateLayout before Update/RingBuffer/Bind; Update/RingBuffer/Bind can be parallel after T014
- T018, T019, T020, T021 can run in parallel (Polish)

---

## Implementation Strategy

### MVP First (e.g. US1 only)

1. Phase 1 Setup → Phase 2 Foundational
2. Phase 3 US1 (ResourceDesc)
3. Validate: descriptors accepted by RHI creation
4. Add api.hpp and minimal tests (T018, T020) for US1

### Incremental Delivery

1. Setup + Foundational → types and layout ready
2. US1 (ResourceDesc) → test independently
3. US2 (ShaderParams) → test independently
4. US3 (PassProtocol) → test independently
5. US4 (UniformBuffer) → test independently
6. Polish (api.hpp, docs, tests, quickstart)

### Contract Compliance

- Every task exposes **only** types and functions listed in `specs/_contracts/009-rendercore-public-api.md` (能力列表 + API 雏形).
- Use only upstream 001-Core and 008-RHI contract-declared types/APIs in implementation.

---

## Notes

- [P] = different files or independent subtasks; no ordering within same phase where marked.
- [USn] = task belongs to that user story for traceability.
- CMake/build: do not run cmake until 构建方式 and 根目录 are confirmed with user (see T002).
- Paths: `src/render_core/` and `tests/` follow plan.md; actual root may follow main repo CMake.
