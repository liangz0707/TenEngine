# US-entity-002：实体序列化与预制体（Prefab、序列化/反序列化、引用解析）

- **标题**：Entity 及其 Component 可**序列化**为字节流或资产文件、并可**反序列化**还原；**预制体**（Prefab）为可复用的实体模板，实例化时解析引用（Resource、Entity）；与 002-Object 反射/序列化、013-Resource 引用对接。
- **编号**：US-entity-002

---

## 1. 角色/触发

- **角色**：Editor、场景加载、网络复制
- **触发**：需要**保存** Entity 树为预制体或场景片段、**加载**时反序列化并还原 Entity 与 Component；预制体**实例化**时解析对资源与其他实体的引用（GUID/ResourceId）。

---

## 2. 端到端流程

1. **序列化**：调用方调用 **serializeEntity(entity)** 或 **serializeToPrefab(entity, path)**；Entity 模块（可选依赖 002-Object）将 Entity 树与各 Component 数据序列化为字节流或文件；**引用**（资源、其他 Entity）以 **GUID/ResourceId** 形式写出。
2. **反序列化**：**deserializeEntity(bytes)** 或 **loadPrefab(path)**；Entity 模块解析字节/文件，创建 Entity 树与 Component、填充数据；**引用解析**：GUID → Resource 加载或 Entity 引用，由 Resource/Scene 协同。
3. **预制体实例化**：**instantiatePrefab(prefabHandle)**；创建 Prefab 的副本 Entity 树、解析引用（资源句柄、父子 Entity）；与场景加载或运行时生成物体一致。
4. 与 002-Object 反射对接时，Component 类型与属性可按 TypeId/PropertyDescriptor 序列化；与 013-Resource 对接时，资源引用为 GUID，反序列化时通过 requestLoadAsync 或 getCached 解析。

---

## 3. 涉及模块

| 模块 | 职责摘要 |
|------|----------|
| 005-Entity | serializeEntity、deserializeEntity、instantiatePrefab、PrefabHandle；与 002-Object 反射/序列化、013-Resource 引用解析对接 |
| 002-Object | 类型信息、属性描述、序列化协议（可选） |
| 013-Resource | GUID/ResourceId、引用解析、getCached 或 requestLoadAsync |
| 004-Scene | 反序列化后 Entity 挂入场景、父子关系 |

---

## 4. 每模块职责与 I/O

### 005-Entity

- **职责**：提供 **serializeEntity**、**deserializeEntity**、**saveAsPrefab**、**loadPrefab**、**instantiatePrefab**；引用以 GUID 写出、反序列化时解析；与 002-Object、013-Resource、004-Scene 协同。
- **输入**：Entity、路径、字节流、PrefabHandle。
- **输出**：字节流、Prefab 文件、实例化后的 Entity 树；引用已解析。

---

## 5. 派生 ABI（与契约对齐）

- **005-entity-ABI**：serializeEntity、deserializeEntity、instantiatePrefab、PrefabHandle；与 002-Object、013-Resource 对接。详见 `specs/_contracts/005-entity-ABI.md`。

---

## 6. 验收要点

- Entity 可序列化/反序列化；预制体可保存、加载、实例化；引用（资源、Entity）可正确解析。
