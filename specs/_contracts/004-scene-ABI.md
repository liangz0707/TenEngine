# 004-Scene 模块 ABI

- **契约**：[004-scene-public-api.md](./004-scene-public-api.md)（能力与类型描述）
- **本文件**：004-Scene 对外 ABI 显式表。
- **命名空间约定**：本分支统一使用 **`te::`** 作为全局命名空间；004-Scene 对外符号位于 **`te::scene`**，头文件路径为 **`te/scene/`**（与 `te::rhi`、`te::rendercore`、`te::pipelinecore` 等一致）。
- **参考**：Unity SceneManager（LoadScene/UnloadScene/GetActiveScene）、UE UWorld/Level 流式加载；场景图与层级对应 Unity Transform 层次、UE Actor 层级。

## ABI 表

列定义：**模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明**

### 类型与枚举

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 004-Scene | te::scene | SceneRef | struct/句柄 | 场景引用（与WorldRef别名） | te/scene/SceneTypes.h | SceneRef | 值类型或句柄；标识已加载场景，用于SetActiveWorld/DestroyWorld |
| 004-Scene | te::scene | WorldRef | struct/句柄 | 世界容器引用 | te/scene/SceneTypes.h | WorldRef | 值类型或句柄；标识场景世界，SceneRef是WorldRef的别名 |
| 004-Scene | te::scene | NodeId | struct/句柄 | 场景图节点 ID | te/scene/SceneTypes.h | NodeId | 值类型；标识场景图节点，层级路径与查找；包含value(void*)字段、IsValid()方法、operator==和operator!= |
| 004-Scene | te::scene | NodeType | 枚举 | 节点类型 | te/scene/SceneTypes.h | NodeType::Static, NodeType::Dynamic | `enum class NodeType { Static, Dynamic };` Static=静态节点（空间索引），Dynamic=动态节点（线性列表） |
| 004-Scene | te::scene | SpatialIndexType | 枚举 | 空间索引类型 | te/scene/SceneTypes.h | SpatialIndexType::None, Octree, Quadtree | `enum class SpatialIndexType { None, Octree, Quadtree };` 创建World时指定 |
| 004-Scene | te::scene | Transform | struct | 变换（位置、旋转、缩放） | te/scene/SceneTypes.h | Transform | position(Vector3), rotation(Quaternion), scale(Vector3) |
| 004-Scene | te::scene | Frustum | struct | 视锥体 | te/scene/SceneTypes.h | Frustum | planes[6][4]，用于视锥剔除 |

### 场景管理器（SceneManager 单例）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 004-Scene | te::scene | SceneManager | 类/单例 | 获取单例实例 | te/scene/SceneManager.h | SceneManager::GetInstance | `static SceneManager& GetInstance();` 返回单例引用 |
| 004-Scene | te::scene | SceneManager | 类/单例 | 创建场景世界 | te/scene/SceneManager.h | SceneManager::CreateWorld | `WorldRef CreateWorld(SpatialIndexType indexType, te::core::AABB const& bounds);` 创建新场景世界 |
| 004-Scene | te::scene | SceneManager | 类/单例 | 销毁场景世界 | te/scene/SceneManager.h | SceneManager::DestroyWorld | `void DestroyWorld(WorldRef world);` 销毁场景世界并清理所有节点 |
| 004-Scene | te::scene | SceneManager | 类/单例 | 获取活动世界 | te/scene/SceneManager.h | SceneManager::GetActiveWorld | `WorldRef GetActiveWorld() const;` 返回当前活动世界引用 |
| 004-Scene | te::scene | SceneManager | 类/单例 | 设置活动世界 | te/scene/SceneManager.h | SceneManager::SetActiveWorld | `void SetActiveWorld(WorldRef world);` 设置当前活动世界 |
| 004-Scene | te::scene | SceneManager | 类/单例 | 注册节点 | te/scene/SceneManager.h | SceneManager::RegisterNode | `void RegisterNode(ISceneNode* node);` 注册节点到Scene管理（不持有所有权） |
| 004-Scene | te::scene | SceneManager | 类/单例 | 注销节点 | te/scene/SceneManager.h | SceneManager::UnregisterNode | `void UnregisterNode(ISceneNode* node);` 从Scene管理注销节点 |
| 004-Scene | te::scene | SceneManager | 类/单例 | 更新变换 | te/scene/SceneManager.h | SceneManager::UpdateTransforms | `void UpdateTransforms(WorldRef world);` 更新指定世界的所有脏节点变换 |
| 004-Scene | te::scene | SceneManager | 类/单例 | 移动节点 | te/scene/SceneManager.h | SceneManager::MoveNode | `void MoveNode(ISceneNode* node, te::core::Vector3 const& position);` 移动节点位置 |
| 004-Scene | te::scene | SceneManager | 类/单例 | 节点类型转换 | te/scene/SceneManager.h | SceneManager::ConvertToStatic/ConvertToDynamic | `bool ConvertToStatic(ISceneNode* node); bool ConvertToDynamic(ISceneNode* node);` 转换节点类型 |
| 004-Scene | te::scene | SceneManager | 类/单例 | 层级遍历 | te/scene/SceneManager.h | SceneManager::Traverse | `void Traverse(WorldRef world, std::function<void(ISceneNode*)> const& callback);` 遍历场景图 |
| 004-Scene | te::scene | SceneManager | 类/单例 | 按名称查找 | te/scene/SceneManager.h | SceneManager::FindNodeByName | `ISceneNode* FindNodeByName(WorldRef world, char const* name);` 按名称查找节点 |
| 004-Scene | te::scene | SceneManager | 类/单例 | 按ID查找 | te/scene/SceneManager.h | SceneManager::FindNodeById | `ISceneNode* FindNodeById(WorldRef world, NodeId id);` 按ID查找节点 |
| 004-Scene | te::scene | SceneManager | 类/单例 | 获取世界 | te/scene/SceneManager.h | SceneManager::GetWorld | `SceneWorld* GetWorld(WorldRef world) const;` 根据WorldRef获取SceneWorld指针，返回nullptr如果无效 |

