# Tasks: 002-Object 完整模块实现

**Input**: Design documents from `/specs/002-object-fullmodule-001/`  
**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/

**Tests**: Spec 要求单元测试覆盖 TypeRegistry、CreateInstance、Serializer 往返、PropertyBag；测试须能覆盖上游模块能力（通过本模块调用 001-Core Alloc/Free 等），见 quickstart.md。

**Organization**: Tasks grouped by user story for independent implementation and testing.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: US1–US4 per spec.md
- Include exact file paths in descriptions

## Path Conventions

- **Build root**: 仓库根目录（单仓）；002-object 源码在 `Engine/TenEngine-002-object/`
- **Public headers**: `Engine/TenEngine-002-object/include/te/object/*.hpp`
- **Source**: `Engine/TenEngine-002-object/src/*.cpp`
- **Tests**: `Engine/TenEngine-002-object/tests/unit/*.cpp`

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Project structure and CMake alignment with plan; dependency resolution.

> **构建/CMake 说明**：执行配置或构建（如 `cmake -B build`）前，**须已与用户确认构建根目录**（本仓库为仓库根）；各子模块均使用**源码**方式引入依赖。未澄清根目录时**禁止**直接执行 cmake，须先询问用户。cmake 生成之后须检查：引入的头文件/源文件是否完整、是否存在循环依赖或缺失依赖；发现问题须在任务中标注或先修复再继续。规约见 `docs/engine-build-module-convention.md` §3。

