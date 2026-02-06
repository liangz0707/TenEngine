# TenEngine-005-Entity Module

Entity模块提供实体与组件模型，集成ISceneNode接口以纳入场景管理。

## 依赖

- 001-Core
- 002-Object
- 004-Scene

## 构建

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## 测试

测试会自动构建（除非设置了`TENENGINE_SKIP_DEPENDENCY_TESTS`）。

运行测试：
```bash
./bin/te_entity_tests
```

## 主要功能

- Entity类实现ISceneNode接口
- 组件系统（Component）基础架构
- EntityManager管理Entity生命周期
- ComponentQuery组件查询
- ComponentRegistry组件类型注册
- 可选ECS系统（System/SystemManager）

注意：
- Entity模块不提供任何具体的Component实现
- 所有Component应由各自的模块实现（如World模块实现ModelComponent，Scene模块实现TransformComponent等）
- 详见 `docs/ComponentUsageGuide.md` 了解如何实现和使用Component

## 文件结构

```
TenEngine-005-entity/
├── CMakeLists.txt
├── cmake/
│   ├── TenEngineHelpers.cmake
│   └── TenEngineModuleDependencies.cmake
├── include/te/entity/
│   ├── Entity.h
│   ├── EntityId.h
│   ├── EntityManager.h
│   ├── Component.h
│   ├── ComponentQuery.h
│   ├── ComponentRegistry.h
│   ├── ComponentRegistration.h
│   └── System.h
├── src/
│   ├── Entity.cpp
│   ├── EntityManager.cpp
│   ├── ComponentRegistry.cpp
│   ├── ComponentRegistration.cpp
│   └── System.cpp
├── docs/
│   └── ComponentUsageGuide.md
└── tests/
    ├── CMakeLists.txt
    └── unit/
        ├── main.cpp
        ├── test_entity.cpp
        ├── test_component.cpp
        ├── test_component_query.cpp
        └── test_entity_manager.cpp
```
