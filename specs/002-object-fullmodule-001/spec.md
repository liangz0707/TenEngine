# Feature Specification: 002-Object 完整模块实现

**Feature Branch**: `002-object-fullmodule-001`  
**Created**: 2026-02-05  
**Status**: Draft  
**Input**: 本 feature 的完整模块规约见 `docs/module-specs/002-object.md`，契约见 `specs/_contracts/002-object-public-api.md`；**本 feature 实现完整模块内容**。

## 规约与契约引用 *(模块切片时必填)*

- **完整模块规约**：`docs/module-specs/002-object.md`（002-Object 对象模型与元数据：反射、序列化、属性系统、类型注册、GUID；仅依赖 Core）。
- **本模块范围**（本 feature 实现完整模块内容）：
  1. **Reflection**：TypeDescriptor、PropertyDescriptor/MethodDescriptor、类型名/ID 解析、基类链；TypeRegistry::RegisterType、GetTypeByName、GetTypeById。
  2. **TypeRegistry**：注册表、按模块注册、类型工厂 CreateInstance、与 Core 模块加载协调。
  3. **Serialization**：ISerializer 接口、Serialize/Deserialize、IVersionMigration、SerializedBuffer、ObjectRef 与 GUID 解析；往返等价可验证。
  4. **Properties**：PropertyBag 接口、PropertyDescriptor、元数据、默认值、范围/枚举约束；与反射和序列化联动。

实现时只使用**本 feature 依赖的上游契约**（`specs/_contracts/001-core-public-api.md`）中已声明的类型与 API；不实现本规约未列出的能力。**上游依赖**：002-Object 依赖 001-Core；依赖的上游 API 见 `specs/_contracts/001-core-public-api.md`（如 Alloc/Free、容器、字符串、日志等），实现时仅使用该契约已声明的类型与接口。

- **ABI 与构建规约**（`.specify/memory/constitution.md` §VI）：本模块须实现其 **ABI 文件**（`specs/_contracts/002-object-ABI.md`）中的**全部**符号与能力；构建须通过**引入真实子模块源码**（如 CMake `add_subdirectory`）满足依赖，**禁止**使用 stub、mock 或与契约不一致的代替实现作为长期方案。**契约更新**：接口变更须在对应 **ABI 文件**中**增补或替换**对应的 ABI 条目；下游所需接口须在**上游模块的 ABI 文件**中以 **TODO** 登记。详见 `specs/_contracts/README.md`「契约更新流程」。

- **第三方依赖**：第三方库引入说明在契约 `specs/_contracts/002-object-public-api.md` 中声明；本 spec 引用该契约即可，不在 spec 中重复列出。Plan 从 public-api 读取并自动填入「第三方依赖」小节。详见 `docs/third_party-integration-workflow.md`。

## User Scenarios & Testing *(mandatory)*

### User Story 1 - 类型注册与查询 (Priority: P1)

下游模块（如 013-Resource、004-Scene）在启动或加载时向 002-Object 注册类型描述（TypeDescriptor），并通过类型名或 TypeId 查询类型信息。

**Why this priority**: 反射与类型工厂是所有序列化与资源描述解析的前提。

**Independent Test**: RegisterType 注册一类型后，GetTypeByName/GetTypeById 返回一致描述；重复相同 TypeId 注册被拒绝。

**Acceptance Scenarios**:

1. **Given** Core 已初始化，**When** 调用 RegisterType(desc)，**Then** 返回 true；同一 TypeId 再次 RegisterType 返回 false。
2. **Given** 已注册类型 "MyAssetDesc"，**When** GetTypeByName("MyAssetDesc")，**Then** 返回非空 TypeDescriptor，且 id/name/size 与注册一致。
3. **Given** 已注册类型，**When** GetTypeById(id)，**Then** 返回与 GetTypeByName 一致的描述；未注册 id 返回 nullptr。

---

### User Story 2 - 类型工厂创建实例 (Priority: P1)

调用方通过 TypeRegistry::CreateInstance(TypeId) 创建已注册类型的实例，内存由 Core Alloc 分配。

**Why this priority**: 资源加载与反序列化依赖按类型创建对象。

**Independent Test**: 注册类型后 CreateInstance(id) 返回非空指针，可安全用于后续 Deserialize；非法 id 返回 nullptr。

**Acceptance Scenarios**:

1. **Given** 已注册类型 T，**When** CreateInstance(T 的 TypeId)，**Then** 返回非 nullptr，且可对该指针进行 Deserialize 等操作。
2. **Given** 未注册的 TypeId 或无效 id，**When** CreateInstance(id)，**Then** 返回 nullptr。

---

### User Story 3 - 序列化与反序列化往返 (Priority: P1)

调用方使用 ISerializer::Serialize 将对象写入 SerializedBuffer，再使用 Deserialize 从缓冲恢复对象；往返后数据等价（可由业务层或测试验证）。

**Why this priority**: 资源保存与加载、场景/资产持久化依赖序列化能力。

**Independent Test**: 对一已注册类型实例 Serialize 得到 buffer，再 Deserialize 到新实例，关键属性一致。

**Acceptance Scenarios**:

