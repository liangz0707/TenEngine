# Research: 002-Object 完整模块实现

**Feature**: 002-object-fullmodule-001 | **Phase 0**

## 1. 类型注册表存储与查找

**Decision**: 使用按 TypeId 的线性表或数组 + 按类型名的哈希表（Map）双索引；TypeId 为连续或稀疏由实现决定，重复 RegisterType(相同 id) 拒绝。

**Rationale**: GetTypeById 需 O(1) 或 O(log n)；GetTypeByName 需按名查找，Map 或类似结构满足。与 Unreal UClass、Unity Type 注册模式一致。

**Alternatives considered**: 仅按 id 数组 + 线性扫描 name（简单但 GetTypeByName 为 O(n)）；仅按 name Map（GetTypeById 需二次查找）。双索引在类型数量数百量级下可接受。

## 2. 序列化格式与 ISerializer 默认实现

**Decision**: ABI 约定序列化器抽象（ISerializer 接口）；本模块提供**至少一种**可用的默认实现（如二进制格式），满足 Serialize/Deserialize 往返等价。具体二进制布局（字节序、头、版本字）在 data-model 或实现中约定；可选支持文本格式由后续扩展。

**Rationale**: 契约要求「二进制/文本格式」；先实现二进制以保证 013/004 等下游可反序列化资源描述。版本迁移通过 IVersionMigration 在反序列化前调用。

**Alternatives considered**: 仅定义接口、不提供默认实现——会迫使每个类型自写序列化，不利于 AssetDesc 等通用描述；故至少提供一种默认实现。

## 3. CreateInstance 与 Core Alloc

**Decision**: CreateInstance(TypeId) 内部调用 001-Core 的 Alloc（或 GetDefaultAllocator()->Alloc）分配 size 字节（来自 TypeDescriptor::size），对齐按实现约定（如 alignof(max_align_t)）；失败返回 nullptr。不负责调用构造函数；若类型需可构造，由注册时传入的工厂或由调用方在分配后 placement new。

**Rationale**: 契约明确「使用 Core Alloc 分配」；001-core-public-api 声明 Alloc/Free、GetDefaultAllocator。C++ 侧 POD 或带默认构造的类型可由实现选择在 CreateInstance 内 placement new，或由调用方负责。

**Alternatives considered**: 使用 malloc/free——违反「仅使用上游契约 API」；故必须使用 Core Alloc。

## 4. PropertyBag 与 PropertyDescriptor

**Decision**: PropertyBag 为抽象接口；可提供基于 TypeDescriptor 的默认实现（按 name 索引属性、void* 读写），与反射和序列化联动。PropertyDescriptor 为值类型，含 name、valueTypeId、defaultValue；元数据与类型注册一致。

**Rationale**: 契约要求 GetProperty/SetProperty/FindProperty；与 TypeDescriptor 的 properties 列表一致即可。默认实现便于 AssetDesc 等通用结构复用。

**Alternatives considered**: 仅接口、无默认实现——会增加下游样板代码；提供默认实现更符合「完整模块」范围。

## 5. MethodDescriptor 占位

**Decision**: MethodDescriptor 按 ABI 为「方法描述（占位）」；本 feature 可定义最小 struct（如 name、methodSignature 或空），不实现运行时方法调用；满足 ABI 表「本切片可为占位或最小集」。

**Rationale**: 契约与 ABI 允许占位；反射完整方法调用可后续迭代。

**Alternatives considered**: 不导出 MethodDescriptor——ABI 已声明，需有类型定义。

## 6. ObjectRef 与 GUID

**Decision**: ObjectRef 为 16 字节（uint8_t guid[16]）；GUID 同。跨资源引用仅读写此 16 字节；不存指针或路径。生成与解析由 013/004 等调用方负责；002 仅提供类型与序列化时读写 GUID 的约定。

**Rationale**: 与 `specs/_contracts/002-object-ABI.md` 及「数据约定」一致。

---

*Phase 0 无 NEEDS CLARIFICATION；上述决策用于 Phase 1 设计与 tasks 分解。*
