# US-entity-001：场景通过 ECS 组织，Entity 为场景单元，多种 Component，程序员可快速定义与扩展类型

- **标题**：场景通过 ECS 方式组织；Entity 是场景单元，可有各种各样的 Component；含可执行脚本的 ScriptComponent、渲染相关的 ModelComponent/EffectComponent/DecalComponent/TerrainComponent/LightComponent、处理变换的 TransformComponent 等；程序员可以快速定义和扩展类型。
- **编号**：US-entity-001

---

## 1. 角色/触发

- **角色**：程序员（引擎或游戏侧）
- **触发**：在构建场景或运行时，希望用 **ECS** 组织场景：**Entity** 作为场景单元，挂载多种 **Component**（如 Transform、Model、Script、Light、Effect、Decal、Terrain 等）；并能够**快速定义和扩展**新的组件类型，无需改引擎核心即可注册新 Component。

---

## 2. 端到端流程

1. **场景**由 **Scene/World** 管理，其内容以 **Entity** 为单元；每个 Entity 拥有唯一 ID，可挂载零个或多个 **Component**。
2. **内置/常用 Component 类型**（示例，可按实现扩展）：
   - **TransformComponent**：位置、旋转、缩放，用于层级与渲染变换。
   - **ModelComponent**：网格/模型引用，供渲染 Pass 收集绘制。
   - **ScriptComponent**：可执行脚本引用或实例，供脚本子系统执行。
   - **EffectComponent**：粒子/VFX 等效果引用。
   - **DecalComponent**：贴花渲染。
   - **TerrainComponent**：地形块引用。
   - **LightComponent**：光源参数（方向光、点光、聚光等）。
   - 以及其它引擎或游戏自定义的 Component。
3. 程序员对某 Entity **addComponent\<T\>()**、**getComponent\<T\>()**、**removeComponent\<T\>()**；通过 **IComponentRegistry** 或 **registerComponentType\<T\>()** 将自定义类型注册到引擎，即可在 Entity 上挂载与查询，实现**快速定义和扩展**。
4. 场景/渲染/脚本等系统通过 **Entity 查询**（如「带 TransformComponent + ModelComponent 的 Entity」）做剔除、收集、绘制或脚本驱动；Component 数据由 Entity 模块存储，类型信息由 **Object/反射** 模块提供（可选），便于序列化与编辑器展示。

---

## 3. 涉及模块

| 模块 | 职责摘要 |
|------|----------|
| 004-Scene | 场景/World 作为 Entity 的容器；创建/销毁 Entity、获取场景内 Entity 列表或迭代；场景根或层级（若与 Transform 结合） |
| 005-Entity | Entity、Component 的增删查；内置 Component 类型（Transform、Model、Script、Effect、Decal、Terrain、Light 等）；IComponentRegistry 或 registerComponentType 供程序员扩展 |
| 002-Object | 类型注册、反射、序列化；供 Entity 模块「快速定义和扩展」组件类型时使用（registerType、getTypeInfo） |

---

## 4. 每模块职责与 I/O

### 004-Scene

- **职责**：提供 **ISceneWorld** 或 **IScene**，作为 Entity 的容器；**createEntity**、**destroyEntity**；可选 **getRootEntities** 或按层级/空间索引查询 Entity；场景加载/卸载时创建或销毁其下 Entity。
- **输入**：创建/加载场景请求；Entity 模块的 Entity 句柄。
- **输出**：ISceneWorld::createEntity、destroyEntity、getEntities（或迭代器）；Scene 与 Entity 一一对应或一对多（场景包含多个 Entity）。

### 005-Entity

- **职责**：**IEntity** 表示场景单元，支持 **addComponent\<T\>()**、**getComponent\<T\>()**、**removeComponent\<T\>()**；提供内置 Component 类型：**TransformComponent**、**ModelComponent**、**ScriptComponent**、**EffectComponent**、**DecalComponent**、**TerrainComponent**、**LightComponent** 等；提供 **IComponentRegistry** 或 **registerComponentType\<T\>()**，使程序员可注册自定义 Component 类型，实现快速扩展；Component 数据存储与查询由本模块实现，类型信息可依赖 002-Object。
- **输入**：来自 Scene 的 createEntity 请求；程序员或系统的 addComponent/getComponent/removeComponent 调用；registerComponentType 注册的自定义类型。
- **输出**：IEntity、addComponent、getComponent、removeComponent、hasComponent；TransformComponent、ModelComponent、ScriptComponent 等类型；IComponentRegistry、registerComponentType、getComponentTypeInfo。

### 002-Object

- **职责**：提供**类型注册与反射**（**registerType**、**getTypeInfo**、**TypeId**）；Entity 模块在注册新 Component 类型时，可调用 Object 的 registerType 登记类型名、大小、序列化等，便于「快速定义和扩展」、编辑器属性面板与存盘。
- **输入**：Entity 模块或游戏代码调用 registerType\<T\>(name, ...)。
- **输出**：registerType、getTypeInfo、TypeId；可选序列化/反序列化接口。

