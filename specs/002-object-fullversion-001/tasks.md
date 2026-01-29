# Tasks: 002-Object Full Feature Set

**Input**: plan.md, spec.md, `specs/_contracts/002-object-public-api.md`  
**Authority**: 以 plan.md 与 `specs/_contracts/002-object-public-api.md` 为准；任务只暴露契约已声明的 API（见契约「本切片 002-object-fullversion-001 完整功能集」）。  
**Prerequisites**: 仅暴露契约已声明的类型与 API；仅使用 `specs/_contracts/001-core-public-api.md` 已声明的类型与接口。

**Organization**: 按 User Story 分组，每故事可独立实现与验证。

## Format: `[ID] [P?] [Story] Description`

- **[P]**: 可并行（不同文件、无依赖）
- **[Story]**: 所属 User Story（US1–US4）
- 描述中含具体文件路径；对外仅暴露契约 API 雏形中的签名与类型。

## Path Conventions (from plan.md)

- **库根**: `modules/object/`（或 `te-object/`）
- **对外头文件**: `include/te/object/*.hpp`（仅契约声明的 API）
- **实现**: `src/*.cpp`
- **测试**: `tests/unit/*.cpp`

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: 工程初始化与目录结构，与 plan 一致

- [x] T001 Create directory structure per plan (modules/object/include/te/object, modules/object/src, modules/object/tests/unit)
- [x] T002 [P] Add CMakeLists.txt at modules/object/ with C++17, link to 001-Core (only contract-declared APIs)
- [x] T003 [P] Add tests/CMakeLists.txt and wire unit test targets

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: 所有 User Story 依赖的公共类型与约束；仅使用 001-Core 契约与 002-object 契约已声明的类型

**Constraint**: 对外仅暴露 `specs/_contracts/002-object-public-api.md` 中「本切片（002-object-fullversion-001）完整功能集」所列类型与 API。

- [x] T004 [P] Declare TypeId and MethodDescriptor (minimal/placeholder) in include/te/object/TypeId.hpp per contract
- [x] T005 [P] Declare PropertyDescriptor struct in include/te/object/PropertyDescriptor.hpp per contract API 雏形
- [x] T006 [P] Declare GUID and ObjectRef in include/te/object/Guid.hpp and include/te/object/ObjectRef.hpp per contract
- [x] T007 Declare SerializedBuffer in include/te/object/SerializedBuffer.hpp per contract
- [x] T008 Ensure Core init precondition: document or hook that Object APIs require Core initialized (per contract 调用顺序与约束)

**Checkpoint**: 基础类型与契约一致；US 实现可开始

---

## Phase 3: User Story 1 - 类型注册与反射查询 (Priority: P1) — MVP

**Goal**: TypeRegistry::RegisterType、GetTypeByName/ById、TypeDescriptor 含属性/方法列表与基类链；仅暴露契约声明的 API。

**Independent Test**: 注册类型后通过 GetTypeByName/ById 查询到 TypeDescriptor（含属性/方法、基类链）。

- [x] T009 [P] [US1] Implement TypeDescriptor (id, name, size, properties, propertyCount, methods, methodCount, baseTypeId) in include/te/object/TypeDescriptor.hpp per contract
- [x] T010 [US1] Implement TypeRegistry::RegisterType (reject duplicate TypeId; duplicate name per implementation) in src/TypeRegistry.cpp, header in include/te/object/TypeRegistry.hpp
- [x] T011 [US1] Implement TypeRegistry::GetTypeByName and GetTypeById in src/TypeRegistry.cpp (expose only contract-declared signatures)
- [x] T012 [US1] Unit test: register type and query by name/ID in tests/unit/TypeRegistry_test.cpp

**Checkpoint**: US1 可独立验证；反射与类型注册符合契约

---

## Phase 4: User Story 2 - 完整序列化与引用解析 (Priority: P2)

**Goal**: ISerializer（Serialize/Deserialize、GetCurrentVersion、SetVersionMigration）、IVersionMigration、ObjectRef/GUID；仅暴露契约声明的 API。

**Independent Test**: 序列化含 ObjectRef/GUID 的对象图后反序列化，引用与数据等价；版本迁移可调用。

- [x] T013 [P] [US2] Implement IVersionMigration interface in include/te/object/VersionMigration.hpp and default/optional impl in src/VersionMigration.cpp per contract
- [x] T014 [US2] Implement ISerializer (Serialize, Deserialize, GetCurrentVersion, SetVersionMigration) in include/te/object/Serializer.hpp and src/Serializer.cpp; support binary and text via implementation/factory per contract
- [x] T015 [US2] Implement ObjectRef/GUID handling in serialization path (only contract-declared types; resolution semantics per contract 调用顺序与约束)
- [x] T016 [US2] Unit test: round-trip serialization with ObjectRef/GUID and version migration in tests/unit/Serializer_roundtrip_test.cpp

