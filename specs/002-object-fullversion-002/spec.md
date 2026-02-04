# Feature Specification: 002-Object Full Module Content

**Feature Branch**: `002-object-fullversion-002`  
**Created**: 2026-01-29  
**Status**: Draft  
**Input**: 本 feature 的完整模块规约见 `docs/module-specs/002-object.md`，契约见 `specs/_contracts/002-object-public-api.md`；**本 feature 实现完整模块内容**。

## 规约与契约引用 *(模块切片时必填)*

- **完整模块规约**：`docs/module-specs/002-object.md`（对象模型与元数据：反射、序列化、属性系统、类型注册）。
- **对外 API 契约**：`specs/_contracts/002-object-public-api.md`。
- **本模块范围**（本 feature 实现完整模块内容）：
  1. **反射**：类型注册、类型信息查询、属性/方法枚举、基类链、GetTypeByName/ById。
  2. **序列化**：序列化器抽象、二进制与文本格式、版本迁移（必选）、对象引用与 GUID 解析；往返等价性可验证。
  3. **属性系统**：属性描述、元数据、默认值、范围/枚举约束；与反射和序列化联动。
  4. **类型注册**：注册表、按模块注册、类型工厂 CreateInstance、生命周期；与 Core 模块加载协调。

**上游依赖**：本模块依赖 001-Core。依赖的上游 API 见 `specs/_contracts/001-core-public-api.md`；实现时只使用上游契约已声明的类型与 API（如分配器、容器、字符串等），不引入契约外的接口。

- **ABI 与构建规约**（`.specify/memory/constitution.md` §VI）：本模块须实现其 **ABI 文件**（`specs/_contracts/002-object-ABI.md`）中的**全部**符号与能力；构建须通过**引入真实子模块源码**（如 CMake `add_subdirectory`）满足依赖，**禁止**使用 stub、mock 或与契约不一致的代替实现作为长期方案。契约更新流程见 `specs/_contracts/README.md`。

- **第三方依赖**：第三方库引入说明在契约 `specs/_contracts/002-object-public-api.md` 中声明；本 spec 引用该契约即可。详见 `docs/third_party-integration-workflow.md`。

## User Scenarios & Testing *(mandatory)*

### User Story 1 - 类型注册与反射查询 (Priority: P1)

作为引擎或模块作者，需要在 Core 初始化之后注册类型并查询类型信息（属性/方法列表、基类链），以便下游（Scene、Entity、Resource 等）可发现与使用类型。

**Why this priority**: 类型注册与反射是序列化与属性系统的基础。

**Independent Test**: 注册类型后通过 GetTypeByName/ById 查询 TypeDescriptor（含属性/方法、基类链）；可独立于序列化验证。

**Acceptance Scenarios**:

1. **Given** Core 已初始化，**When** 调用 TypeRegistry::RegisterType 注册类型 T（含属性/方法描述），**Then** T 可被 GetTypeByName/ById 查询到，且 TypeDescriptor 包含属性/方法列表与基类链。
2. **Given** 已注册类型 T，**When** 查询基类链或属性枚举，**Then** 返回与规约一致的信息。

---

### User Story 2 - 完整序列化与引用解析 (Priority: P2)

作为需要持久化或传输对象图的调用方，需要将对象序列化（含版本、对象引用与 GUID 解析）并反序列化，保证往返等价性与引用正确性。

**Why this priority**: 完整序列化是 Resource、Editor 等下游依赖的能力。

**Independent Test**: 序列化含嵌套引用与 GUID 的对象图，反序列化后验证引用与数据等价；可验证版本迁移行为。

**Acceptance Scenarios**:

1. **Given** 已注册类型与序列化器，**When** 序列化含 ObjectRef/GUID 的对象图，**Then** 得到 SerializedBuffer，且引用可被解析。
2. **Given** 上述字节流，**When** 反序列化，**Then** 得到等价对象图，引用与 GUID 解析正确。
3. **Given** 旧版本序列化数据，**When** 使用版本迁移，**Then** 可升级到当前格式或行为有定义。

---

### User Story 3 - 属性系统与元数据 (Priority: P3)

作为编辑器或工具方，需要读写类型的属性描述、元数据、默认值及范围/枚举约束，并与序列化联动。

**Why this priority**: 属性系统支撑编辑器可见/可编辑与约束校验。

**Independent Test**: 通过 PropertyDescriptor/PropertyBag 读写属性元数据与默认值；可验证范围/枚举约束。

