# GLM（OpenGL Mathematics）

## 引入方式

**header-only**（仅头文件，无 IMPLEMENTATION 宏）

## 名称与简介

**GLM**：仅头文件的 C++ 数学库，提供向量、矩阵、四元数、常用数学函数等。与渲染/物理无关的纯数学也可使用。

## 仓库/来源

- **URL**：https://github.com/g-truc/glm  
- **推荐版本**：`1.0.1` 或当前稳定 tag（如 1.0.x）

## 许可证

MIT。

## CMake 集成

```cmake
include(FetchContent)
FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG        1.0.1
)
FetchContent_MakeAvailable(glm)
# 使用: glm::glm（INTERFACE 库，仅 include）
```

## 引用方式（自动集成）

- **变量**：`TENENGINE_USE_GLM=ON`（默认 ON）。  
- **清单**：`glm`。  
- `target_link_libraries(本模块 PRIVATE glm::glm)` 即可获得 include 路径。

## 可选配置

- GLM 为 header-only，无额外编译选项；可按需定义 `GLM_FORCE_*` 等宏。

## 使用模块

001-Core（数学子模块）、004-Scene、005-Entity、008-RHI、009-RenderCore、010-Shader、012-Mesh、014-Physics、019-PipelineCore、020-Pipeline、022-2D、023-Terrain 等。
