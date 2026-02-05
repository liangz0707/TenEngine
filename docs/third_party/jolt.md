# Jolt Physics（jolt）

## 引入方式

**source**（源码纳入工程，FetchContent 或 add_subdirectory 拉取并随主工程编译）

## 名称与简介

**Jolt Physics**：高性能 3D 物理引擎（原 Horizon Zero Dawn 所用）。可作为 014-Physics 的实现后端之一。

## 仓库/来源

- **URL**：https://github.com/jrouwe/JoltPhysics  
- **推荐版本**：最新 release tag（如 `v3.0.1`）

## 许可证

MIT（Jolt 部分）；部分依赖可能另有许可（见仓库）。

## CMake 集成

```cmake
include(FetchContent)
FetchContent_Declare(
  jolt
  GIT_REPOSITORY https://github.com/jrouwe/JoltPhysics.git
  GIT_TAG        v3.0.1
)
set(TARGET_UNIT_TESTS OFF CACHE BOOL "")
set(TARGET_HELLO_WORLD OFF CACHE BOOL "")
FetchContent_MakeAvailable(JoltPhysics)
# 使用: JoltPhysics（或文档中给出的 target 名）
```

## 引用方式（自动集成）

- **变量**：`TENENGINE_USE_JOLT=ON`（014-Physics 选用 Jolt 时）。  
- **清单**：`jolt`。

## 可选配置

- 关闭单元测试与示例以缩短构建；014-Physics 仅暴露 TenEngine 契约接口，内部封装 Jolt。

## 使用模块

014-Physics（可选实现）；与 Bullet、PhysX、Box2D 等并列可选。