**Acceptance Scenarios**:

1. **Given** 已注册类型含属性描述，**When** 查询 PropertyDescriptor 或 PropertyBag，**Then** 获得元数据、默认值、范围/枚举约束。
2. **Given** 属性值，**When** 校验范围/枚举，**Then** 拒绝非法值或行为有定义。

---

### User Story 4 - 类型工厂与生命周期 (Priority: P4)

作为需要按类型创建实例的调用方，需要通过 TypeId 创建对象（CreateInstance），并与模块加载/卸载协调生命周期。

**Why this priority**: 类型工厂是运行时动态创建对象的基础。

**Independent Test**: 按 TypeId 调用 CreateInstance 得到实例；卸载或重复注册时行为有定义。

**Acceptance Scenarios**:

1. **Given** 类型已注册，**When** 调用 CreateInstance(typeId)，**Then** 返回该类型的实例（或约定句柄）。
2. **Given** 模块卸载或类型注销，**When** 后续使用已卸载类型的句柄，**Then** 行为有定义（如失效或报错）。

---

### Edge Cases

- Core 未初始化时调用 Object API：应失败或明确报错。
- 重复注册相同 TypeId/类型名：重复 TypeId 拒绝；重复类型名由实现约定，须在契约或实现中明确。
- 序列化/反序列化时类型未注册、或版本不兼容：应安全失败或按契约约定。
- 属性范围/枚举校验失败、或 GUID 无法解析：行为有定义，不崩溃。

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: 系统必须提供类型注册与反射能力（TypeRegistry::RegisterType、GetTypeByName/ById、TypeDescriptor 含属性/方法列表与基类链），且须在 Core 初始化之后使用。
- **FR-002**: 系统必须提供完整序列化能力：序列化器抽象、二进制与文本格式、版本迁移（必选）、对象引用与 GUID 解析；往返等价性可验证。
- **FR-003**: 系统必须提供属性系统：PropertyDescriptor、PropertyBag、元数据、默认值、范围/枚举约束；与反射和序列化联动。
- **FR-004**: 系统必须提供类型工厂 CreateInstance（按 TypeId 创建实例）及与模块加载协调的生命周期。
- **FR-005**: 系统必须仅使用 `specs/_contracts/001-core-public-api.md` 已声明的类型与 API；对外暴露与 `specs/_contracts/002-object-public-api.md` 能力列表一致的全部 API。

### Key Entities

- **TypeId / TypeDescriptor**：类型标识与描述；属性/方法列表、基类链；注册后直至卸载。
- **SerializedBuffer / ObjectRef**：序列化字节流、对象引用；由调用方管理缓冲；ObjectRef 与资源 GUID 对应。
- **GUID**：全局唯一标识，用于资源引用与解析。
- **PropertyBag / PropertyDescriptor**：属性描述、元数据、默认值、范围/枚举；与类型或实例绑定。
- **类型工厂**：CreateInstance、按 TypeId 创建；按调用约定。

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: 调用方能注册类型并查询完整 TypeDescriptor（含属性/方法、基类链），无未定义行为。
- **SC-002**: 调用方能序列化与反序列化含引用与 GUID 的对象图，往返等价且引用解析正确。
- **SC-003**: 调用方能通过属性系统读写元数据与约束，并与序列化联动。
- **SC-004**: 调用方能按 TypeId 创建实例，并与模块生命周期协调。
- **SC-005**: 实现与契约能力列表一致；仅依赖上游契约已声明的 API。

## Interface Contracts *(multi-agent sync)*

- **本模块实现的契约**：`specs/_contracts/002-object-public-api.md`（本 feature 实现完整能力列表：反射、序列化、属性系统、类型注册）。
- **本模块依赖的契约**：见下方 Dependencies。
- **ABI/构建**：须实现 ABI 中全部符号；构建须引入真实子模块代码，禁止长期使用 stub 或代替方案。

## Dependencies

- **001-Core**：`specs/_contracts/001-core-public-api.md`。依赖的上游 API 见该契约；实现时只使用契约中已声明的类型与接口（基础类型、容器、字符串、内存分配、日志等）。
- **GUID 与下游约定**：GUID 引用格式与 013-Resource 等消费者的约定留待集成或跨模块契约；本 feature 保证 GUID 可解析、可扩展。
- 依赖关系总览：`specs/_contracts/000-module-dependency-map.md`。
