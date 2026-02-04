# Tasks: 002-Object Full Module Content

**Input**: plan.md, spec.md, `contracts/002-object-ABI-full.md`  
**Authority**: 以 plan.md、`specs/_contracts/002-object-public-api.md` 及 `contracts/002-object-ABI-full.md` 全量 ABI 为准；任务实现契约与 ABI 声明的全部符号与能力。  
**Prerequisites**: 仅暴露契约已声明的类型与 API；仅使用 `specs/_contracts/001-core-public-api.md` 已声明的类型与接口。

**Organization**: 按 User Story 分组，每故事可独立实现与验证。

## Format: `[ID] [P?] [Story] Description`

- **[P]**: 可并行（不同文件、无依赖）
- **[Story]**: 所属 User Story（US1–US4）
- 描述中含具体文件路径；实现须覆盖 `contracts/002-object-ABI-full.md` 全量 ABI。

## Path Conventions (from plan.md)

- **库根**: `modules/object/`
- **对外头文件**: `include/te/object/*.hpp`（仅契约声明的 API）
- **实现**: `src/*.cpp`
- **测试**: `tests/unit/*.cpp`

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: 工程初始化与目录结构，与 plan 一致

- [x] T001 Create directory structure per plan (modules/object/include/te/object, modules/object/include/te/object/detail, modules/object/src, modules/object/tests/unit)
- [x] T002 [P] Add CMakeLists.txt at modules/object/ with C++17, link to 001-Core (only contract-declared APIs); follow docs/build-module-convention.md
- [x] T003 [P] Add tests/CMakeLists.txt and wire unit test targets per tenengine_add_module_test

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: 所有 User Story 依赖的公共类型与约束；实现 `contracts/002-object-ABI-full.md` 中的类型声明

- [x] T004 [P] Declare TypeId and kInvalidTypeId in modules/object/include/te/object/TypeId.hpp per contracts/002-object-ABI-full.md
- [x] T005 [P] Declare PropertyDescriptor struct in modules/object/include/te/object/PropertyDescriptor.hpp per ABI
- [x] T006 [P] Declare MethodDescriptor (placeholder) in modules/object/include/te/object/TypeDescriptor.hpp per ABI
- [x] T007 [P] Declare GUID and ObjectRef in modules/object/include/te/object/Guid.hpp and modules/object/include/te/object/ObjectRef.hpp per ABI (16-byte only)
- [x] T008 [P] Declare SerializedBuffer in modules/object/include/te/object/SerializedBuffer.hpp per ABI (data, size, capacity)
- [x] T009 Add CoreMemory adapter (Alloc/Free via 001-Core or fallback) in modules/object/include/te/object/detail/CoreMemory.hpp and modules/object/src/CoreMemory.cpp; document Core init precondition per contract

**Checkpoint**: 基础类型与 ABI 一致；US 实现可开始

---

## Phase 3: User Story 1 - 类型注册与反射查询 (Priority: P1) — MVP

**Goal**: TypeRegistry::RegisterType、GetTypeByName、GetTypeById、TypeDescriptor 含属性/方法列表与基类链。

**Independent Test**: 注册类型后通过 GetTypeByName/ById 查询到 TypeDescriptor（含属性/方法、基类链）。

- [x] T010 [P] [US1] Implement TypeDescriptor (id, name, size, properties, propertyCount, methods, methodCount, baseTypeId) in modules/object/include/te/object/TypeDescriptor.hpp per contracts/002-object-ABI-full.md
- [x] T011 [US1] Implement TypeRegistry::RegisterType (reject duplicate TypeId; duplicate name per implementation) in modules/object/src/TypeRegistry.cpp, header in modules/object/include/te/object/TypeRegistry.hpp; use Core Alloc for metadata
- [x] T012 [US1] Implement TypeRegistry::GetTypeByName and GetTypeById in modules/object/src/TypeRegistry.cpp per ABI
- [x] T013 [US1] Unit test: register type and query by name/ID in modules/object/tests/unit/TypeRegistry_test.cpp; verify upstream Core memory usage

**Checkpoint**: US1 可独立验证；反射与类型注册符合契约

---

## Phase 4: User Story 2 - 完整序列化与引用解析 (Priority: P2)

**Goal**: ISerializer（Serialize、Deserialize、GetCurrentVersion、SetVersionMigration）、IVersionMigration、ObjectRef/GUID；跨资源引用仅读写 16 字节 GUID。

**Independent Test**: 序列化含 ObjectRef/GUID 的对象图后反序列化，引用与数据等价；版本迁移可调用。

- [x] T014 [P] [US2] Implement IVersionMigration interface in modules/object/include/te/object/VersionMigration.hpp and default impl in modules/object/src/VersionMigration.cpp per ABI (Migrate(buf, fromVersion, toVersion))
- [x] T015 [US2] Implement ISerializer (Serialize, Deserialize, GetCurrentVersion, SetVersionMigration) in modules/object/include/te/object/Serializer.hpp and modules/object/src/Serializer.cpp per ABI; support binary format; use Core Alloc for buffers
- [x] T016 [US2] Implement ObjectRef/GUID handling in serialization path (only 16-byte GUID per ABI; no pointer/path storage)
- [x] T017 [US2] Unit test: round-trip serialization with ObjectRef/GUID and version migration in modules/object/tests/unit/Serializer_roundtrip_test.cpp

