# 005-Entity 模块 ABI

- **契约**：[005-entity-public-api.md](./005-entity-public-api.md)（能力与类型描述）
- **本文件**：005-Entity 对外 ABI 显式表。

## ABI 表

列定义：**模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明**

### Entity类（实现ISceneNode接口）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 005-Entity | te::entity | Entity | 类 | 创建Entity | te/entity/Entity.h | Entity::Create | `static Entity* Create(te::scene::WorldRef world, char const* name = nullptr);` 创建新Entity并注册到Scene |
| 005-Entity | te::entity | Entity | 类 | 从节点创建Entity | te/entity/Entity.h | Entity::CreateFromNode | `static Entity* CreateFromNode(te::scene::NodeId nodeId, te::scene::WorldRef world);` 从现有Scene节点创建Entity |
| 005-Entity | te::entity | Entity | 类 | 销毁Entity | te/entity/Entity.h | Entity::Destroy | `void Destroy();` 注销Scene节点并清理组件 |
| 005-Entity | te::entity | Entity | 类 | 获取Entity ID | te/entity/Entity.h | Entity::GetEntityId | `EntityId GetEntityId() const;` 返回Entity唯一标识 |
| 005-Entity | te::entity | Entity | 类 | 获取Scene节点 | te/entity/Entity.h | Entity::GetSceneNode | `te::scene::ISceneNode* GetSceneNode();` 返回ISceneNode指针（this） |
| 005-Entity | te::entity | Entity | 类 | 获取World引用 | te/entity/Entity.h | Entity::GetWorldRef | `te::scene::WorldRef GetWorldRef() const;` 返回Entity所属的World引用 |
| 005-Entity | te::entity | Entity | 类 | 设置启用状态 | te/entity/Entity.h | Entity::SetEnabled | `void SetEnabled(bool enabled);` 设置Entity启用状态（对应ISceneNode::SetActive） |
| 005-Entity | te::entity | Entity | 类 | 查询启用状态 | te/entity/Entity.h | Entity::IsEnabled | `bool IsEnabled() const;` 查询Entity启用状态（对应ISceneNode::IsActive） |
| 005-Entity | te::entity | Entity | 类 | 添加组件 | te/entity/Entity.h | Entity::AddComponent | `template<typename T> T* AddComponent();` 添加组件到Entity，返回组件指针 |
| 005-Entity | te::entity | Entity | 类 | 获取组件 | te/entity/Entity.h | Entity::GetComponent | `template<typename T> T* GetComponent();` `template<typename T> T const* GetComponent() const;` 获取Entity的组件指针 |
| 005-Entity | te::entity | Entity | 类 | 移除组件 | te/entity/Entity.h | Entity::RemoveComponent | `template<typename T> void RemoveComponent();` 从Entity移除组件 |
| 005-Entity | te::entity | Entity | 类 | 检查组件 | te/entity/Entity.h | Entity::HasComponent | `template<typename T> bool HasComponent() const;` 检查Entity是否有指定组件 |
| 005-Entity | te::entity | Entity | 类 | ISceneNode层级接口 | te/entity/Entity.h | Entity::GetParent/SetParent/GetChildren/GetChildCount | `ISceneNode* GetParent() const override;` `void SetParent(ISceneNode* parent) override;` `void GetChildren(std::vector<ISceneNode*>& out) const override;` `size_t GetChildCount() const override;` |
| 005-Entity | te::entity | Entity | 类 | ISceneNode变换接口 | te/entity/Entity.h | Entity::GetLocalTransform/SetLocalTransform/GetWorldTransform/GetWorldMatrix | `Transform const& GetLocalTransform() const override;` `void SetLocalTransform(Transform const& t) override;` `Transform const& GetWorldTransform() const override;` `Matrix4 const& GetWorldMatrix() const override;` |
| 005-Entity | te::entity | Entity | 类 | ISceneNode标识接口 | te/entity/Entity.h | Entity::GetNodeId/GetName | `NodeId GetNodeId() const override;` `char const* GetName() const override;` |
| 005-Entity | te::entity | Entity | 类 | ISceneNode激活接口 | te/entity/Entity.h | Entity::IsActive/SetActive | `bool IsActive() const override;` `void SetActive(bool active) override;` |
| 005-Entity | te::entity | Entity | 类 | ISceneNode类型接口 | te/entity/Entity.h | Entity::GetNodeType | `NodeType GetNodeType() const override;` 返回Static或Dynamic |
| 005-Entity | te::entity | Entity | 类 | ISceneNode AABB接口 | te/entity/Entity.h | Entity::HasAABB/GetAABB | `bool HasAABB() const override;` `AABB GetAABB() const override;` 可选实现 |
| 005-Entity | te::entity | Entity | 类 | ISceneNode脏标记接口 | te/entity/Entity.h | Entity::IsDirty/SetDirty | `bool IsDirty() const override;` `void SetDirty(bool dirty) override;` |