- [x] T001 Verify project structure in Engine/TenEngine-002-object/ matches plan (include/te/object/, src/, tests/unit/, cmake/); confirm build root is repo root with user before running cmake
- [x] T002 Ensure Engine/TenEngine-002-object/CMakeLists.txt declares target te_object and resolves 001-core via tenengine_resolve_my_dependencies("002-object"); add build-root clarification note in comments. Do not run cmake until build root is confirmed
- [x] T003 [P] Ensure public include path Engine/TenEngine-002-object/include/ is set so headers are exposed as te/object/*.hpp per ABI

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: All ABI types and Core memory integration; required before any user story.

**⚠️ CRITICAL**: No user story work can begin until this phase is complete.

- [x] T004 [P] Define TypeId and kInvalidTypeId in Engine/TenEngine-002-object/include/te/object/TypeId.hpp per ABI
- [x] T005 [P] Define MethodDescriptor (minimal placeholder struct) in Engine/TenEngine-002-object/include/te/object/TypeId.hpp per ABI
- [x] T006 [P] Define PropertyDescriptor (name, valueTypeId, defaultValue) in Engine/TenEngine-002-object/include/te/object/PropertyDescriptor.hpp
- [x] T007 [P] Define TypeDescriptor (id, name, size, properties, propertyCount, methods, methodCount, baseTypeId) in Engine/TenEngine-002-object/include/te/object/TypeDescriptor.hpp
- [x] T008 [P] Define SerializedBuffer (data, size, capacity) in Engine/TenEngine-002-object/include/te/object/SerializedBuffer.hpp
- [x] T009 [P] Define ObjectRef (uint8_t guid[16]) in Engine/TenEngine-002-object/include/te/object/ObjectRef.hpp and GUID in Engine/TenEngine-002-object/include/te/object/Guid.hpp
- [x] T010 Implement Core memory integration: wrap 001-Core Alloc/Free (or GetDefaultAllocator) in Engine/TenEngine-002-object/src/CoreMemory.cpp and optional detail header so CreateInstance and type metadata use only 001-core-public-api

**Checkpoint**: Foundation ready – all ABI types and Core Alloc path available; user story implementation can begin.

---

## Phase 3: User Story 1 – 类型注册与查询 (Priority: P1) 🎯 MVP

**Goal**: TypeRegistry::RegisterType, GetTypeByName, GetTypeById; duplicate TypeId rejected.

**Independent Test**: Register one type; GetTypeByName/GetTypeById return same descriptor; second RegisterType with same id returns false.

### Tests for User Story 1

- [x] T011 [P] [US1] Add unit test Engine/TenEngine-002-object/tests/unit/TypeRegistry_test.cpp: register type, GetTypeByName/GetTypeById match, duplicate id returns false; ensure test uses Core init if required (cover upstream)

### Implementation for User Story 1

- [x] T012 [P] [US1] Declare TypeRegistry with static RegisterType, GetTypeByName, GetTypeById in Engine/TenEngine-002-object/include/te/object/TypeRegistry.hpp
- [x] T013 [US1] Implement TypeRegistry in Engine/TenEngine-002-object/src/TypeRegistry.cpp: double index by id and by name; reject duplicate TypeId; return nullptr for unknown name/id

**Checkpoint**: User Story 1 complete; TypeRegistry test passes.

---

## Phase 4: User Story 2 – 类型工厂创建实例 (Priority: P1)

**Goal**: TypeRegistry::CreateInstance(TypeId) using Core Alloc; nullptr for invalid id.

**Independent Test**: After register, CreateInstance(id) returns non-null; for unregistered id returns nullptr.

### Tests for User Story 2

- [x] T014 [P] [US2] Add unit test Engine/TenEngine-002-object/tests/unit/CreateInstance_test.cpp: register type, CreateInstance returns non-null and usable; invalid id returns nullptr; verify allocation via Core (cover upstream Alloc path)

### Implementation for User Story 2

- [x] T015 [US2] Implement CreateInstance(TypeId) in Engine/TenEngine-002-object/src/TypeRegistry.cpp: get TypeDescriptor by id, allocate size bytes via Core Alloc, return pointer or nullptr

**Checkpoint**: User Stories 1 and 2 complete; CreateInstance test passes.

---

## Phase 5: User Story 3 – 序列化与反序列化往返 (Priority: P1)

**Goal**: ISerializer (Serialize, Deserialize, GetCurrentVersion, SetVersionMigration), IVersionMigration::Migrate; round-trip equivalence.

**Independent Test**: Register type, create instance, serialize to buffer, deserialize to new instance; key fields match.

### Tests for User Story 3

- [x] T016 [P] [US3] Add unit test Engine/TenEngine-002-object/tests/unit/Serializer_roundtrip_test.cpp: register type, create instance, serialize then deserialize, compare; optional version migration path (cover upstream via CreateInstance/Alloc)

### Implementation for User Story 3

- [x] T017 [P] [US3] Declare IVersionMigration::Migrate in Engine/TenEngine-002-object/include/te/object/VersionMigration.hpp
- [x] T018 [P] [US3] Declare ISerializer (Serialize, Deserialize, GetCurrentVersion, SetVersionMigration) in Engine/TenEngine-002-object/include/te/object/Serializer.hpp
- [x] T019 [US3] Implement at least one ISerializer (e.g. binary) and version-migration hook in Engine/TenEngine-002-object/src/Serializer.cpp and Engine/TenEngine-002-object/src/VersionMigration.cpp if needed; Deserialize calls Migrate when version < GetCurrentVersion()

**Checkpoint**: User Story 3 complete; Serializer roundtrip test passes.

---

## Phase 6: User Story 4 – 属性读写与 PropertyBag (Priority: P2)

**Goal**: PropertyBag::GetProperty, SetProperty, FindProperty; consistent with TypeDescriptor/PropertyDescriptor.

**Independent Test**: Object implementing PropertyBag: SetProperty then GetProperty match; FindProperty returns descriptor; unknown name returns false/nullptr.

### Tests for User Story 4

- [x] T020 [P] [US4] Add unit test Engine/TenEngine-002-object/tests/unit/PropertyBag_test.cpp: implement or use default PropertyBag, SetProperty/GetProperty/FindProperty; unknown name returns false/nullptr

### Implementation for User Story 4

- [x] T021 [P] [US4] Declare PropertyBag (GetProperty, SetProperty, FindProperty) in Engine/TenEngine-002-object/include/te/object/PropertyBag.hpp
- [x] T022 [US4] Implement default PropertyBag (e.g. backed by TypeDescriptor property list) in Engine/TenEngine-002-object/src/PropertyBag.cpp

**Checkpoint**: All user stories complete; PropertyBag test passes.

---

## Phase 7: Polish & Cross-Cutting Concerns

**Purpose**: ABI completeness, build validation, docs.

- [x] T023 [P] Verify every symbol in plan.md full ABI table is implemented and exposed in Engine/TenEngine-002-object (no missing types or methods)
- [x] T024 Run build and ctest per specs/002-object-fullmodule-001/quickstart.md; fix any missing includes or link errors (build root already confirmed)
- [x] T025 [P] Update Engine/TenEngine-002-object/README.md or docs if needed for build and usage

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 1 (Setup)**: No dependencies – start first; do not run cmake until build root confirmed.
- **Phase 2 (Foundational)**: Depends on Phase 1 – BLOCKS all user stories.
- **Phase 3–6 (User Stories)**: Depend on Phase 2; US1 → US2 → US3 → US4 in order (US2 uses TypeRegistry; US3 uses CreateInstance; US4 can use TypeDescriptor).
- **Phase 7 (Polish)**: Depends on Phases 3–6 complete.

### User Story Dependencies

- **US1 (P1)**: After Phase 2 – no other story dependency.
- **US2 (P1)**: After US1 (needs TypeRegistry).
- **US3 (P1)**: After US2 (needs CreateInstance for test).
- **US4 (P2)**: After Phase 2; may use TypeDescriptor from US1.

### Parallel Opportunities

- Phase 1: T003 [P]
- Phase 2: T004–T009 [P], T010 sequential after types
- Phase 3: T011, T012 [P]; T013 after T012
- Phase 4: T014 [P]; T015 after T013
- Phase 5: T016, T017, T018 [P]; T019 after T017–T018
- Phase 6: T020, T021 [P]; T022 after T021
- Phase 7: T023, T025 [P]; T024 after build

---

## Parallel Example: User Story 1

```text
# After Phase 2:
T011: TypeRegistry_test.cpp
T012: TypeRegistry.hpp declarations
# Then T013: TypeRegistry.cpp implementation
```

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Phase 1: Setup (confirm build root, CMake layout).
2. Phase 2: Foundational (all types + Core memory).
3. Phase 3: US1 (TypeRegistry + test).
4. **STOP and VALIDATE**: Run TypeRegistry test; then optionally add US2–US4.

### Incremental Delivery

1. Setup + Foundational → types and Core ready.
2. US1 → TypeRegistry test passes (MVP).
3. US2 → CreateInstance test passes.
4. US3 → Serializer roundtrip test passes.
5. US4 → PropertyBag test passes.
6. Phase 7 → ABI check + quickstart validation.

### Notes

- [P] = different files, no ordering requirement within phase.
- [USn] = task belongs to that user story for traceability.
- Tests must call through public API so 001-Core usage (Alloc/Free) is exercised where applicable.
- No third-party dependencies; no 7-step third-party tasks.
- Build root: repo root; 001-core via source only; clarify with user before running cmake.
