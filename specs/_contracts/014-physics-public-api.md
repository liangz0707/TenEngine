# 契约：014-Physics 模块对外 API

## 适用模块

- **实现方**：014-Physics（L2；碰撞、刚体、查询、2D/3D）
- **对应规格**：`docs/module-specs/014-physics.md`
- **依赖**：001-Core、004-Scene、005-Entity

## 消费者

- 022-2D

## 能力列表

### 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| ShapeHandle | 碰撞体形状（盒/球/胶囊/网格等）；CreateShape、Filter、Layer | 创建后直至显式释放 |
| RigidBodyHandle | 刚体句柄；CreateRigidBody、SetMass、AddForce、Constraint、Integrate | 创建后直至显式释放 |
| SceneHandle | 物理场景；CreateScene、SyncFromScene、Step、Substep；与 Scene/Entity 变换同步 | 创建后直至显式释放 |
| QueryResult | 射线/形状扫描/重叠查询结果；Raycast、ShapeCast、Overlap、Filter、Group | 单次查询 |

### 能力（提供方保证）

| 序号 | 能力 | 说明 |
|------|------|------|
| 1 | Collision | CreateShape、CollisionEvent、Filter、Layer；碰撞检测与事件 |
| 2 | RigidBody | CreateRigidBody、SetMass、AddForce、Constraint、Integrate |
| 3 | Query | Raycast、ShapeCast、Overlap、Filter、Group |
| 4 | Scene | CreateScene、SyncFromScene、Step、Substep；与 Scene/Entity 同步 |

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。

## 约束

- 须在 Core、Scene、Entity 初始化之后使用；变换同步与 Scene/Entity 约定一致。2D 物理与 022-2D 的 Physics2D 桥接接口须明确。

## 实现状态

**待实现**：当前模块仅包含占位 CMakeLists.txt，无实际代码实现。上述能力列表描述了预期 API，待后续开发。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 014-Physics 契约 |
| 2026-02-05 | 统一目录；能力列表用表格 |
| 2026-02-22 | 同步代码：标注待实现状态（模块仅有占位 CMakeLists.txt） |