1. **Given** 已实现 ISerializer 且对象 obj 与 TypeId 对应，**When** Serialize(out, obj, typeId)，**Then** 返回 true，out 的 data/size 有效。
2. **Given** 上述 buffer，**When** Deserialize(buf, newObj, typeId)，**Then** 返回 true，newObj 与 obj 在契约约定字段上等价。
3. **Given** 版本迁移已通过 SetVersionMigration 设置，**When** 反序列化低版本数据，**Then** 先 Migrate 再 Deserialize，结果符合当前版本语义。

---

### User Story 4 - 属性读写与 PropertyBag (Priority: P2)

调用方通过 PropertyBag::GetProperty/SetProperty/FindProperty 按名称读写属性，与反射和序列化联动。

**Why this priority**: 编辑器、脚本与资源描述均依赖属性元数据与运行时读写。

**Independent Test**: 对实现 PropertyBag 的对象，GetProperty/SetProperty 与 FindProperty 行为符合 PropertyDescriptor 元数据。

**Acceptance Scenarios**:

1. **Given** 对象实现 PropertyBag 且已注册属性 "name"，**When** SetProperty(value, "name") 再 GetProperty(out, "name")，**Then** 返回 true 且 out 与 value 一致。
2. **Given** 同上，**When** FindProperty("name")，**Then** 返回非空 PropertyDescriptor，与类型注册一致。
3. **Given** 不存在的属性名，**When** GetProperty/FindProperty，**Then** 返回 false 或 nullptr。

---

### Edge Cases

- 重复 RegisterType(相同 TypeId)：拒绝并返回 false。
- CreateInstance(无效/未注册 TypeId)：返回 nullptr。
- Serialize/Deserialize 时 buffer 为 null 或 size 不足：返回 false 或明确错误语义。
- 反序列化版本低于当前：先 IVersionMigration::Migrate 再 Deserialize；Migrate 失败则整体失败。
- 跨资源引用：仅读写 16 字节 GUID（ObjectRef），不存指针或路径；由上游 013/004 等负责解析。

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**：模块 MUST 提供 TypeRegistry::RegisterType、GetTypeByName、GetTypeById，行为符合 `specs/_contracts/002-object-ABI.md`。
- **FR-002**：模块 MUST 提供 TypeRegistry::CreateInstance(TypeId)，使用 001-Core Alloc 分配；失败返回 nullptr。
- **FR-003**：模块 MUST 提供 ISerializer::Serialize、Deserialize、GetCurrentVersion、SetVersionMigration，及 IVersionMigration::Migrate，满足版本化与往返等价。
- **FR-004**：模块 MUST 提供 PropertyBag::GetProperty、SetProperty、FindProperty，与 TypeDescriptor/PropertyDescriptor 一致。
- **FR-005**：模块 MUST 暴露 TypeId、TypeDescriptor、PropertyDescriptor、MethodDescriptor、SerializedBuffer、ObjectRef、GUID 等类型，且与 ABI 表一致。
- **FR-006**：实现 MUST 仅在 Core 初始化之后使用；仅使用 `specs/_contracts/001-core-public-api.md` 中已声明的类型与 API 作为上游依赖。

### Key Entities

- **TypeId**：类型唯一标识；0 或 kInvalidTypeId 表示无效。
- **TypeDescriptor**：类型描述，含 id、name、size、properties、propertyCount、methods、methodCount、baseTypeId。
- **PropertyDescriptor**：属性描述，含 name、valueTypeId、defaultValue。
- **SerializedBuffer**：data、size、capacity；由调用方分配与释放。
- **ObjectRef / GUID**：跨资源引用仅读写 16 字节；与资源/对象绑定。
- **ISerializer / IVersionMigration**：序列化与版本迁移接口；与 TypeRegistry 协同完成反序列化流程。

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**：构建通过，te_object 库可被下游链接，且仅依赖 001-Core 契约声明的 API。
- **SC-002**：单元测试覆盖 TypeRegistry 注册/查询/CreateInstance、ISerializer 往返、PropertyBag 读写；全部通过。
- **SC-003**：对外符号与 `specs/_contracts/002-object-ABI.md` 一致，无未声明公开 API。
- **SC-004**：在 Core 初始化后，可完成「注册类型 → CreateInstance → Serialize → Deserialize」完整流程且数据等价。

## Interface Contracts *(multi-agent sync)*

- **本模块实现的契约**：`specs/_contracts/002-object-public-api.md`（与 002-object-ABI.md 一致）。
- **本模块依赖的契约**：`specs/_contracts/001-core-public-api.md`（实现时只使用其中已声明的类型与 API）。
- **ABI/构建**：须实现 `specs/_contracts/002-object-ABI.md` 中全部符号；构建须引入真实 001-Core 子模块或满足契约的依赖，禁止长期使用 stub 或代替方案。接口变更须在 ABI 文件中更新；下游所需接口须在上游 ABI 中以 TODO 登记。

## Dependencies

- **001-Core**：`specs/_contracts/001-core-public-api.md`。依赖其声明的 Alloc/Free、容器、字符串、日志等；实现时仅使用该契约已声明的类型与 API。
- 依赖关系总览见 `specs/_contracts/000-module-dependency-map.md`。