### 场景世界（SceneWorld）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 004-Scene | te::scene | SceneWorld | 类 | 获取世界引用 | te/scene/SceneWorld.h | SceneWorld::GetWorldRef | `WorldRef GetWorldRef() const;` 返回世界引用 |
| 004-Scene | te::scene | SceneWorld | 类 | 注册节点 | te/scene/SceneWorld.h | SceneWorld::RegisterNode | `void RegisterNode(ISceneNode* node);` 注册节点到世界（不持有所有权） |
| 004-Scene | te::scene | SceneWorld | 类 | 注销节点 | te/scene/SceneWorld.h | SceneWorld::UnregisterNode | `void UnregisterNode(ISceneNode* node);` 从世界注销节点 |
| 004-Scene | te::scene | SceneWorld | 类 | 更新变换 | te/scene/SceneWorld.h | SceneWorld::UpdateTransforms | `void UpdateTransforms();` 更新所有脏节点的世界变换 |
| 004-Scene | te::scene | SceneWorld | 类 | 获取根节点 | te/scene/SceneWorld.h | SceneWorld::GetRootNodes | `void GetRootNodes(std::vector<ISceneNode*>& out) const;` 获取所有根节点 |
| 004-Scene | te::scene | SceneWorld | 类 | 层级遍历 | te/scene/SceneWorld.h | SceneWorld::Traverse | `void Traverse(std::function<void(ISceneNode*)> const& callback) const;` 遍历场景图 |
| 004-Scene | te::scene | SceneWorld | 类 | 按名称查找 | te/scene/SceneWorld.h | SceneWorld::FindNodeByName | `ISceneNode* FindNodeByName(char const* name) const;` 按名称查找节点 |
| 004-Scene | te::scene | SceneWorld | 类 | 按ID查找 | te/scene/SceneWorld.h | SceneWorld::FindNodeById | `ISceneNode* FindNodeById(NodeId id) const;` 按ID查找节点 |
| 004-Scene | te::scene | SceneWorld | 类 | 获取空间索引类型 | te/scene/SceneWorld.h | SceneWorld::GetSpatialIndexType | `SpatialIndexType GetSpatialIndexType() const;` 返回世界使用的空间索引类型（None/Octree/Quadtree） |

