# Feature Specification: 002-object minimal

**Feature Branch**: `002-object-minimal`  
**Created**: 2025-01-28  
**Status**: Draft  
**Input**: User description: "本 feature 的完整模块规约见 docs/module-specs/002-object.md，对外 API 契约见 specs/_contracts/002-object-public-api.md。依赖的 Core API 见 specs/_contracts/001-core-public-api.md。本 feature 仅实现其中最小子集：类型注册 TypeRegistry::RegisterType、简单序列化接口（必须显式枚举）。spec.md 中引用规约与契约，只描述本切片范围；不重复完整模块规约。实现时只使用 001-Core 契约中已声明的类型与 API。"

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Type registration (Priority: P1)

As an engine or module author, I register my types with the Object module (TypeRegistry::RegisterType) so that downstream systems (Scene, Entity, Resource, etc.) can discover and use them via TypeId/TypeDescriptor.

**Why this priority**: Type registration is the foundation for reflection and serialization; without it, no types can be serialized or introspected.

**Independent Test**: Can be fully tested by registering one or more types, then querying TypeId/TypeDescriptor (e.g. GetTypeByName/ById). Delivers measurable value: types are discoverable.

**Acceptance Scenarios**:

1. **Given** Core is initialized and Object module is ready, **When** I call TypeRegistry::RegisterType for a type T with a unique name, **Then** T is registered and I can obtain TypeId/TypeDescriptor for it.
2. **Given** type T is registered, **When** I query by type name or TypeId, **Then** I receive the correct TypeDescriptor (or equivalent) and no duplicate registration occurs for the same name.

---

### User Story 2 - Simple serialization (Priority: P2)

As an engine or module author, I serialize and deserialize objects using the Object module’s minimal serialization API so that I can persist or transfer object graphs (within this slice: basic types and registered type references).

**Why this priority**: Serialization builds on type registration; it is the next minimal capability to enable persistence and tooling.

**Independent Test**: Can be fully tested by serializing a small object graph to a buffer and deserializing it back, then verifying round-trip equivalence for the in-scope types.

**Acceptance Scenarios**:

1. **Given** types are registered and I have an object instance, **When** I invoke the minimal serialization API to serialize to a buffer, **Then** I receive a SerializedBuffer (or equivalent) without crashing and using only Core-allocated memory.
2. **Given** a SerializedBuffer produced in-scope, **When** I invoke the minimal deserialization API, **Then** I obtain an equivalent object graph (for in-scope types) and can verify round-trip equality.

---

### Edge Cases

- What happens when TypeRegistry::RegisterType is called with a duplicate type name? (Reject, overwrite, or idempotent—document behaviour.)
- How does the system handle serialization of unregistered types in this minimal slice? (e.g. reject or restrict to registered types only.)
- What happens when Core is not initialized before using Object APIs? (Fail fast, documented precondition.)

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: The feature MUST provide TypeRegistry::RegisterType (or equivalent) to register types; registration MUST be queryable by type name and TypeId.
- **FR-002**: The feature MUST provide a minimal serialization interface to serialize registered types to a buffer and deserialize back, using only 001-Core契约中已声明的类型与 API (e.g. Alloc/Free, containers, Log).
- **FR-003**: The module MUST depend only on `specs/_contracts/001-core-public-api.md`; no use of internal or undocumented Core APIs.
- **FR-004**: Type descriptors and serialization format MUST remain within the scope of this slice (TypeRegistry::RegisterType + simple serialization); no full property system or generic reflection in this feature.
- **FR-005**: Behaviour MUST align with `specs/_contracts/002-object-public-api.md` (能力列表、类型与句柄、调用顺序与约束) for the in-scope subset.

### Key Entities *(include if feature involves data)*

- **TypeRegistry**: Registry of registered types; provides RegisterType and lookup by name/TypeId.
- **TypeId / TypeDescriptor**: Identifier and descriptor for a registered type; attributes essential for serialization (e.g. size, name).
- **SerializedBuffer**: Opaque buffer holding serialized data; lifecycle and ownership per contract (e.g. caller-managed).
- **ObjectRef**: Reference to an object in serialized form; used for references within object graphs (minimal support in this slice).

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: Developers can register at least one type and resolve it by name/TypeId without runtime errors.
- **SC-002**: A simple object graph (in-scope types only) can be serialized and deserialized with round-trip equivalence in unit tests.
- **SC-003**: No dependencies on APIs outside `specs/_contracts/001-core-public-api.md`; static or runtime checks (as agreed) confirm contract compliance.
- **SC-004**: Spec and implementation stay within the slice (TypeRegistry::RegisterType + simple serialization); no scope creep to full reflection or property system.

## Interface Contracts *(multi-agent sync)*

- **本模块实现的契约**（若有）: `specs/_contracts/002-object-public-api.md`（本 feature 实现其最小子集：类型注册、简单序列化）  
- **本模块依赖的契约**: 见下方 Dependencies；实现时只使用契约中声明的类型与接口。

## Dependencies

- **001-Core**: `specs/_contracts/001-core-public-api.md`。本 feature 仅使用其中已声明的类型与 API（如 Alloc/Free、String、Log、容器等）；不在契约内的接口禁止使用。
- **规约与契约**: 完整模块规约见 `docs/module-specs/002-object.md`；本 feature 只实现最小子集并引用上述规约与契约，不重复全文。