---

## 5. 派生接口（ABI 条目）

以下按 `docs/engine-abi-interface-generation-spec.md` 书写。

### 004-Scene

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 004-Scene | TenEngine::scene | ISceneWorld | 抽象接口 | 场景世界（Entity 容器） | TenEngine/scene/SceneWorld.h | ISceneWorld::createEntity, destroyEntity | IEntity* createEntity(); void destroyEntity(IEntity* entity); 场景内 Entity 的创建与销毁 |
| 004-Scene | TenEngine::scene | ISceneWorld | 抽象接口 | 获取场景内 Entity | TenEngine/scene/SceneWorld.h | ISceneWorld::getEntities | 迭代器或 Entity 列表；供渲染/脚本等系统遍历 |
| 004-Scene | TenEngine::scene | — | 自由函数/单例 | 获取当前场景世界 | TenEngine/scene/SceneWorld.h | getSceneWorld | ISceneWorld* getSceneWorld(); 或由 Subsystems 注册；调用方不拥有指针 |

### 005-Entity

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 005-Entity | TenEngine::entity | IEntity | 抽象接口 | 实体（场景单元） | TenEngine/entity/Entity.h | IEntity::addComponent, getComponent, removeComponent, hasComponent | template\<T\> T* addComponent(); T* getComponent(); void removeComponent\<T\>(); bool hasComponent\<T\>() const; |
| 005-Entity | TenEngine::entity | — | struct | 变换组件 | TenEngine/entity/TransformComponent.h | TransformComponent | 位置、旋转、缩放；层级父子由本模块或 Scene 管理 |
| 005-Entity | TenEngine::entity | — | struct | 模型组件 | TenEngine/entity/ModelComponent.h | ModelComponent | 网格/模型引用，供渲染收集 |
| 005-Entity | TenEngine::entity | — | struct | 脚本组件 | TenEngine/entity/ScriptComponent.h | ScriptComponent | 可执行脚本引用或实例，供脚本子系统执行 |
| 005-Entity | TenEngine::entity | — | struct | 效果组件 | TenEngine/entity/EffectComponent.h | EffectComponent | 粒子/VFX 等效果引用 |
| 005-Entity | TenEngine::entity | — | struct | 贴花组件 | TenEngine/entity/DecalComponent.h | DecalComponent | 贴花渲染参数与引用 |
| 005-Entity | TenEngine::entity | — | struct | 地形组件 | TenEngine/entity/TerrainComponent.h | TerrainComponent | 地形块引用 |
| 005-Entity | TenEngine::entity | — | struct | 光源组件 | TenEngine/entity/LightComponent.h | LightComponent | 光源类型与参数（方向光、点光、聚光等） |
| 005-Entity | TenEngine::entity | IComponentRegistry | 抽象接口/单例 | 组件类型注册 | TenEngine/entity/ComponentRegistry.h | IComponentRegistry::registerComponentType, getComponentTypeInfo | void registerComponentType\<T\>(char const* name); 程序员可快速注册自定义 Component，Entity 即可 addComponent\<T\> |
| 005-Entity | TenEngine::entity | — | 自由函数 | 获取组件类型注册表 | TenEngine/entity/ComponentRegistry.h | getComponentRegistry | IComponentRegistry* getComponentRegistry(); 或由 Subsystems 注册 |

### 002-Object

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 002-Object | TenEngine::object | — | 自由函数/模板 | 类型注册 | TenEngine/object/TypeRegistry.h | registerType | void registerType\<T\>(char const* name, ...); 供 Entity 等模块注册组件类型，实现快速定义与扩展 |
| 002-Object | TenEngine::object | — | 类型/接口 | 类型信息 | TenEngine/object/TypeRegistry.h | ITypeInfo, getTypeInfo | ITypeInfo const* getTypeInfo(TypeId id); 类型名、大小、序列化等；Entity 注册 Component 时可选使用 |

---

## 6. 参考（可选）

- **Unity**：GameObject、Component（Transform、MeshRenderer、Light、MonoBehaviour/Script 等）；[DOTS Entities](https://docs.unity3d.com/Packages/com.unity.entities@latest)（IComponentData、ComponentSystem）。
- **Unreal**：Actor、UActorComponent（USceneComponent、UStaticMeshComponent、ULightComponent 等）；UClass 与反射注册。
- **快速扩展**：通过 registerType/registerComponentType 将自定义 struct 注册为 Component，即可在 Entity 上挂载与序列化，无需改引擎核心。

---

*本故事派生出的 ABI 条目将同步到 `specs/_contracts/004-scene-ABI.md`、`005-entity-ABI.md`、`002-object-ABI.md`。*