### 场景图与节点（ISceneNode，对齐 Unity Transform / UE 层级）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 004-Scene | te::scene | ISceneNode | 抽象接口 | 获取父节点 | te/scene/ISceneNode.h | ISceneNode::GetParent | `ISceneNode* GetParent() const;` 根节点返回 nullptr |
| 004-Scene | te::scene | ISceneNode | 抽象接口 | 设置父节点 | te/scene/ISceneNode.h | ISceneNode::SetParent | `void SetParent(ISceneNode* parent);` 父子关系变更后标记脏 |
| 004-Scene | te::scene | ISceneNode | 抽象接口 | 获取子节点列表 | te/scene/ISceneNode.h | ISceneNode::GetChildren | `void GetChildren(std::vector<ISceneNode*>& out) const;` 输出子节点列表 |
| 004-Scene | te::scene | ISceneNode | 抽象接口 | 获取子节点数量 | te/scene/ISceneNode.h | ISceneNode::GetChildCount | `size_t GetChildCount() const;` 返回子节点数量 |
| 004-Scene | te::scene | ISceneNode | 抽象接口 | 获取局部变换 | te/scene/ISceneNode.h | ISceneNode::GetLocalTransform | `Transform const& GetLocalTransform() const;` 返回局部变换（position, rotation, scale） |
| 004-Scene | te::scene | ISceneNode | 抽象接口 | 设置局部变换 | te/scene/ISceneNode.h | ISceneNode::SetLocalTransform | `void SetLocalTransform(Transform const& t);` 设置局部变换后标记脏 |
| 004-Scene | te::scene | ISceneNode | 抽象接口 | 获取世界变换 | te/scene/ISceneNode.h | ISceneNode::GetWorldTransform | `Transform const& GetWorldTransform() const;` 依赖 UpdateTransforms 已执行；只读 |
| 004-Scene | te::scene | ISceneNode | 抽象接口 | 获取世界矩阵 | te/scene/ISceneNode.h | ISceneNode::GetWorldMatrix | `te::core::Matrix4 const& GetWorldMatrix() const;` 4×4 变换矩阵 |
| 004-Scene | te::scene | ISceneNode | 抽象接口 | 获取节点 ID | te/scene/ISceneNode.h | ISceneNode::GetNodeId | `NodeId GetNodeId() const;` 返回节点ID |
| 004-Scene | te::scene | ISceneNode | 抽象接口 | 获取节点名称 | te/scene/ISceneNode.h | ISceneNode::GetName | `char const* GetName() const;` 返回节点名称（null-terminated） |
| 004-Scene | te::scene | ISceneNode | 抽象接口 | 设置激活状态 | te/scene/ISceneNode.h | ISceneNode::SetActive | `void SetActive(bool active);` 节点及子树是否参与更新/渲染 |
| 004-Scene | te::scene | ISceneNode | 抽象接口 | 查询是否激活 | te/scene/ISceneNode.h | ISceneNode::IsActive | `bool IsActive() const;` 考虑父链：父未激活则本节点视为未激活 |
| 004-Scene | te::scene | ISceneNode | 抽象接口 | 获取节点类型 | te/scene/ISceneNode.h | ISceneNode::GetNodeType | `NodeType GetNodeType() const;` 返回节点类型（Static/Dynamic） |
| 004-Scene | te::scene | ISceneNode | 抽象接口 | 检查是否有AABB | te/scene/ISceneNode.h | ISceneNode::HasAABB | `bool HasAABB() const;` 默认返回false，可选实现 |
| 004-Scene | te::scene | ISceneNode | 抽象接口 | 获取AABB | te/scene/ISceneNode.h | ISceneNode::GetAABB | `te::core::AABB GetAABB() const;` 返回世界空间AABB，默认返回空AABB |
| 004-Scene | te::scene | ISceneNode | 抽象接口 | 检查是否脏 | te/scene/ISceneNode.h | ISceneNode::IsDirty | `bool IsDirty() const;` 检查是否需要变换更新 |
| 004-Scene | te::scene | ISceneNode | 抽象接口 | 设置脏标记 | te/scene/ISceneNode.h | ISceneNode::SetDirty | `void SetDirty(bool dirty);` 设置脏标记 |

