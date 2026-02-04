# 005-Entity 模块 ABI

- **契约**：[005-entity-public-api.md](./005-entity-public-api.md)（能力与类型描述）
- **本文件**：005-Entity 对外 ABI 显式表。

## ABI 表

列定义：**模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明**

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 005-Entity | te::entity | IEntity | 抽象接口 | 实体（场景单元） | te/entity/Entity.h | IEntity::AddComponent, GetComponent, RemoveComponent, HasComponent | `template<typename T> T* AddComponent();` `template<typename T> T* GetComponent();` `template<typename T> void RemoveComponent();` `template<typename T> bool HasComponent() const;` |
| 005-Entity | te::entity | — | struct | 变换组件 | te/entity/TransformComponent.h | TransformComponent | 位置、旋转、缩放；层级父子由本模块或 Scene 管理 |
| 005-Entity | te::entity | — | struct | 模型组件 | te/entity/ModelComponent.h | ModelComponent | 网格/模型引用，供渲染收集 |
| 005-Entity | te::entity | — | struct | 脚本组件 | te/entity/ScriptComponent.h | ScriptComponent | 可执行脚本引用或实例，供脚本子系统执行 |
| 005-Entity | te::entity | — | struct | 效果组件 | te/entity/EffectComponent.h | EffectComponent | 粒子/VFX 等效果引用 |
| 005-Entity | te::entity | — | struct | 贴花组件 | te/entity/DecalComponent.h | DecalComponent | 贴花渲染参数与引用 |
| 005-Entity | te::entity | — | struct | 地形组件 | te/entity/TerrainComponent.h | TerrainComponent | 地形块引用 |
| 005-Entity | te::entity | — | struct | 光源组件 | te/entity/LightComponent.h | LightComponent | 光源类型与参数（方向光、点光、聚光等） |
| 005-Entity | te::entity | IComponentRegistry | 抽象接口/单例 | 组件类型注册 | te/entity/ComponentRegistry.h | IComponentRegistry::RegisterComponentType, GetComponentTypeInfo | `template<typename T> void RegisterComponentType(char const* name);` `IComponentTypeInfo const* GetComponentTypeInfo(TypeId id) const;` 程序员可快速注册自定义 Component |
| 005-Entity | te::entity | — | 自由函数 | 获取组件类型注册表 | te/entity/ComponentRegistry.h | GetComponentRegistry | `IComponentRegistry* GetComponentRegistry();` 或由 Subsystems 注册；调用方不拥有指针 |

*来源：用户故事 US-entity-001（场景通过 ECS 组织，Entity 为场景单元，多种 Component，程序员可快速定义与扩展类型）。*

---

## 数据相关 TODO

（依据 [docs/assets/013-resource-data-model.md](../../docs/assets/013-resource-data-model.md) §Entity/Component；本模块上游：001-Core、002-Object、004-Scene。）

### 注册

| 需提供 | 需调用上游 |
|--------|------------|
| [ ] 在引擎启动时注册 Component 类型（TransformComponent、ModelComponent 等） | 002：`RegisterType<ModelComponent>` 等 |

### 序列化 / 反序列化

| 需提供 | 需调用上游 |
|--------|------------|
| [ ] Component 属性按 002 可序列化约定；跨资源引用**仅存 ResourceId（GUID）** | 002：`Serialize` / `Deserialize`（由 004 SceneNodeDesc.components 或预制体流程调用） |

### 数据

- [ ] **ModelComponent**：modelGuid（ResourceId）
- [ ] **Component 内资源引用**：Mesh/Material/Texture/Model 均存 **ResourceId**，不长期持有 IResource* 指针

### 需提供的对外接口

- [ ] `GetModelGuid(IEntity*, ModelComponent*) → ResourceId` / `SetModelGuid(...)`
- [ ] `CreateEntityFromPrefab(prefabDesc) → IEntity*`：从预制体描述实例化 Entity 与 Component 树；prefabDesc 由调用方加载得到

### 调用流程

1. 预制体加载 → 调用方取得 prefabDesc → 005.CreateEntityFromPrefab(prefabDesc) → 005 创建 Entity 树，组件内 modelGuid 等存 ResourceId
2. 运行时绘制 → 调用方 005.GetModelGuid(entity) → 按 ResourceId 向 013 加载 → 取得 IModelResource* 后提交绘制
