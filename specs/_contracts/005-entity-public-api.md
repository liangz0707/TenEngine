# 契约：005-Entity 模块对外 API

## 适用模块

- **实现方**：**005-Entity**（T0 实体与组件模型）
- **对应规格**：`docs/module-specs/005-entity.md`
- **依赖**：001-Core（001-core-public-api）, 002-Object（002-object-public-api）, 004-Scene（004-scene-public-api）

## 消费者（T0 下游）

- 014-Physics, 015-Animation, 020-Pipeline, 026-Networking, 024-Editor, 027-XR。

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。
- 当前契约版本：（由实现或计划阶段填写）

## 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| EntityId | 实体唯一标识；与 Scene 节点关联或独立 | 创建后直至销毁 |
| ComponentHandle | 组件实例句柄；按类型查询、挂载/卸载 | 与实体或显式移除同生命周期 |
| Transform | 局部/世界变换（可与 Scene 共用或实体专用） | 与实体/节点同步 |
| ComponentQuery | 按类型/条件查询实体或组件；迭代接口 | 查询时有效 |

## 能力列表（提供方保证）

1. **实体**：CreateEntity、DestroyEntity、GetSceneNode、SetEnabled；生命周期与 Scene 节点可选绑定。
2. **组件**：RegisterComponentType、AddComponent、RemoveComponent、GetComponent；与 Object 反射联动。
3. **变换**：GetLocalTransform、SetLocalTransform、GetWorldTransform；与 Scene 节点共用或专用。
4. **可选 ECS**：RegisterSystem、ExecutionOrder、与主循环 Tick 集成；与传统组件模型边界明确。

## 调用顺序与约束

- 须在 Core、Object、Scene 可用之后使用；组件类型须在 Object 中注册。
- 实体销毁时须释放或转移组件与绑定资源；下游（Physics、Pipeline、Networking）依赖 EntityId 与 ComponentHandle 稳定性约定。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 按 005-Entity 模块规格与依赖表新增契约 |
| 2025-01-30 | 契约更新由 plan 005-entity-full 同步 |