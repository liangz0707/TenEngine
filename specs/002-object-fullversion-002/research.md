# Research: 002-Object Full Module Content

**Feature**: 002-object-fullversion-002  
**Date**: 2026-01-29

## Phase 0 Findings

### 1. 序列化格式与版本迁移

**Decision**: 支持二进制格式（最小实现），版本迁移通过 `IVersionMigration::Migrate` 在反序列化前调用；formatVersion 由各描述类型所属模块与本契约约定。

**Rationale**: 与 `docs/assets/resource-serialization.md` 一致；013-Resource、004-Scene 等下游在 Load 时按 GetTypeByName → Deserialize 流程；版本迁移为本 feature 必选能力。

**Alternatives considered**: 纯文本格式（JSON/YAML）增加解析开销；仅二进制可满足资源加载性能需求；文本可作为后续扩展。

### 2. GUID 与跨资源引用

**Decision**: 跨资源引用**仅读写 16 字节 GUID**，不存指针或路径；解析由 013-Resource 等消费者在 Load 阶段完成。

**Rationale**: 符合 `docs/assets/resource-serialization.md` §3、§4；002-Object 仅负责序列化层读写 GUID，解析与 013 约定留待集成。

### 3. 类型注册与 ITypeInfo/TypeDescriptor 对齐

**Decision**: 采用 TypeDescriptor（含 properties、methods、baseTypeId）作为类型元数据；与 public-api 及 `specs/_contracts/002-object-ABI.md` TODO 中的 ITypeInfo 语义等价。

**Rationale**: Unity 的 TypeInfo、Unreal 的 UClass 均提供类型名、大小、属性列表；本模块对齐 public-api 完整功能集。

### 4. 上游内存接口

**Decision**: 类型元数据、序列化缓冲分配使用 001-Core `Alloc`/`Free`；未链接 001-Core 时提供回退实现（std::malloc/free）。

**Rationale**: 与 002-object-fullversion-001 实现一致；Constitution §VI 要求构建引入真实子模块，回退仅用于 standalone 模式。
