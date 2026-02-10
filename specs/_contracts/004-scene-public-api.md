# 契约：004-Scene 模块对外 API

## 适用模块

- **实现方**：004-Scene（L1；纯算法模块：场景管理、遍历、空间查询算法；不持有资源、不依赖 Resource 和 Object。Level/关卡句柄由 029-World 负责，上层通过 029 调用本模块）
- **对应规格**：`docs/module-specs/004-scene.md`
- **依赖**：001-Core（仅依赖Core模块，不依赖Object模块）

## 消费者

- 029-World（内部调 004 CreateSceneFromDesc/UnloadScene）、005-Entity、014-Physics、020-Pipeline、024-Editor（经 029 获取 SceneRef 后调本模块遍历 API 或 029 委托 API）

## 能力列表

### 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| SceneRef / WorldRef | 场景/世界容器引用（不透明句柄，void*值）；包含IsValid()方法、operator==和operator!= | 创建后直至销毁 |
| NodeId | 场景图节点 ID（不透明句柄，void*指向ISceneNode*）；包含IsValid()方法、operator==和operator!=；父子关系、层级路径；与 Entity 一一对应或映射 | 与节点同生命周期 |
| ISceneNode | 场景节点接口：所有Scene管理的节点必须实现此接口；World/Entity可创建自己的节点类型实现ISceneNode | 与节点同生命周期 |
| Transform | 局部/世界变换（position, rotation, scale，使用Core.Math类型） | 与节点/实体同步 |
| SceneWorld | 场景世界类：管理场景图、节点注册、变换更新、层级遍历 | 与 WorldRef 绑定 |
| SceneManager | 场景管理器单例：管理所有场景世界、节点注册、提供全局API | 进程生命周期 |
| NodeType | 节点类型枚举（Static/Dynamic）：Static使用空间索引，Dynamic使用线性列表 | 与节点同生命周期 |
| SpatialIndexType | 空间索引类型枚举（None/Octree/Quadtree）：创建World时指定 | 与World绑定 |
| AABB | 轴对齐包围盒（使用Core.Math::AABB）：用于空间查询和剔除 | 查询时使用 |
| Frustum | 视锥体（6个平面方程）：用于视锥剔除查询 | 查询时使用 |
| Ray | 射线（使用Core.Math::Ray）：用于射线检测 | 查询时使用 |
| INodeManager | 节点管理器接口（内部实现）：DynamicNodeManager和StaticNodeManager的基类 | SceneWorld内部使用 |
| DynamicNodeManager | 动态节点管理器（内部实现）：使用线性列表管理动态节点 | SceneWorld内部使用 |
| StaticNodeManager | 静态节点管理器（内部实现）：使用空间索引管理静态节点 | SceneWorld内部使用 |
| ISpatialIndex | 空间索引接口（内部实现）：Octree和Quadtree的基类 | StaticNodeManager内部使用 |
| Octree | 八叉树空间索引（内部实现）：3D空间索引实现 | StaticNodeManager内部使用 |
| Quadtree | 四叉树空间索引（内部实现）：2D空间索引实现 | StaticNodeManager内部使用 |
| SceneDesc | 场景描述：roots（根节点描述列表）；仅 Core 类型，004 定义 | CreateSceneFromDesc 入参 |
| SceneNodeDesc | 节点描述：name、localTransform、children、opaqueUserData；004 不解析 opaqueUserData | CreateSceneFromDesc 树形结构 |
| NodeFactoryFn | 节点工厂回调：ISceneNode*(SceneNodeDesc const&)；由 029 提供 | CreateSceneFromDesc 时创建节点 |

### 能力（提供方保证）

