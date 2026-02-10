# Component实现和使用指南

## 概述

Entity模块提供了组件系统的基础架构，但不提供任何具体的Component实现。所有Component都应该由各自的模块实现（如World模块实现ModelComponent，Scene模块实现TransformComponent等）。

## Component基类

所有Component必须继承自`te::entity::Component`基类：

```cpp
#include <te/entity/Component.h>

namespace te {
namespace entity {

class MyComponent : public Component {
public:
    // Component数据
    int myValue = 0;
    float myFloat = 0.0f;
    
    // 可选：生命周期回调
    void OnAttached(Entity* entity) override {
        // Component被添加到Entity时调用
    }
    
    void OnDetached(Entity* entity) override {
        // Component从Entity移除时调用
    }
};

}  // namespace entity
}  // namespace te
```

## 实现Component的步骤

### 1. 定义Component类

创建Component类，继承自`Component`基类：

```cpp
// MyComponent.h
#include <te/entity/Component.h>

namespace te {
namespace entity {

struct MyComponent : public Component {
    // 组件数据
    int value = 0;
    std::string name;
    
    // 构造函数
    MyComponent() = default;
    MyComponent(int v, std::string const& n) : value(v), name(n) {}
};

}  // namespace entity
}  // namespace te
```

### 2. 注册Component类型

在模块初始化时通过 ComponentRegistry 注册类型；**一次调用即同时注册到 Entity 与 002-Object**（反射/序列化可用）。其他模块（如 029-World 的 ModelComponent）在各自模块初始化时调用即可，无需修改 005 源码。

```cpp
#include <te/entity/ComponentRegistry.h>

void MyModule::Initialize() {
    IComponentRegistry* registry = GetComponentRegistry();
    registry->RegisterComponentType<MyComponent>("MyComponent");
}
```

### 3. 使用Component

#### 添加Component到Entity

```cpp
#include <te/entity/Entity.h>
#include <te/entity/EntityManager.h>

// 创建Entity
Entity* entity = EntityManager::GetInstance().CreateEntity(world, "MyEntity");

// 添加Component
MyComponent* comp = entity->AddComponent<MyComponent>();
comp->value = 42;
comp->name = "Test";
```

#### 获取Component

```cpp
// 获取Component
MyComponent* comp = entity->GetComponent<MyComponent>();
if (comp) {
    // 使用Component
    int value = comp->value;
}

// 检查是否有Component
if (entity->HasComponent<MyComponent>()) {
    // Entity有MyComponent
}
```

#### 移除Component

```cpp
// 移除Component
entity->RemoveComponent<MyComponent>();
```

### 4. Component查询

使用ComponentQuery查询具有特定Component的Entity：

```cpp
#include <te/entity/ComponentQuery.h>

// 查询所有有MyComponent的Entity
std::vector<Entity*> entities;
ComponentQuery::Query<MyComponent>(entities);

// 查询有多个Component的Entity（AND查询）
std::vector<Entity*> entities;
ComponentQuery::Query<MyComponent, AnotherComponent>(entities);

// 迭代查询
ComponentQuery::ForEach<MyComponent>([](Entity* e, MyComponent* comp) {
    // 处理每个Entity和Component
    comp->value++;
});
```

## 示例：TransformComponent实现

以下是一个TransformComponent的实现示例（实际实现应由Scene模块或游戏代码提供）：

```cpp
// TransformComponent.h
#include <te/entity/Component.h>
#include <te/scene/SceneTypes.h>

namespace te {
namespace entity {

/**
 * @brief Transform component
 * 
 * 提供Entity的变换访问接口。
 * 实际变换数据存储在Entity的ISceneNode接口中。
 */
class TransformComponent : public Component {
public:
    // 注意：TransformComponent可以作为便捷访问接口
    // 或者作为数据存储（如果不需要与ISceneNode同步）
    
    // 方式1：作为便捷接口（推荐，因为Entity已经实现ISceneNode）
    static te::scene::Transform GetLocalTransform(Entity* entity);
    static void SetLocalTransform(Entity* entity, te::scene::Transform const& t);
    static te::scene::Transform GetWorldTransform(Entity* entity);
    
    // 方式2：作为数据存储（如果需要独立于ISceneNode）
    // te::scene::Transform localTransform;
    // te::scene::Transform worldTransform;
};

}  // namespace entity
}  // namespace te
```

```cpp
// TransformComponent.cpp
#include <te/entity/TransformComponent.h>
#include <te/entity/Entity.h>

namespace te {
namespace entity {

te::scene::Transform TransformComponent::GetLocalTransform(Entity* entity) {
    if (!entity) return te::scene::Transform{};
    return entity->GetLocalTransform();  // 通过ISceneNode接口获取
}

void TransformComponent::SetLocalTransform(Entity* entity, te::scene::Transform const& t) {
    if (!entity) return;
    entity->SetLocalTransform(t);  // 通过ISceneNode接口设置
}

te::scene::Transform TransformComponent::GetWorldTransform(Entity* entity) {
    if (!entity) return te::scene::Transform{};
    return entity->GetWorldTransform();  // 通过ISceneNode接口获取
}

}  // namespace entity
}  // namespace te
```

## 示例：ModelComponent实现

ModelComponent应该由World模块实现：

```cpp
// World模块：ModelComponent.h
#include <te/entity/Component.h>
#include <te/resource/ResourceId.h>

namespace te {
namespace world {

struct ModelComponent : public te::entity::Component {
    te::resource::ResourceId modelResourceId;
    
    ModelComponent() : modelResourceId{} {}
    explicit ModelComponent(te::resource::ResourceId const& id) 
        : modelResourceId(id) {}
};

// World模块提供的便捷函数
te::resource::ResourceId GetEntityModelResourceId(te::entity::EntityId entity);
bool SetEntityModelResourceId(te::entity::EntityId entity, te::resource::ResourceId const& id);

}  // namespace world
}  // namespace te
```

## 最佳实践

1. **模块职责分离**：
   - Entity模块：提供组件系统基础架构
   - 各功能模块：实现各自的Component（World实现ModelComponent，Script实现ScriptComponent等）

2. **Component设计**：
   - Component应该只包含数据，不包含逻辑
   - 逻辑应该在System中实现（如果使用ECS架构）
   - 使用OnAttached/OnDetached进行初始化/清理

3. **类型注册**：
   - 在模块初始化时注册Component类型
   - 确保TypeId唯一性
   - 与Object模块集成以支持序列化

4. **资源引用**：
   - Component中只存储ResourceId，不存储IResource*指针
   - 资源加载和解析由Resource模块处理

5. **性能考虑**：
   - 使用ComponentQuery进行批量查询
   - 避免频繁的AddComponent/RemoveComponent操作
   - 考虑使用ECS架构以提高性能

## 相关文档

- Component基类：`include/te/entity/Component.h`
- Component查询：`include/te/entity/ComponentQuery.h`
- Component注册：`include/te/entity/ComponentRegistry.h`
- Entity类：`include/te/entity/Entity.h`
