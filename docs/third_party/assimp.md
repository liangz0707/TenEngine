# Assimp

## 名称与简介

**Assimp**：模型与场景导入库，支持多种格式（FBX、glTF、OBJ 等）。用于 012-Mesh、013-Resource、024-Editor 的资产导入（可选）。

## 仓库/来源

- **URL**：https://github.com/assimp/assimp  
- **推荐版本**：`v5.4.0` 或当前 v5.x 稳定 tag

## 许可证

BSD 3-Clause。

## CMake 集成

```cmake
include(FetchContent)
FetchContent_Declare(
  assimp
  GIT_REPOSITORY https://github.com/assimp/assimp.git
  GIT_TAG        v5.4.0
)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "")
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "")
set(BUILD_SHARED_LIBS OFF CACHE BOOL "")
FetchContent_MakeAvailable(assimp)
# 使用: assimp::assimp
```

## 引用方式（自动集成）

- **变量**：`TENENGINE_USE_ASSIMP=ON`（012/013/024 需要运行时导入时）。  
- **清单**：`assimp`。

## 可选配置

- 关闭工具与测试；按格式需求可关闭不需要的格式以减小库体积。

## 使用模块

012-Mesh、013-Resource、024-Editor（可选）；若仅用离线转换工具则可不必链接 Assimp。