| 序号 | 能力 | 说明 |
|------|------|------|
| 1 | 场景图 | 节点树、父子关系、局部/世界变换、脏标记与变换更新；SceneWorld::UpdateTransforms、SceneManager::UpdateTransforms |
| 2 | 层级遍历 | SceneWorld::Traverse、SceneManager::Traverse：层级遍历；FindByName、FindById：按名称/ID查找节点；GetRootNodes：获取根节点；GetSpatialIndexType：获取空间索引类型 |
| 3 | World/Scene 容器 | SceneManager::CreateWorld、DestroyWorld：创建/销毁场景世界；GetActiveWorld、SetActiveWorld：获取/设置活动世界 |
| 4 | 节点注册 | SceneManager::RegisterNode(node)、RegisterNode(node, world)：注册/注销节点；根节点使用 RegisterNode(node, world)；Scene模块不拥有节点所有权，World/Entity负责节点生命周期 |
| 5 | 激活/禁用 | ISceneNode::SetActive、IsActive：节点与子树参与更新/渲染的开关；考虑父链激活状态 |
| 6 | 节点类型管理 | NodeType枚举（Static/Dynamic）；ISceneNode::GetNodeType：获取节点类型；ConvertToStatic/ConvertToDynamic：节点类型转换 |
| 7 | 节点操作 | SceneManager::MoveNode：移动节点位置；节点变换通过ISceneNode接口管理 |
| 8 | 空间查询 | SpatialQuery::QueryFrustum：视锥剔除；QueryAABB、QueryIntersecting：AABB相交查询；QueryContained：AABB包含查询；Raycast：射线检测；FindNearest：最近点查询；辅助函数FrustumIntersectsAABB、AABBContains、AABBIntersects（public供空间索引使用） |
| 9 | 空间索引 | SpatialIndexType枚举（None/Octree/Quadtree）：创建World时指定空间索引类型；静态节点使用空间索引优化查询 |
| 10 | 节点管理器 | SceneWorld内部使用DynamicNodeManager（线性列表）和StaticNodeManager（空间索引）管理节点；动态节点O(1)添加/删除，静态节点使用空间索引O(log n)查询；StaticNodeManager提供RebuildIndex批量更新 |
| 11 | 空间索引实现 | Octree（3D八叉树）和Quadtree（2D四叉树）实现ISpatialIndex接口；支持插入、删除、更新、查询操作；自动分割和合并节点优化性能 |
| 12 | 从描述创建/卸载场景 | SceneManager::CreateSceneFromDesc(indexType, bounds, SceneDesc, NodeFactoryFn)：创建 World 并按描述树注册节点，节点由 factory 创建、004 不持有；UnloadScene(WorldRef)：卸载场景（等价 DestroyWorld），节点对象由 029 销毁 |

Scene模块是纯算法模块，不依赖Resource和Object模块。命名空间 `te::scene`；头文件 SceneTypes.h、SceneDesc.h、ISceneNode.h、SceneWorld.h、SceneManager.h、SpatialQuery.h。

## 节点管理架构

Scene模块通过ISceneNode接口管理节点，不持有节点对象的所有权：

- **ISceneNode接口**：所有Scene管理的节点必须实现ISceneNode接口，包含层级关系、变换、激活状态、名称、节点类型、AABB（可选）等方法
- **节点所有权**：Scene模块不拥有节点对象的所有权；World/Entity模块负责创建和销毁节点对象
- **节点注册**：使用SceneManager::RegisterNode()注册外部节点，UnregisterNode()注销节点
- **自定义节点**：World和Entity可以创建自己的节点类型，实现ISceneNode接口，然后通过RegisterNode()注册到Scene管理
- **AABB可选**：节点可以选择性实现HasAABB()和GetAABB()方法，只有实现的节点才能参与空间查询
- **节点管理器**：SceneWorld内部使用DynamicNodeManager和StaticNodeManager分别管理动态和静态节点
  - **DynamicNodeManager**：使用线性列表（std::vector + std::unordered_set）管理动态节点，提供O(1)添加/删除操作，适合频繁移动的对象（角色、载具等）
  - **StaticNodeManager**：使用空间索引（Octree或Quadtree）管理静态节点，提供O(log n)查询操作，适合静态对象（地形、建筑等）；支持RebuildIndex批量更新脏节点
- **空间索引**：StaticNodeManager内部使用ISpatialIndex接口的空间索引实现
  - **Octree**：3D八叉树空间索引，每个节点将空间分为8个八分体，适合3D场景
  - **Quadtree**：2D四叉树空间索引，每个节点将空间分为4个象限，适合2D场景或固定高度的3D场景
  - 空间索引支持自动分割和合并，根据节点密度优化查询性能

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。

