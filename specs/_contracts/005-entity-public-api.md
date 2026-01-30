# 契约：005-Entity 模块对外 API

## 适用模块

- **实现方**：**005-Entity**（T0 实体与组件模型）
- **对应规格**：`docs/module-specs/005-entity.md`
- **依赖**：001-Core（001-core-public-api）, 002-Object（002-object-public-api）, 004-Scene（004-scene-public-api）

## 消费者（T0 下游）

- 014-Physics, 015-Animation, 020-Pipeline, 026-Networking, 024-Editor, 027-XR。

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。
- 当前契约版本：（由实现或计划阶段填写）

## 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| EntityId | 实体唯一标识；与 Scene 节点关联或独立 | 创建后直至销毁 |
| ComponentHandle | 组件实例句柄；按类型查询、挂载/卸载 | 与实体或显式移除同生命周期 |
| Transform | 局部/世界变换（可与 Scene 共用或实体专用） | 与实体/节点同步 |
| ComponentQuery | 按类型/条件查询实体或组件；迭代接口 | 查询时有效 |

## 能力列表（提供方保证）

1. **实体**：CreateEntity、DestroyEntity、GetSceneNode、SetEnabled；生命周期与 Scene 节点可选绑定。
2. **组件**：RegisterComponentType、AddComponent、RemoveComponent、GetComponent；与 Object 反射联动。
3. **变换**：GetLocalTransform、SetLocalTransform、GetWorldTransform；与 Scene 节点共用或专用。
4. **可选 ECS**：RegisterSystem、ExecutionOrder、与主循环 Tick 集成；与传统组件模型边界明确。

## API 雏形（简化声明）

本切片（005-entity-full）暴露：实体、组件、变换（委托 Scene）、ComponentQuery；仅使用 001-Core、002-Object、004-Scene 契约已声明类型。可选 ECS 本切片不实现。

### 类型与句柄（本切片）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| EntityId | 实体唯一标识；与 Scene NodeId 一一对应 | 创建后直至 DestroyEntity |
| ComponentHandle | 组件实例句柄；按类型查询、挂载/卸载 | 与实体或显式 RemoveComponent 同生命周期 |
| Transform | 局部/世界变换；与 004-Scene 契约一致（与 Core.Math 或共用类型） | 与实体/节点同步 |
| ComponentQuery | 按类型/条件查询实体或组件；迭代接口；**本帧不包含**语义（创建时快照） | 查询创建后单次有效，迭代结束即失效 |

- **EntityId**：不透明句柄；实现可采用 `using EntityId = void*;` 或 `struct EntityId { uintptr_t id; };` 等，与 NodeId 一一对应由实现保证。
- **ComponentHandle**：不透明句柄；实现可采用 `using ComponentHandle = void*;` 或类似；与实体及 TypeId 绑定。
- **Transform**：使用 004-Scene 契约中的 Transform（与 Core.Math 一致）；本模块不重新定义。
- **ComponentQuery**：可移动类型；创建时对符合条件的实体/组件做快照；迭代器单次有效，本帧内增删不反映于当前迭代。

### 实体（Entity）

```cpp
namespace te::entity {

using EntityId = /* 不透明句柄，与 Scene NodeId 一一对应 */;
using WorldRef = te::scene::WorldRef;   // 来自 004-Scene 契约
using NodeId   = te::scene::NodeId;     // 来自 004-Scene 契约

EntityId CreateEntity(WorldRef world);
void DestroyEntity(EntityId entity);
NodeId GetSceneNode(EntityId entity);
void SetEnabled(EntityId entity, bool enabled);
bool IsValid(EntityId entity);

}
```

### 组件（Component）

```cpp
namespace te::entity {

using ComponentHandle = /* 不透明句柄 */;
using TypeId          = te::object::TypeId;   // 来自 002-Object 契约

bool RegisterComponentType(TypeId typeId);
ComponentHandle AddComponent(EntityId entity, TypeId typeId);
void RemoveComponent(EntityId entity, ComponentHandle handle);
ComponentHandle GetComponent(EntityId entity, TypeId typeId);
bool IsValid(ComponentHandle handle);

}
```

### 变换（Transform，委托 Scene）

```cpp
namespace te::entity {

using Transform = te::scene::Transform;   // 来自 004-Scene 契约

Transform GetLocalTransform(EntityId entity);
void SetLocalTransform(EntityId entity, Transform const& t);
Transform GetWorldTransform(EntityId entity);

}
```

### 组件查询（ComponentQuery，本帧不包含）

```cpp
namespace te::entity {

struct ComponentQuery {
    static ComponentQuery ByComponentType(WorldRef world, TypeId componentType);
    struct Iterator {
        bool Next();
        EntityId GetEntityId() const;
        ComponentHandle GetComponentHandle() const;
    };
    Iterator Begin();
    Iterator End() const;
};

}
```

### 调用顺序与约束（本切片）

- 须在 Core、Object、Scene 可用之后使用；组件类型须先在 Object 中注册，再调用 `RegisterComponentType`。
- 实体销毁时须释放其组件与绑定资源；EntityId、ComponentHandle 稳定性约定须满足下游（Physics、Pipeline、Networking）。
- ComponentQuery 迭代采用**本帧不包含**语义：迭代基于创建时的快照；本帧内新增的实体/组件不出现于当前迭代，本帧内已销毁/已移除的项不访问。
- 可选 ECS（RegisterSystem、ExecutionOrder、Tick）本切片不实现，留待后续 feature。

## 调用顺序与约束

- 须在 Core、Object、Scene 可用之后使用；组件类型须在 Object 中注册。
- 实体销毁时须释放或转移组件与绑定资源；下游（Physics、Pipeline、Networking）依赖 EntityId 与 ComponentHandle 稳定性约定。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 按 005-Entity 模块规格与依赖表新增契约 |
| 2025-01-30 | API 雏形：由 plan（feature 005-entity-full）同步 |
