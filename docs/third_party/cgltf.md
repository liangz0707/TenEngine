# cgltf

## 名称与简介

**cgltf**：单头文件 C 库，用于解析 glTF 2.0（.gltf / .glb）。轻量、无额外依赖，适合运行时加载 glTF 模型与场景。

## 仓库/来源

- **URL**：https://github.com/jkuhlmann/cgltf  
- **推荐版本**：最新 release tag 或固定 commit

## 许可证

MIT。

## CMake 集成

```cmake
include(FetchContent)
FetchContent_Declare(
  cgltf
  GIT_REPOSITORY https://github.com/jkuhlmann/cgltf.git
  GIT_TAG        master
)
FetchContent_Populate(cgltf)
add_library(cgltf INTERFACE)
target_include_directories(cgltf INTERFACE ${cgltf_SOURCE_DIR})
# 在一个 .c 中 #define CGLTF_IMPLEMENTATION 后 #include "cgltf.h"
```

## 引用方式（自动集成）

- **变量**：`TENENGINE_USE_CGLTF=ON`（012-Mesh、013-Resource 需要 glTF 时）。  
- **清单**：`cgltf`。

## 可选配置

- 仅解析不写；若需写 glTF 可考虑 tinygltf 或自写导出。

## 使用模块

012-Mesh、013-Resource（glTF 模型/场景加载）；与 tinygltf、assimp 可二选一或并存（cgltf 轻量、assimp 多格式）。