**Checkpoint**: US2 可独立验证；序列化与契约一致

---

## Phase 5: User Story 3 - 属性系统与元数据 (Priority: P3)

**Goal**: PropertyBag（GetProperty、SetProperty、FindProperty）；与反射和序列化联动。

**Independent Test**: 通过 PropertyBag 读写属性元数据与默认值；范围/枚举约束可校验。

- [x] T018 [US3] Implement PropertyBag (GetProperty, SetProperty, FindProperty) in modules/object/include/te/object/PropertyBag.hpp and modules/object/src/PropertyBag.cpp per ABI
- [x] T019 [US3] Wire PropertyBag/PropertyDescriptor to TypeDescriptor and serialization per contract 能力列表
- [x] T020 [US3] Unit test: property get/set and FindProperty in modules/object/tests/unit/PropertyBag_test.cpp

**Checkpoint**: US3 可独立验证；属性系统与契约一致

---

## Phase 6: User Story 4 - 类型工厂与生命周期 (Priority: P4)

**Goal**: TypeRegistry::CreateInstance(TypeId)；与模块加载协调；使用 Core Alloc 分配。

**Independent Test**: 按 TypeId 调用 CreateInstance 得到实例；卸载或重复注册时行为有定义。

- [x] T021 [US4] Implement TypeRegistry::CreateInstance(TypeId id) in modules/object/src/TypeRegistry.cpp per ABI; use Core Alloc; return nullptr on failure
- [x] T022 [US4] Document or enforce lifecycle: use after unload/duplicate registration behavior per contract
- [x] T023 [US4] Unit test: CreateInstance and lifecycle in modules/object/tests/unit/CreateInstance_test.cpp; verify Alloc/Free pairing

**Checkpoint**: US4 可独立验证；类型工厂与契约一致

---

## Phase 7: Polish & Cross-Cutting

**Purpose**: 跨故事收尾与契约一致性检查

- [x] T024 Configure root CMakeLists.txt per docs/build-module-convention.md: TENENGINE_CMAKE_DIR, tenengine_resolve_my_dependencies, tenengine_add_module_test, enable_testing(); **执行前须已澄清根目录**（构建所在模块路径）；各子模块均使用源码方式；cmake 生成后检查头文件/源文件完整性与依赖
- [x] T025 [P] Verify all public headers in modules/object/include/te/object/ only declare types and APIs listed in contracts/002-object-ABI-full.md and specs/_contracts/002-object-public-api.md
- [x] T026 [P] Add or update quickstart in specs/002-object-fullversion-002/quickstart.md using only contract-declared API
- [x] T027 Run full unit tests (ctest --test-dir build -C Debug) and confirm no use of non-contract APIs from 001-Core or 002-Object

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies - can start immediately
- **Foundational (Phase 2)**: Depends on Setup - BLOCKS all user stories
- **User Stories (Phase 3–6)**: All depend on Foundational
  - US1 (P1) → US2 (P2) → US3 (P3) → US4 (P4) 或并行（若人力允许）
- **Polish (Phase 7)**: Depends on all user stories complete

### User Story Dependencies

- **US1 (P1)**: Can start after Foundational - No dependencies on other stories
- **US2 (P2)**: Depends on US1 (TypeRegistry, TypeDescriptor) - Independently testable
- **US3 (P3)**: Depends on US1 (PropertyDescriptor in TypeDescriptor) - Independently testable
- **US4 (P4)**: Depends on US1 (TypeRegistry) - Independently testable

### Parallel Opportunities

- T004–T008 within Phase 2 can run in parallel
- T010, T014, T018 can start in parallel after Foundational (different stories)
- T025, T026 within Polish can run in parallel

---

## Parallel Example: User Story 1

```text
# Foundational types (Phase 2) - parallel:
T004: TypeId.hpp
T005: PropertyDescriptor.hpp
T006: MethodDescriptor in TypeDescriptor.hpp
T007: Guid.hpp, ObjectRef.hpp
T008: SerializedBuffer.hpp

# US1 implementation - sequential:
T010 → T011 → T012 → T013
```

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Phase 1: Setup
2. Complete Phase 2: Foundational
3. Complete Phase 3: User Story 1
4. **STOP and VALIDATE**: Test TypeRegistry independently
5. Build and run ctest

### Incremental Delivery

1. Setup + Foundational → Foundation ready
2. Add US1 → Test independently (TypeRegistry_test)
3. Add US2 → Test independently (Serializer_roundtrip_test)
4. Add US3 → Test independently (PropertyBag_test)
5. Add US4 → Test independently (CreateInstance_test)
6. Polish → Full ctest pass

---

## Notes

- 实现须覆盖 `contracts/002-object-ABI-full.md` 全量 ABI，不得仅实现变化部分
- `specs/_contracts/002-object-ABI.md` 数据相关 TODO 中的 RegisterType、GetTypeInfo、GetTypeByName、Serialize、Deserialize、IVersionMigration::Migrate、formatVersion 约定、调用流程，**本次必须全部实现**
- 构建：执行 T024 前须已与用户确认 **根目录**；各子模块均使用源码方式；cmake 生成后检查依赖完整性
- 测试须覆盖上游 Core 能力（Alloc/Free 等），不得仅测本模块孤立逻辑
