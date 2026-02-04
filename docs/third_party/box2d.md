# Box2D

## 引入方式

**source**（源码纳入工程，FetchContent 或 add_subdirectory 拉取并随主工程编译）

## 名称与简介

**Box2D**：2D 刚体物理引擎。用于 014-Physics 的 2D 部分、022-2D 模块。

## 仓库/来源

- **URL**：https://github.com/erincatto/box2d  
- **推荐版本**：`v2.4.1` 或当前 v2.x 稳定 tag

## 许可证

MIT（Box2D 2.x）。

## CMake 集成

```cmake
include(FetchContent)
FetchContent_Declare(
  box2d
  GIT_REPOSITORY https://github.com/erincatto/box2d.git
  GIT_TAG        v2.4.1
)
set(BOX2D_BUILD_TESTBED OFF CACHE BOOL "")
set(BOX2D_BUILD_UNIT_TESTS OFF CACHE BOOL "")
FetchContent_MakeAvailable(box2d)
# 使用: box2d::box2d（或 CMake 提供的 target 名）
```

## 引用方式（自动集成）

- **变量**：`TENENGINE_USE_BOX2D=ON`（014-Physics/022-2D 需要 2D 物理时）。  
- **清单**：`box2d`。

## 可选配置

- 关闭 testbed 与单元测试；仅链接 box2d 库。

## 使用模块

014-Physics（2D 部分）、022-2D。
