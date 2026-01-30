# 004-Scene 模块 ABI

- **契约**：[004-scene-public-api.md](./004-scene-public-api.md)（能力与类型描述）
- **本文件**：004-Scene 对外 ABI 显式表。

## ABI 表

列定义：**模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明**

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 004-Scene | TenEngine::scene | ISceneWorld | 抽象接口 | 场景世界（Entity 容器） | TenEngine/scene/SceneWorld.h | ISceneWorld::CreateEntity, ISceneWorld::DestroyEntity | `IEntity* CreateEntity();` 场景内创建 Entity。`void DestroyEntity(IEntity* entity);` 销毁指定 Entity |
| 004-Scene | TenEngine::scene | ISceneWorld | 抽象接口 | 获取场景内 Entity | TenEngine/scene/SceneWorld.h | ISceneWorld::GetEntities | `IRange<IEntity*> GetEntities() const;` 或等价迭代器/列表；供渲染/脚本等系统遍历 |
| 004-Scene | TenEngine::scene | — | 自由函数/单例 | 获取当前场景世界 | TenEngine/scene/SceneWorld.h | GetSceneWorld | `ISceneWorld* GetSceneWorld();` 或由 Subsystems 注册；调用方不拥有指针 |

*来源：用户故事 US-entity-001（场景通过 ECS 组织，Entity 为场景单元，多种 Component）。*