## 约束

- 须在 Core 可用后使用。Scene模块不依赖Object模块。Level 加载由上层经 029 进行；029 创建节点并注册到Scene模块。变换更新与脏标记须一致；多 World 时当前活动场景语义由 004 提供、029 可封装。

## TODO 列表

（以下任务来自 `docs/asset/` 资源管理/加载/存储设计。）

- [x] **轻量资源引用**：004 节点支持持有ResourceId（不持有IResource*），仍不依赖013-Resource模块；SceneDesc包含ResourceId字段。
- [x] **ResourceId查询接口**：GetNodeModelResourceId、GetNodeEntityPrefabResourceId、HasNodeModelResourceId、HasNodeEntityPrefabResourceId已实现。
- [x] **接口**：CreateSceneFromDesc(SceneDesc, NodeFactoryFn) 由 029 调用；SceneDesc 仅 Core 类型、opaqueUserData 供 029 绑定；UnloadScene(world) 等价 DestroyWorld，与 029 协同（029 先销毁 Entity 再调 UnloadScene）。
- [x] **节点管理器实现**：实现DynamicNodeManager和StaticNodeManager，完成动态静态节点分离管理
- [x] **空间索引实现**：实现Octree和Quadtree空间索引，完成空间查询优化
- [x] **SceneWorld集成**：SceneWorld集成节点管理器，根据节点类型自动分配到对应管理器
- [x] **CMake构建配置**：按照上游模块方式配置CMake构建系统，使用cmake辅助文件管理依赖

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 004-Scene 契约；仅场景管理算法；上层经 029 调用 |
| 2026-02-05 | CreateSceneFromDesc 入参改为 004 的 SceneDesc，避免依赖 029/013 |
| 2026-02-05 | 统一目录；能力列表用表格；不引用 ABI 文件 |
| 2026-02-06 | 架构重构：引入SceneManager统一管理，封装节点实现，实现迭代式变换更新，完善变换数学，实现节点对象池 |
| 2026-02-06 | 性能优化与动态静态分离：添加NodeType枚举（Static/Dynamic），实现DynamicNodeManager和StaticNodeManager，支持快速节点操作（CreateNodeFast、DestroyNodeFast、CreateNodesBatch、DestroyNodesBatch、MoveNode、ConvertToDynamic/ConvertToStatic），优化空间查询接口（QueryFrustum、QueryAABB及其静态/动态分离版本） |
| 2026-02-06 | 架构调整：移除ResourceId支持，Scene模块仅提供算法不持有数据；资源关联由World模块管理 |
| 2026-02-06 | 架构重构：引入ISceneNode接口架构，Scene模块通过ISceneNode接口管理节点，不持有节点所有权；World/Entity可创建自定义节点类型实现ISceneNode接口 |
| 2026-02-06 | 重新设计：Scene模块重新设计为纯算法模块，移除Object依赖；实现SceneManager、SceneWorld、SpatialQuery；支持八叉树/四叉树空间索引（待实现）；提供完整的空间查询算法 |
| 2026-02-06 | 实现节点管理器和空间索引：实现DynamicNodeManager（线性列表，O(1)操作）和StaticNodeManager（空间索引，O(log n)查询）；实现Octree（3D八叉树）和Quadtree（2D四叉树）空间索引；SceneWorld集成节点管理器，根据节点类型自动分配；SpatialQuery辅助函数标记为public供空间索引使用；完善CMake构建配置 |
| 2026-02-06 | 文档一致性检查与更新：添加SceneWorld::GetSpatialIndexType到ABI和API文档；更新FindNearest默认参数值描述；完善实现说明、约束描述和类型说明；更新TODO列表标记已完成任务；明确内部实现头文件的可见性 |
| 2026-02-06 | 完成TODO实现：实现Transform到Matrix4转换、矩阵乘法、变换组合算法；实现节点类型转换功能（ConvertToStatic/ConvertToDynamic）；修复测试文件链接错误，创建统一测试运行器；完善变换更新算法 |
