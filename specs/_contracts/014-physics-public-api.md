# 契约：014-Physics 模块对外 API

## 适用模块

- **实现方**：**014-Physics**（碰撞、刚体与查询，2D/3D）
- **对应规格**：`docs/module-specs/014-physics.md`
- **依赖**：001-Core（001-core-public-api）、004-Scene（004-scene-public-api）、005-Entity（005-entity-public-api）

## 消费者（T0 下游）

- 022-2D（2D 碰撞体/刚体、Physics2D 桥接）

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。
- 当前契约版本：（由实现或计划阶段填写）

## 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| ShapeHandle | 碰撞体形状（盒/球/胶囊/网格等）；CreateShape、Filter、Layer | 创建后直至显式释放 |
| RigidBodyHandle | 刚体句柄；CreateRigidBody、SetMass、AddForce、Constraint、Integrate | 创建后直至显式释放 |
| SceneHandle | 物理场景；CreateScene、SyncFromScene、Step、Substep；与 Scene/Entity 变换同步 | 创建后直至显式释放 |
| QueryResult | 射线/形状扫描/重叠查询结果；Raycast、ShapeCast、Overlap、Filter、Group | 单次查询 |

下游仅通过上述类型与句柄访问；无直接 GPU 资源，调试绘制可由 Pipeline 或独立 DebugDraw 接口完成。

## 能力列表（提供方保证）

1. **Collision**：CreateShape、CollisionEvent、Filter、Layer；碰撞检测与事件。
2. **RigidBody**：CreateRigidBody、SetMass、AddForce、Constraint、Integrate。
3. **Query**：Raycast、ShapeCast、Overlap、Filter、Group。
4. **Scene**：CreateScene、SyncFromScene、Step、Substep；与 Scene/Entity 同步。

## 调用顺序与约束

- 须在 Core、Scene、Entity 初始化之后使用；变换同步与 Scene/Entity 约定一致。
- 2D 物理与 022-2D 的 Physics2D 桥接接口须明确。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 每模块一契约：014-Physics 对应本契约；与 docs/module-specs/014-physics.md 一致 |
