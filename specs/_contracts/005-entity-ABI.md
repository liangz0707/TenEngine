# 005-Entity 模块 ABI

- **契约**：[005-entity-public-api.md](./005-entity-public-api.md)（能力与类型描述）
- **本文件**：005-Entity 对外 ABI 显式表。

## ABI 表

列定义：**模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明**

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 005-Entity | TenEngine::entity | IEntity | 抽象接口 | 实体（场景单元） | TenEngine/entity/Entity.h | IEntity::AddComponent, GetComponent, RemoveComponent, HasComponent | `template<typename T> T* AddComponent();` `template<typename T> T* GetComponent();` `template<typename T> void RemoveComponent();` `template<typename T> bool HasComponent() const;` |
| 005-Entity | TenEngine::entity | — | struct | 变换组件 | TenEngine/entity/TransformComponent.h | TransformComponent | 位置、旋转、缩放；层级父子由本模块或 Scene 管理 |
| 005-Entity | TenEngine::entity | — | struct | 模型组件 | TenEngine/entity/ModelComponent.h | ModelComponent | 网格/模型引用，供渲染收集 |
| 005-Entity | TenEngine::entity | — | struct | 脚本组件 | TenEngine/entity/ScriptComponent.h | ScriptComponent | 可执行脚本引用或实例，供脚本子系统执行 |
| 005-Entity | TenEngine::entity | — | struct | 效果组件 | TenEngine/entity/EffectComponent.h | EffectComponent | 粒子/VFX 等效果引用 |
| 005-Entity | TenEngine::entity | — | struct | 贴花组件 | TenEngine/entity/DecalComponent.h | DecalComponent | 贴花渲染参数与引用 |
| 005-Entity | TenEngine::entity | — | struct | 地形组件 | TenEngine/entity/TerrainComponent.h | TerrainComponent | 地形块引用 |
| 005-Entity | TenEngine::entity | — | struct | 光源组件 | TenEngine/entity/LightComponent.h | LightComponent | 光源类型与参数（方向光、点光、聚光等） |
| 005-Entity | TenEngine::entity | IComponentRegistry | 抽象接口/单例 | 组件类型注册 | TenEngine/entity/ComponentRegistry.h | IComponentRegistry::RegisterComponentType, GetComponentTypeInfo | `template<typename T> void RegisterComponentType(char const* name);` `IComponentTypeInfo const* GetComponentTypeInfo(TypeId id) const;` 程序员可快速注册自定义 Component |
| 005-Entity | TenEngine::entity | — | 自由函数 | 获取组件类型注册表 | TenEngine/entity/ComponentRegistry.h | GetComponentRegistry | `IComponentRegistry* GetComponentRegistry();` 或由 Subsystems 注册；调用方不拥有指针 |

*来源：用户故事 US-entity-001（场景通过 ECS 组织，Entity 为场景单元，多种 Component，程序员可快速定义与扩展类型）。*