### 空间查询（SpatialQuery）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 004-Scene | te::scene | SpatialQuery | 静态类 | 视锥剔除查询 | te/scene/SpatialQuery.h | SpatialQuery::QueryFrustum | `static void QueryFrustum(WorldRef world, Frustum const& frustum, std::function<void(ISceneNode*)> const& callback);` 查询视锥内的节点 |
| 004-Scene | te::scene | SpatialQuery | 静态类 | AABB查询 | te/scene/SpatialQuery.h | SpatialQuery::QueryAABB | `static void QueryAABB(WorldRef world, te::core::AABB const& aabb, std::function<void(ISceneNode*)> const& callback);` 查询与AABB相交的节点 |
| 004-Scene | te::scene | SpatialQuery | 静态类 | AABB包含查询 | te/scene/SpatialQuery.h | SpatialQuery::QueryContained | `static void QueryContained(WorldRef world, te::core::AABB const& aabb, std::function<void(ISceneNode*)> const& callback);` 查询完全包含在AABB内的节点 |
| 004-Scene | te::scene | SpatialQuery | 静态类 | AABB相交查询 | te/scene/SpatialQuery.h | SpatialQuery::QueryIntersecting | `static void QueryIntersecting(WorldRef world, te::core::AABB const& aabb, std::function<void(ISceneNode*)> const& callback);` 查询与AABB相交的节点（QueryAABB的别名） |
| 004-Scene | te::scene | SpatialQuery | 静态类 | 射线检测 | te/scene/SpatialQuery.h | SpatialQuery::Raycast | `static bool Raycast(WorldRef world, te::core::Ray const& ray, ISceneNode*& outHitNode, float& outDistance);` 射线检测，返回最近的相交节点 |
| 004-Scene | te::scene | SpatialQuery | 静态类 | 最近点查询 | te/scene/SpatialQuery.h | SpatialQuery::FindNearest | `static ISceneNode* FindNearest(WorldRef world, te::core::Vector3 const& point, float maxDistance = 3.402823466e+38f);` 查找距离点最近的节点（maxDistance默认值为FLT_MAX） |
| 004-Scene | te::scene | SpatialQuery | 静态类 | 视锥-AABB相交测试 | te/scene/SpatialQuery.h | SpatialQuery::FrustumIntersectsAABB | `static bool FrustumIntersectsAABB(Frustum const& frustum, te::core::AABB const& aabb);` 测试AABB是否与视锥相交（辅助函数，public供空间索引使用） |
| 004-Scene | te::scene | SpatialQuery | 静态类 | AABB包含测试 | te/scene/SpatialQuery.h | SpatialQuery::AABBContains | `static bool AABBContains(te::core::AABB const& inner, te::core::AABB const& outer);` 测试inner AABB是否完全包含在outer AABB内（辅助函数，public） |
| 004-Scene | te::scene | SpatialQuery | 静态类 | AABB相交测试 | te/scene/SpatialQuery.h | SpatialQuery::AABBIntersects | `static bool AABBIntersects(te::core::AABB const& a, te::core::AABB const& b);` 测试两个AABB是否相交（辅助函数，public供空间索引使用） |

### 节点管理器（内部实现，SceneWorld使用）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 004-Scene | te::scene | INodeManager | 抽象接口 | 添加节点 | te/scene/NodeManager.h | INodeManager::AddNode | `virtual void AddNode(ISceneNode* node) = 0;` 添加节点到管理器 |
| 004-Scene | te::scene | INodeManager | 抽象接口 | 移除节点 | te/scene/NodeManager.h | INodeManager::RemoveNode | `virtual void RemoveNode(ISceneNode* node) = 0;` 从管理器移除节点 |
| 004-Scene | te::scene | INodeManager | 抽象接口 | 更新节点 | te/scene/NodeManager.h | INodeManager::UpdateNode | `virtual void UpdateNode(ISceneNode* node) = 0;` 更新节点（用于空间索引重建） |
| 004-Scene | te::scene | INodeManager | 抽象接口 | 清空所有节点 | te/scene/NodeManager.h | INodeManager::Clear | `virtual void Clear() = 0;` 清空所有节点 |
| 004-Scene | te::scene | INodeManager | 抽象接口 | 获取节点数量 | te/scene/NodeManager.h | INodeManager::GetNodeCount | `virtual size_t GetNodeCount() const = 0;` 返回节点数量 |
| 004-Scene | te::scene | INodeManager | 抽象接口 | 遍历节点 | te/scene/NodeManager.h | INodeManager::Traverse | `virtual void Traverse(std::function<void(ISceneNode*)> const& callback) const = 0;` 遍历所有节点 |
| 004-Scene | te::scene | DynamicNodeManager | 类 | 动态节点管理器 | te/scene/DynamicNodeManager.h | DynamicNodeManager | 实现INodeManager，使用线性列表管理动态节点（O(1)添加/删除） |
| 004-Scene | te::scene | StaticNodeManager | 类 | 静态节点管理器 | te/scene/StaticNodeManager.h | StaticNodeManager | 实现INodeManager，使用空间索引管理静态节点；提供RebuildIndex、QueryFrustum、QueryAABB方法 |
| 004-Scene | te::scene | StaticNodeManager | 类 | 重建空间索引 | te/scene/StaticNodeManager.h | StaticNodeManager::RebuildIndex | `void RebuildIndex();` 重建空间索引（批量更新脏节点） |
| 004-Scene | te::scene | StaticNodeManager | 类 | 视锥查询 | te/scene/StaticNodeManager.h | StaticNodeManager::QueryFrustum | `void QueryFrustum(Frustum const& frustum, std::function<void(ISceneNode*)> const& callback) const;` 使用空间索引查询视锥内节点 |
| 004-Scene | te::scene | StaticNodeManager | 类 | AABB查询 | te/scene/StaticNodeManager.h | StaticNodeManager::QueryAABB | `void QueryAABB(te::core::AABB const& aabb, std::function<void(ISceneNode*)> const& callback) const;` 使用空间索引查询AABB相交节点 |

