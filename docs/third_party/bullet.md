# Bullet Physics（bullet）

## 引入方式

**source**（源码纳入工程，FetchContent 或 add_subdirectory 拉取并随主工程编译）

## 名称与简介

**Bullet**：成熟的 3D（及部分 2D）物理引擎，广泛用于游戏与仿真。可作为 014-Physics 的实现后端之一。

## 仓库/来源

- **URL**：https://github.com/bulletphysics/bullet3  
- **推荐版本**：`3.25` 或当前稳定 tag

## 许可证

Zlib（见仓库 LICENSE）。

## CMake 集成

```cmake
include(FetchContent)
FetchContent_Declare(
  bullet
  GIT_REPOSITORY https://github.com/bulletphysics/bullet3.git
  GIT_TAG        3.25
)
set(BUILD_BULLET2_DEMOS OFF CACHE BOOL "")
set(BUILD_UNIT_TESTS OFF CACHE BOOL "")
FetchContent_MakeAvailable(bullet)
# 使用: BulletDynamics, BulletCollision, LinearMath 等（见 Bullet 文档）
```

## 引用方式（自动集成）

- **变量**：`TENENGINE_USE_BULLET=ON`（014-Physics 选用 Bullet 时）。  
- **清单**：`bullet`。

## 可选配置

- 关闭 Demo 与单元测试；仅链接所需子库（Dynamics、Collision、LinearMath）。

## 使用模块

014-Physics（可选实现）；与 Jolt、PhysX、Box2D 等并列可选。
