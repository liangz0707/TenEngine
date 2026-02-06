# 契约：005-Entity 模块对外 API

## 适用模块

- **实现方**：005-Entity（L1；实体/组件模型或 ECS；Entity直接实现ISceneNode接口，Entity-Node 1:1）
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
| Component | 组件基类；所有Component必须继承此类 | 与实体或显式移除同生命周期 |
| Transform | 局部/世界变换（通过ISceneNode接口管理，与Scene共用） | 与实体/节点同步 |
| ComponentQuery | 按类型/条件查询实体或组件；迭代接口 | 查询时有效 |

### 能力（提供方保证）

| 序号 | 能力 | 说明 |
|------|------|------|
| 1 | 实体 | Entity::Create、Entity::Destroy、Entity::GetSceneNode、Entity::SetEnabled、Entity::IsEnabled；Entity直接实现ISceneNode接口；生命周期与Scene节点绑定 |
| 2 | 组件 | Component基类；Entity::AddComponent、Entity::GetComponent、Entity::RemoveComponent、Entity::HasComponent；ComponentRegistry::RegisterComponentType；与Object反射联动；Entity模块不提供具体Component实现 |
| 3 | 变换 | Entity通过ISceneNode接口管理变换：GetLocalTransform、SetLocalTransform、GetWorldTransform、GetWorldMatrix；与Scene节点共用 |
| 4 | Entity管理器 | EntityManager::CreateEntity、EntityManager::DestroyEntity、EntityManager::GetEntity、EntityManager::FindEntityByName、EntityManager::GetEntitiesInWorld；EntityManager::QueryEntitiesWithComponent、EntityManager::QueryEntitiesWithComponents |
| 5 | 组件查询 | ComponentQuery::Query（单组件和多组件AND查询）、ComponentQuery::ForEach（迭代查询） |
| 6 | 组件注册 | IComponentRegistry::RegisterComponentType、IComponentRegistry::GetComponentTypeInfo、IComponentRegistry::IsComponentTypeRegistered；GetComponentRegistry获取注册表单例 |
| 7 | 可选ECS | System基类、SystemManager::RegisterSystem、SystemManager::UnregisterSystem、SystemManager::SetExecutionOrder、SystemManager::Update；SystemExecutionOrder枚举（PreUpdate、Update、PostUpdate、Render、PostRender） |

## 架构说明

### Entity与ISceneNode集成

- **Entity直接实现ISceneNode接口**：Entity类继承自`te::scene::ISceneNode`，直接作为场景节点被Scene模块管理
- **变换管理**：Entity的变换直接通过ISceneNode接口管理，与Scene节点共用变换数据，避免重复存储
- **节点注册**：Entity创建时自动调用`SceneManager::RegisterNode`注册到Scene，销毁时自动注销
- **节点类型**：Entity支持Static/Dynamic节点类型，影响Scene的空间索引策略

### Component系统

- **Component基类**：所有Component必须继承自`te::entity::Component`基类
- **生命周期回调**：Component可以重写`OnAttached`和`OnDetached`方法
- **组件存储**：Entity内部使用`std::unordered_map<TypeId, std::unique_ptr<Component>>`存储组件
- **类型注册**：Component类型需要注册到ComponentRegistry和Object模块的TypeRegistry
- **Entity模块不提供具体实现**：Entity模块只提供Component系统基础架构，不提供任何具体的Component实现（如TransformComponent、ModelComponent等），这些应由各自的模块实现

### Component查询

- **单组件查询**：`ComponentQuery::Query<T>`查询所有有指定组件的Entity
- **多组件AND查询**：`ComponentQuery::Query<C1, C2, ...>`查询同时有多个组件的Entity
- **迭代查询**：`ComponentQuery::ForEach<T>`和`ComponentQuery::ForEach<C1, C2, ...>`提供Lambda回调迭代

### ECS系统（可选）

- **System基类**：所有ECS System继承自`System`基类
- **执行顺序**：通过`SystemExecutionOrder`枚举和`GetExecutionOrder`方法控制执行顺序
- **SystemManager**：管理System的注册、执行顺序和更新

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。

## 约束

- 须在 Core、Object、Scene 可用之后使用；组件类型须在 Object 中注册。实体销毁时须释放或转移组件与绑定资源。
- Entity模块不处理资源相关内容，资源相关组件（如ModelComponent）应由World模块实现。
- Entity模块不提供任何具体的Component实现，所有Component应由各自的模块实现。

## TODO 列表

- [x] **Entity实现ISceneNode**：Entity类直接实现ISceneNode接口
- [x] **组件系统基础架构**：Component基类、组件存储、组件查询
- [x] **EntityManager**：Entity生命周期管理和查询
- [x] **ComponentQuery**：组件查询系统
- [x] **ComponentRegistry**：组件类型注册系统
- [x] **ECS系统**：System和SystemManager
- [ ] **数据**：Component属性按002可序列化约定；跨资源引用仅存ResourceId
- [ ] **接口**：CreateEntityFromPrefab(prefabDesc)，prefabDesc由调用方加载得到

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 005-Entity 契约 |
| 2026-02-05 | 统一目录；能力列表用表格；去除冗余衔接说明 |
| 2026-02-06 | 实现ModelComponent的ResourceId支持；实现Entity-Node资源自动同步；增加资源查询接口 |
| 2026-02-06 | 架构重构：Entity直接实现ISceneNode接口；移除ModelComponent和TransformComponent实现；Entity模块不提供具体Component实现；添加Component使用指南文档 |
