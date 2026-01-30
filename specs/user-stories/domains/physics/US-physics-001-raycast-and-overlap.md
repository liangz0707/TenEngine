# US-physics-001：射线检测与形状查询（Raycast、Overlap、与 Scene/Entity 对接）

- **标题**：游戏逻辑或 Editor 可进行**射线检测**（Raycast）与**形状重叠查询**（Overlap）；命中结果包含碰撞体/Entity 引用、交点、法线等；物理世界与 Scene/Entity 变换同步。
- **编号**：US-physics-001

---

## 1. 角色/触发

- **角色**：游戏逻辑、Editor（如点击拾取）
- **触发**：需要**从某点某方向发射射线**，检测第一个或全部命中；或**在某区域/形状内**查询重叠的碰撞体；结果用于拾取、技能检测、AI 等。

---

## 2. 端到端流程

1. **Physics** 模块持有**物理场景**（与 Scene 或 World 对应）；碰撞体与 **Entity** 或节点绑定，变换与 Scene/Entity **同步**（SyncFromScene 或每帧同步）。
2. 调用方调用 **raycast(origin, direction, maxDistance, filter, result)** 或 **raycastAll(...)**；Physics 返回命中列表（HitResult：entity/collider、point、normal、distance）。
3. 调用方调用 **overlapSphere(center, radius, filter, results)** 或 **overlapBox** 等；Physics 返回重叠的碰撞体/Entity 列表。
4. 过滤（filter）可按 Layer、Tag、Entity 类型等，由实现与 005-Entity/004-Scene 约定。

---

## 3. 涉及模块

| 模块 | 职责摘要 |
|------|----------|
| 014-Physics | raycast、raycastAll、overlapSphere、overlapBox；HitResult、过滤；物理场景与 Scene/Entity 变换同步 |
| 004-Scene / 005-Entity | 提供变换、Entity 引用；碰撞体与 Entity 绑定 |
| 020-Pipeline / 024-Editor | 消费射线结果（如点击拾取） |

---

## 4. 每模块职责与 I/O

### 014-Physics

- **职责**：提供 **raycast**、**raycastAll**、**overlapSphere**、**overlapBox** 等；HitResult（entity、point、normal、distance）；过滤参数；物理场景 **SyncFromScene** 或每帧同步 Entity 变换。
- **输入**：射线/形状参数、过滤、最大距离等。
- **输出**：命中或重叠列表；供游戏逻辑/Editor 使用。

---

## 5. 派生 ABI（与契约对齐）

- **014-physics-ABI**：raycast、raycastAll、overlapSphere、overlapBox、HitResult、SceneHandle、SyncFromScene。详见 `specs/_contracts/014-physics-ABI.md`。

---

## 6. 验收要点

- 可进行射线检测与形状重叠查询；命中结果含 Entity/碰撞体、交点、法线。
- 物理场景与 Scene/Entity 变换同步。
