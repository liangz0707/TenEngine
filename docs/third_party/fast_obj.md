# fast_obj

## 引入方式

**header-only**（单头文件 + 单实现，需在某一 .c 中 `#define FAST_OBJ_IMPLEMENTATION` 后 include）

## 名称与简介

**fast_obj**：单文件 C 库，快速解析 OBJ 模型（.obj）。轻量、无依赖，适合运行时或工具链中 OBJ 加载。

## 仓库/来源

- **URL**：https://github.com/thisistherk/fast_obj  
- **推荐版本**：最新 release 或固定 commit

## 许可证

MIT。

## CMake 集成

```cmake
include(FetchContent)
FetchContent_Declare(
  fast_obj
  GIT_REPOSITORY https://github.com/thisistherk/fast_obj.git
  GIT_TAG        master
)
FetchContent_Populate(fast_obj)
add_library(fast_obj INTERFACE)
target_include_directories(fast_obj INTERFACE ${fast_obj_SOURCE_DIR})
# 在一个 .c 中 #define FAST_OBJ_IMPLEMENTATION 后 #include "fast_obj.h"
```

## 引用方式（自动集成）

- **变量**：`TENENGINE_USE_FAST_OBJ=ON`（012-Mesh、013-Resource 需要 OBJ 时）。  
- **清单**：`fast_obj`。

## 可选配置

- 仅解析几何与材质引用；贴图路径由上层用 stb/libpng 等加载。

## 使用模块

012-Mesh、013-Resource（OBJ 模型加载）；与 assimp 可二选一（fast_obj 轻量、assimp 多格式）。