### EntityId

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 005-Entity | te::entity | EntityId | struct | Entity唯一标识 | te/entity/EntityId.h | EntityId | `struct EntityId { void* value; bool IsValid() const; bool operator==(EntityId const&) const; struct Hash { ... }; };` |

### Component基类

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 005-Entity | te::entity | Component | 抽象基类 | 组件基类 | te/entity/Component.h | Component::OnAttached/OnDetached | `class Component { virtual void OnAttached(Entity*); virtual void OnDetached(Entity*); };` 所有Component必须继承此类 |
| 005-Entity | te::entity | ComponentHandle | struct | 组件句柄 | te/entity/Component.h | ComponentHandle | `struct ComponentHandle { EntityId entityId; TypeId componentTypeId; void* componentPtr; bool IsValid() const; };` |

### EntityManager

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 005-Entity | te::entity | EntityManager | 类/单例 | 获取单例 | te/entity/EntityManager.h | EntityManager::GetInstance | `static EntityManager& GetInstance();` 获取EntityManager单例 |
| 005-Entity | te::entity | EntityManager | 类/单例 | 创建Entity | te/entity/EntityManager.h | EntityManager::CreateEntity | `Entity* CreateEntity(te::scene::WorldRef world, char const* name = nullptr);` |
| 005-Entity | te::entity | EntityManager | 类/单例 | 从节点创建Entity | te/entity/EntityManager.h | EntityManager::CreateEntityFromNode | `Entity* CreateEntityFromNode(te::scene::NodeId nodeId, te::scene::WorldRef world);` |
| 005-Entity | te::entity | EntityManager | 类/单例 | 销毁Entity | te/entity/EntityManager.h | EntityManager::DestroyEntity | `void DestroyEntity(EntityId entityId);` `void DestroyEntity(Entity* entity);` |
| 005-Entity | te::entity | EntityManager | 类/单例 | 获取Entity | te/entity/EntityManager.h | EntityManager::GetEntity | `Entity* GetEntity(EntityId entityId);` |
| 005-Entity | te::entity | EntityManager | 类/单例 | 按名称查找Entity | te/entity/EntityManager.h | EntityManager::FindEntityByName | `Entity* FindEntityByName(te::scene::WorldRef world, char const* name);` |
| 005-Entity | te::entity | EntityManager | 类/单例 | 查询有指定组件的Entity（变参 AND） | te/entity/EntityManager.h | EntityManager::QueryEntitiesWithComponents | `template<typename... Components> void QueryEntitiesWithComponents(std::vector<Entity*>& out);` 单组件与多组件均用此变参接口 |
| 005-Entity | te::entity | EntityManager | 类/单例 | 获取World中的Entity | te/entity/EntityManager.h | EntityManager::GetEntitiesInWorld | `void GetEntitiesInWorld(te::scene::WorldRef world, std::vector<Entity*>& out);` |
| 005-Entity | te::entity | EntityManager | 自由函数 | 获取EntityManager | te/entity/EntityManager.h | GetEntityManager | `EntityManager* GetEntityManager();` 获取EntityManager指针 |

