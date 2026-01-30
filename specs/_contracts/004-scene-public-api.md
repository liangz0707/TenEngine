# 契约：004-Scene 模块对外 API

## 适用模块

- **实现方**：**004-Scene**（T0 场景/关卡与层次结构）
- **对应规格**：`docs/module-specs/004-scene.md`
- **依赖**：001-Core（001-core-public-api）, 002-Object（002-object-public-api）

## 消费者（T0 下游）

- 005-Entity, 014-Physics, 020-Pipeline, 022-2D, 024-Editor。

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。
- 当前契约版本：（由实现或计划阶段填写）

## 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| SceneRef / WorldRef | 场景/世界容器引用 | 加载后直至卸载 |
| NodeId | 场景图节点 ID；父子关系、层级路径 | 与节点同生命周期 |
| Transform | 局部/世界变换（与 Core.Math 或共用类型一致） | 与节点/实体同步 |
| HierarchyIterator | 层级遍历、按名/按类型查找 | 迭代器，单次有效或由调用方管理 |
| Active 状态 | 节点/子树是否参与更新/渲染 | 与节点绑定 |
| Level 资源句柄 | 关卡加载/卸载边界；与 Resource 配合 | 按 Resource 契约 |

## 能力列表（提供方保证）

1. **场景图**：节点树、父子关系、局部/世界变换、脏标记与变换更新。
2. **层级**：Traverse、FindByName/ByType、GetPath、GetId、SetActive。
3. **World/Level**：GetCurrentWorld、SetActiveWorld、LoadLevel、UnloadLevel；与 Entity 根实体挂接、与 Resource 关卡引用对接。
4. **激活/禁用**：节点与子树参与更新/渲染的开关；下游可查询 Active 状态。

## 调用顺序与约束

- 须在 Core、Object 可用之后使用；关卡加载可与 013-Resource 约定句柄与加载时机。
- 变换更新顺序与脏标记须保证一致性；多 World 时当前活动场景切换语义明确。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 按 004-Scene 模块规格与依赖表新增契约 |
| 2026-01-29 | 契约更新由 plan 004-scene-fullversion-001 同步 |
| 2026-01-28 | 004-scene-ABI 补全：参考 Unity SceneManager/Transform、UE UWorld/Level 流式，补全 LoadScene/LoadSceneAsync/UnloadScene、GetActiveScene/SetActiveScene、ISceneWorld（CreateEntity/DestroyEntity/GetEntities/GetRootNodes/UpdateTransforms/FindNodeByName/FindNodeById/CreateNode）、ISceneNode（GetParent/SetParent/GetChildren、GetLocalTransform/SetLocalTransform、GetWorldTransform/GetWorldMatrix、GetNodeId/GetPath、SetActive/IsActive）、LoadSceneMode、SceneRef、NodeId |
