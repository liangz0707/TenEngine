# 契约：005-Entity 模块对外 API

## 适用模块

- **实现方**：005-Entity（L1；实体/组件模型或 ECS；Entity-Node 1:1；ModelComponent 持 ResourceId/句柄，对 IResource 不可见）
- **对应规格**：`docs/module-specs/005-entity.md`
- **依赖**：001-Core、002-Object、004-Scene

## 消费者

- 014-Physics、015-Animation、020-Pipeline、024-Editor、026-Networking

## 能力列表

### 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| EntityId | 实体唯一标识；与 Scene 节点一一对应或映射 | 创建后直至销毁 |
| ComponentHandle | 组件实例句柄；按类型查询、挂载/卸载 | 与实体或显式移除同生命周期 |
| ModelComponent / RenderableComponent | 渲染组件；持有 ResourceId 或句柄，005 对 IResource 不可见；由 Pipeline 经 013 解析后对接 | 与实体或显式移除同生命周期 |
| Transform | 局部/世界变换（可与 Scene 共用或实体专用） | 与实体/节点同步 |
| ComponentQuery | 按类型/条件查询实体或组件；迭代接口 | 查询时有效 |

### 能力（提供方保证）

| 序号 | 能力 | 说明 |
|------|------|------|
| 1 | 实体 | CreateEntity、DestroyEntity、GetSceneNode、SetEnabled；生命周期与 Scene 节点可选绑定 |
| 2 | 组件 | RegisterComponentType、AddComponent、RemoveComponent、GetComponent；与 Object 反射联动；ModelComponent 仅存 ResourceId/句柄 |
| 3 | 变换 | GetLocalTransform、SetLocalTransform、GetWorldTransform；与 Scene 节点共用或专用 |
| 4 | 可选 ECS | RegisterSystem、ExecutionOrder、与主循环 Tick 集成 |

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。

## 约束

- 须在 Core、Object、Scene 可用之后使用；组件类型须在 Object 中注册。实体销毁时须释放或转移组件与绑定资源。ModelComponent 仅存 ResourceId/句柄；由 Pipeline 或上层经 013 解析为 IModelResource*。

## TODO 列表

（以下任务来自原 ABI 数据相关 TODO。）

- [ ] **注册**：引擎启动时注册 Component 类型（TransformComponent、ModelComponent 等）到 002 RegisterType。
- [ ] **数据**：Component 属性按 002 可序列化约定；跨资源引用仅存 ResourceId；ModelComponent.modelGuid。
- [ ] **接口**：GetModelGuid(IEntity*, ModelComponent*)/SetModelGuid；CreateEntityFromPrefab(prefabDesc)，prefabDesc 由调用方加载得到。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 005-Entity 契约 |
| 2026-02-05 | 统一目录；能力列表用表格；去除冗余衔接说明 |