### ComponentQuery

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 005-Entity | te::entity | ComponentQuery | 静态类 | 组件查询（变参 AND） | te/entity/ComponentQuery.h | ComponentQuery::Query | `template<typename... Components> static void Query(std::vector<Entity*>& out);` 单组件与多组件均用此变参，内部调 EntityManager::QueryEntitiesWithComponents |
| 005-Entity | te::entity | ComponentQuery | 静态类 | 单组件迭代 | te/entity/ComponentQuery.h | ComponentQuery::ForEach | `template<typename T> static void ForEach(std::function<void(Entity*, T*)> const& callback);` 迭代有指定组件的Entity |
| 005-Entity | te::entity | ComponentQuery | 静态类 | 多组件迭代 | te/entity/ComponentQuery.h | ComponentQuery::ForEach | `template<typename... Components> static void ForEach(std::function<void(Entity*, Components*...)> const& callback);` 迭代有多个组件的Entity |

### ComponentRegistry

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 005-Entity | te::entity | IComponentRegistry | 抽象接口/单例 | 注册组件类型（模板） | te/entity/ComponentRegistry.h | IComponentRegistry::RegisterComponentType | `template<typename T> void RegisterComponentType(char const* name);` 头文件内实现，内部调 RegisterComponentTypeByNameAndSize；注册到 Entity 与 002-Object |
| 005-Entity | te::entity | IComponentRegistry | 抽象接口/单例 | 按名称与大小注册（类型擦除） | te/entity/ComponentRegistry.h | IComponentRegistry::RegisterComponentTypeByNameAndSize | `virtual void RegisterComponentTypeByNameAndSize(char const* name, std::size_t size) = 0;` 供模板或 029 等模块在自身 TU 实例化 RegisterComponentType<T> |
| 005-Entity | te::entity | IComponentRegistry | 抽象接口/单例 | 获取类型信息 | te/entity/ComponentRegistry.h | IComponentRegistry::GetComponentTypeInfo | `IComponentTypeInfo const* GetComponentTypeInfo(te::object::TypeId id) const;` `IComponentTypeInfo const* GetComponentTypeInfo(char const* name) const;` |
| 005-Entity | te::entity | IComponentRegistry | 抽象接口/单例 | 检查类型注册 | te/entity/ComponentRegistry.h | IComponentRegistry::IsComponentTypeRegistered | `bool IsComponentTypeRegistered(te::object::TypeId id) const;` |
| 005-Entity | te::entity | IComponentTypeInfo | struct | 组件类型信息 | te/entity/ComponentRegistry.h | IComponentTypeInfo | `struct IComponentTypeInfo { TypeId typeId; char const* name; size_t size; };` |
| 005-Entity | te::entity | — | 自由函数 | 获取组件注册表 | te/entity/ComponentRegistry.h | GetComponentRegistry | `IComponentRegistry* GetComponentRegistry();` 获取ComponentRegistry单例 |

### ComponentRegistration

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 005-Entity | te::entity | — | 自由函数 | 注册内置组件类型 | te/entity/ComponentRegistration.h | RegisterBuiltinComponentTypes | `void RegisterBuiltinComponentTypes();` Entity模块不提供内置组件，此函数为空；各模块应自行注册组件类型 |