### 空间索引（内部实现，StaticNodeManager使用）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 004-Scene | te::scene | ISpatialIndex | 抽象接口 | 插入节点 | te/scene/SpatialIndex.h | ISpatialIndex::Insert | `virtual void Insert(ISceneNode* node) = 0;` 插入节点到空间索引 |
| 004-Scene | te::scene | ISpatialIndex | 抽象接口 | 移除节点 | te/scene/SpatialIndex.h | ISpatialIndex::Remove | `virtual void Remove(ISceneNode* node) = 0;` 从空间索引移除节点 |
| 004-Scene | te::scene | ISpatialIndex | 抽象接口 | 更新节点 | te/scene/SpatialIndex.h | ISpatialIndex::Update | `virtual void Update(ISceneNode* node) = 0;` 更新节点（重新插入） |
| 004-Scene | te::scene | ISpatialIndex | 抽象接口 | 清空索引 | te/scene/SpatialIndex.h | ISpatialIndex::Clear | `virtual void Clear() = 0;` 清空所有节点 |
| 004-Scene | te::scene | ISpatialIndex | 抽象接口 | 视锥查询 | te/scene/SpatialIndex.h | ISpatialIndex::QueryFrustum | `virtual void QueryFrustum(Frustum const& frustum, std::function<void(ISceneNode*)> const& callback) const = 0;` 查询视锥内节点 |
| 004-Scene | te::scene | ISpatialIndex | 抽象接口 | AABB查询 | te/scene/SpatialIndex.h | ISpatialIndex::QueryAABB | `virtual void QueryAABB(te::core::AABB const& aabb, std::function<void(ISceneNode*)> const& callback) const = 0;` 查询AABB相交节点 |
| 004-Scene | te::scene | ISpatialIndex | 抽象接口 | 获取节点数量 | te/scene/SpatialIndex.h | ISpatialIndex::GetNodeCount | `virtual size_t GetNodeCount() const = 0;` 返回索引中的节点数量 |
| 004-Scene | te::scene | Octree | 类 | 八叉树空间索引 | te/scene/Octree.h | Octree | 实现ISpatialIndex，3D八叉树空间索引；构造函数接受bounds、maxDepth、maxNodesPerLeaf参数 |
| 004-Scene | te::scene | Quadtree | 类 | 四叉树空间索引 | te/scene/Quadtree.h | Quadtree | 实现ISpatialIndex，2D四叉树空间索引；构造函数接受bounds、maxDepth、maxNodesPerLeaf参数 |

*来源：用户故事 US-scene-001（场景加载与切换）、US-scene-002（场景图与节点）；参考 Unity SceneManager、Transform 层级；UE UWorld/Level 流式与 Actor 层级。*

---

## 实现说明

- **纯算法模块**：Scene模块不依赖Resource和Object模块，仅依赖Core模块
- **节点所有权**：Scene模块不持有节点对象所有权，通过RegisterNode/UnregisterNode管理节点引用
- **空间查询**：空间查询基于节点的世界AABB，不考虑父子关系；每个节点独立检测
- **层级遍历**：层级遍历按照父子关系，不考虑空间位置；与空间查询遍历独立
- **变换更新**：使用迭代式算法更新变换，避免递归栈溢出；脏标记机制优化性能

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| 2026-02-06 | 重新设计：Scene模块重新设计为纯算法模块，移除Object依赖；实现ISceneNode接口、SceneManager单例、SceneWorld容器、SpatialQuery空间查询；支持八叉树/四叉树空间索引类型（待实现优化） |
| 2026-02-06 | 实现节点管理器和空间索引：实现DynamicNodeManager（线性列表）和StaticNodeManager（空间索引）；实现Octree（3D八叉树）和Quadtree（2D四叉树）空间索引；SceneWorld集成节点管理器；SpatialQuery辅助函数标记为public供空间索引使用 |
| 2026-02-06 | 文档更新：添加SceneWorld::GetSpatialIndexType接口到ABI；更新FindNearest默认参数值；完善实现说明和约束描述；更新TODO列表标记已完成任务 |
| 2026-02-06 | 完成TODO实现：实现Transform到Matrix4转换、矩阵乘法、变换组合；实现节点类型转换（ConvertToStatic/ConvertToDynamic）；修复测试文件链接错误，统一测试运行器 |
