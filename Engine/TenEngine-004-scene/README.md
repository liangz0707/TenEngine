# TenEngine-004-Scene 模块

Scene模块提供场景管理、遍历和空间查询算法。这是一个纯算法模块，不依赖Resource和World模块。

## 模块特性

- **纯算法模块**：仅依赖Core模块，不依赖Object、Resource、World等模块
- **接口驱动**：通过ISceneNode接口管理节点，不持有节点所有权
- **空间索引**：支持八叉树（3D）和四叉树（2D）空间索引
- **动态静态分离**：动态节点使用线性列表，静态节点使用空间索引
- **空间查询**：提供视锥剔除、AABB检测、包含检测、相交检测、射线检测等功能

## 核心接口

### ISceneNode
所有场景管理的节点必须实现ISceneNode接口。World和Entity模块可以创建自己的节点类型实现此接口。

### SceneManager
场景管理器单例，管理所有场景世界：
- `CreateWorld()` - 创建场景世界
- `RegisterNode()` / `UnregisterNode()` - 注册/注销节点
- `UpdateTransforms()` - 更新变换
- `Traverse()` - 层级遍历

### SceneWorld
场景世界容器，管理场景图：
- `RegisterNode()` / `UnregisterNode()` - 注册/注销节点
- `UpdateTransforms()` - 更新变换
- `Traverse()` - 层级遍历
- `FindNodeByName()` / `FindNodeById()` - 查找节点

### SpatialQuery
空间查询算法：
- `QueryFrustum()` - 视锥剔除
- `QueryAABB()` - AABB查询
- `QueryContained()` - 包含检测
- `QueryIntersecting()` - 相交检测
- `Raycast()` - 射线检测
- `FindNearest()` - 最近点查询

## 使用示例

### 创建场景世界

```cpp
#include <te/scene/SceneManager.h>
#include <te/core/math.h>

te::scene::SceneManager& manager = te::scene::SceneManager::GetInstance();

// 创建3D场景（使用八叉树）
te::core::AABB bounds;
bounds.min = {0, 0, 0};
bounds.max = {1000, 1000, 1000};
te::scene::WorldRef world = manager.CreateWorld(
    te::scene::SpatialIndexType::Octree, 
    bounds
);
```

### 注册节点

```cpp
// 假设Entity实现了ISceneNode接口
te::entity::Entity* entity = CreateEntity();

// 注册到Scene
manager.RegisterNode(entity);
```

### 空间查询

```cpp
#include <te/scene/SpatialQuery.h>

// 视锥剔除
te::scene::Frustum frustum;
// ... 设置frustum ...

std::vector<te::scene::ISceneNode*> visibleNodes;
te::scene::SpatialQuery::QueryFrustum(world, frustum, 
    [&](te::scene::ISceneNode* node) {
        visibleNodes.push_back(node);
    }
);

// AABB查询
te::core::AABB queryAABB;
queryAABB.min = {10, 10, 10};
queryAABB.max = {50, 50, 50};

te::scene::SpatialQuery::QueryAABB(world, queryAABB,
    [](te::scene::ISceneNode* node) {
        // 处理查询结果
    }
);
```

## 文件结构

```
Engine/TenEngine-004-scene/
├── include/te/scene/
│   ├── SceneTypes.h          # 基础类型
│   ├── ISceneNode.h          # 场景节点接口
│   ├── SceneManager.h        # 场景管理器
│   ├── SceneWorld.h          # 场景世界
│   ├── SpatialQuery.h        # 空间查询
│   ├── NodeManager.h         # 节点管理器接口
│   ├── DynamicNodeManager.h  # 动态节点管理器
│   ├── StaticNodeManager.h   # 静态节点管理器
│   ├── SpatialIndex.h        # 空间索引接口
│   ├── Octree.h              # 八叉树
│   └── Quadtree.h            # 四叉树
├── src/
│   ├── SceneManager.cpp
│   ├── SceneWorld.cpp
│   ├── SpatialQuery.cpp
│   ├── DynamicNodeManager.cpp
│   ├── StaticNodeManager.cpp
│   ├── Octree.cpp
│   └── Quadtree.cpp
└── tests/
    └── unit/
        ├── test_scene_manager.cpp
        ├── test_scene_world.cpp
        ├── test_spatial_query.cpp
        ├── test_node_managers.cpp
        ├── test_octree.cpp
        └── test_quadtree.cpp
```

## 依赖

- **001-Core**：数学类型、容器、内存、日志

## 契约文档

- API契约：`specs/_contracts/004-scene-public-api.md`
- ABI契约：`specs/_contracts/004-scene-ABI.md`
- 模块规格：`docs/module-specs/004-scene.md`