### ECS System（可选）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 005-Entity | te::entity | System | 抽象基类 | System基类 | te/entity/System.h | System::GetExecutionOrder/Update/Initialize/Shutdown | `class System { virtual SystemExecutionOrder GetExecutionOrder() const; virtual void Update(float deltaTime); virtual void Initialize(); virtual void Shutdown(); };` |
| 005-Entity | te::entity | SystemExecutionOrder | 枚举 | System执行顺序 | te/entity/System.h | SystemExecutionOrder | `enum class SystemExecutionOrder { PreUpdate = 0, Update = 100, PostUpdate = 200, Render = 300, PostRender = 400 };` |
| 005-Entity | te::entity | SystemManager | 类/单例 | 获取单例 | te/entity/System.h | SystemManager::GetInstance | `static SystemManager& GetInstance();` |
| 005-Entity | te::entity | SystemManager | 类/单例 | 注册System | te/entity/System.h | SystemManager::RegisterSystem | `void RegisterSystem(std::unique_ptr<System> system);` |
| 005-Entity | te::entity | SystemManager | 类/单例 | 注销System | te/entity/System.h | SystemManager::UnregisterSystem | `void UnregisterSystem(System* system);` |
| 005-Entity | te::entity | SystemManager | 类/单例 | 设置执行顺序 | te/entity/System.h | SystemManager::SetExecutionOrder | `void SetExecutionOrder(System* system, SystemExecutionOrder order);` |
| 005-Entity | te::entity | SystemManager | 类/单例 | 更新System | te/entity/System.h | SystemManager::Update | `void Update(float deltaTime);` 按执行顺序更新所有System |
| 005-Entity | te::entity | SystemManager | 类/单例 | 初始化System | te/entity/System.h | SystemManager::Initialize | `void Initialize();` 初始化所有System |
| 005-Entity | te::entity | SystemManager | 类/单例 | 关闭System | te/entity/System.h | SystemManager::Shutdown | `void Shutdown();` 关闭所有System |
| 005-Entity | te::entity | — | 自由函数 | 获取SystemManager | te/entity/System.h | GetSystemManager | `SystemManager* GetSystemManager();` |

## 重要说明

1. **Entity实现ISceneNode**：Entity类直接继承并实现`te::scene::ISceneNode`接口，作为场景节点被Scene模块管理。Entity创建时自动调用`SceneManager::RegisterNode`注册，销毁时自动注销。

2. **变换管理**：Entity的变换直接通过ISceneNode接口管理（GetLocalTransform、SetLocalTransform、GetWorldTransform、GetWorldMatrix），与Scene节点共用变换数据。世界变换矩阵通过四元数到矩阵转换算法计算。

3. **Component实现**：Entity模块不提供任何具体的Component实现。所有Component（如TransformComponent、ModelComponent等）应由各自的模块实现。详见`Engine/TenEngine-005-entity/docs/ComponentUsageGuide.md`。

4. **资源处理**：Entity模块不处理资源相关内容。资源相关组件（如ModelComponent）应由World模块实现。Entity模块不依赖013-Resource模块。

5. **组件注册**：Component类型需要注册到ComponentRegistry和Object模块的TypeRegistry。各模块应在初始化时注册自己的Component类型。`RegisterBuiltinComponentTypes()`函数在Entity模块中为空实现。

6. **EntityId和WorldRef Hash支持**：EntityId提供Hash结构体用于unordered_map。EntityManager内部为WorldRef提供WorldRefHash结构体。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 005-Entity ABI |
| 2026-02-06 | 架构重构：Entity直接实现ISceneNode接口；移除ModelComponent和TransformComponent；更新ABI以反映实际实现 |
| 2026-02-10 | ComponentQuery 统一为变参 Query\<Components...\>；EntityManager 仅保留 QueryEntitiesWithComponents；IComponentRegistry 增加 RegisterComponentTypeByNameAndSize，RegisterComponentType\<T\> 在头文件内实现 |
| 2026-02-22 | Verified alignment with code: EntityId includes Hash struct; Component includes virtual destructor and OnAttached/OnDetached; Entity has both template and TypeId overloads for HasComponent/GetComponent; EntityManager has QueryEntitiesWithComponent<T> (single) and QueryEntitiesWithComponents<Components...> (variadic); ComponentQuery::ForEach has single and multi-component overloads; System has Initialize/Shutdown virtuals; SystemExecutionOrder values: PreUpdate=0, Update=100, PostUpdate=200, Render=300, PostRender=400 |
