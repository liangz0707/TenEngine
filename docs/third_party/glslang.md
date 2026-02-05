# glslang

## 引入方式

**source**（源码纳入工程，FetchContent 或 add_subdirectory 拉取并随主工程编译）

## 名称与简介

**glslang**：将 GLSL/HLSL 编译为 SPIR-V，与 Shaderc 同生态。用于 010-Shader 的离线或运行时编译。

## 仓库/来源

- **URL**：https://github.com/KhronosGroup/glslang  
- **推荐版本**：与 Vulkan SDK 兼容的 tag（如 `14.0.0` 或 Khronos 推荐版本）

## 许可证

BSD 3-Clause、Apache-2.0 等（见仓库 LICENSE）。

## CMake 集成

```cmake
include(FetchContent)
FetchContent_Declare(
  glslang
  GIT_REPOSITORY https://github.com/KhronosGroup/glslang.git
  GIT_TAG        14.0.0
)
set(SKIP_GLSLANG_INSTALL OFF CACHE BOOL "")
FetchContent_MakeAvailable(glslang)
# 使用: glslang::glslang, glslang::SPIRV
```

## 引用方式（自动集成）

- **变量**：`TENENGINE_USE_GLSLANG=ON`（当 010-Shader 需要 GLSL→SPIR-V 时）。  
- **清单**：`glslang`。

## 可选配置

- 仅需编译器库时可关闭安装与测试；依赖 Vulkan Headers。

## 使用模块

010-Shader（GLSL/HLSL 编译为 SPIR-V）、离线着色器管线。