**Checkpoint**: US2 可独立验证；序列化与契约一致

---

## Phase 5: User Story 3 - 属性系统与元数据 (Priority: P3)

**Goal**: PropertyDescriptor、PropertyBag（GetProperty, SetProperty, FindProperty）；仅暴露契约声明的 API。

**Independent Test**: 通过 PropertyBag 读写属性元数据与默认值；范围/枚举约束可校验。

- [x] T017 [P] [US3] Implement PropertyDescriptor (name, valueTypeId, defaultValue) in include/te/object/PropertyDescriptor.hpp per contract
- [x] T018 [US3] Implement PropertyBag (GetProperty, SetProperty, FindProperty) in include/te/object/PropertyBag.hpp and src/PropertyBag.cpp per contract
- [x] T019 [US3] Wire PropertyBag/PropertyDescriptor to TypeDescriptor and serialization per contract 能力列表
- [x] T020 [US3] Unit test: property get/set and FindProperty in tests/unit/PropertyBag_test.cpp

**Checkpoint**: US3 可独立验证；属性系统与契约一致

---

## Phase 6: User Story 4 - 类型工厂与生命周期 (Priority: P4)

**Goal**: TypeRegistry::CreateInstance(TypeId)；与模块加载协调；仅暴露契约声明的 API。

**Independent Test**: 按 TypeId 调用 CreateInstance 得到实例；卸载或重复注册时行为有定义。

- [x] T021 [US4] Implement TypeRegistry::CreateInstance(TypeId id) in src/TypeRegistry.cpp per contract (allocation semantics per 调用约定)
- [x] T022 [US4] Document or enforce lifecycle: use after unload/duplicate registration behavior per contract
- [x] T023 [US4] Unit test: CreateInstance and lifecycle in tests/unit/CreateInstance_test.cpp

**Checkpoint**: US4 可独立验证；类型工厂与契约一致

---

## Phase 7: Polish & Cross-Cutting

**Purpose**: 跨故事收尾与契约一致性检查

- [x] T024 [P] Verify all public headers in include/te/object/ only declare types and APIs listed in specs/_contracts/002-object-public-api.md (fullversion-001 slice)
- [x] T025 [P] Add or update quickstart/build instructions in specs/002-object-fullversion-001/quickstart.md using only contract-declared API
- [x] T026 Run full unit tests and confirm no use of non-contract APIs from 001-Core or 002-Object

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 1 (Setup)**: 无依赖，可立即开始
- **Phase 2 (Foundational)**: 依赖 Phase 1；阻塞所有 User Story
- **Phase 3–6 (US1–US4)**: 依赖 Phase 2；US1 为 P1 建议先做，US2–US4 可在 US1 后并行或按优先级
- **Phase 7 (Polish)**: 依赖 Phase 3–6 完成

### User Story Dependencies

- **US1 (P1)**: 仅依赖 Foundational；无其他故事依赖
- **US2 (P2)**: 依赖 Foundational；可与 US1 共享 TypeRegistry/TypeDescriptor
- **US3 (P3)**: 依赖 Foundational；与 US1 反射联动，可与 US2 并行
- **US4 (P4)**: 依赖 US1（TypeRegistry）；可与 US2/US3 并行

### Contract Constraint

- 所有对外头文件与符号必须与 `specs/_contracts/002-object-public-api.md` 中「本切片（002-object-fullversion-001）完整功能集」一致。
- 实现中仅使用 `specs/_contracts/001-core-public-api.md` 已声明的类型与接口。

---

## Parallel Opportunities

- Phase 1: T002 与 T003 可并行
- Phase 2: T004–T007 可并行
- Phase 3: T009 可与 T010/T011 顺序执行后一起测 T012
- Phase 4: T013 与 T014 可部分并行；T016 依赖 T014–T015
- Phase 5: T017 与 T018 可并行
- Phase 7: T024 与 T025 可并行

---

## Implementation Strategy

### MVP First (US1 Only)

1. Phase 1 → Phase 2 → Phase 3  
2. 验证：注册类型并 GetTypeByName/ById 查询 TypeDescriptor  
3. 确认对外仅暴露契约 API 后即可交付 MVP

### Incremental Delivery

1. Phase 1 + 2 → 基础就绪  
2. + Phase 3 (US1) → 反射可用  
3. + Phase 4 (US2) → 序列化可用  
4. + Phase 5 (US3) → 属性系统可用  
5. + Phase 6 (US4) → 类型工厂可用  
6. Phase 7 → 收尾与契约校验

---

## Notes

- 任务只暴露契约已声明的 API；新增类型或签名须先更新 `specs/_contracts/002-object-public-api.md` 再实现。
- [P] 表示可并行；[USn] 表示所属 User Story，便于按故事交付与测试。
- 每任务完成后可提交；Phase 结束做一次契约一致性检查。
