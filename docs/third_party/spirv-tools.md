# SPIRV-Tools

## 引入方式

**source**（源码纳入工程，FetchContent 或 add_subdirectory 拉取并随主工程编译）

## 名称与简介

**SPIRV-Tools**：SPIR-V 验证、优化与反汇编工具库。glslang 的依赖（SPIRV-Tools-opt 等）；010-Shader 通过 glslang 间接使用，用于 SPIR-V 优化与校验。

## 仓库/来源

- **URL**：https://github.com/KhronosGroup/SPIRV-Tools  
- **推荐版本**：与 glslang 兼容的 tag（如 `v2023.7`、`sdk-1.3.268.0`）；须与 Vulkan-Headers、SPIRV-Headers 版本匹配

## 许可证

Apache-2.0（见仓库）。

## CMake 集成

SPIRV-Tools 依赖 SPIRV-Headers，通常由 glslang 的 `BUILD_EXTERNAL` 或统一 FetchContent 链拉取：

```cmake
# 若 glslang 未自动拉取，可显式拉取
include(FetchContent)
FetchContent_Declare(
  spirv-headers
  GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Headers.git
  GIT_TAG        sdk-1.3.268.0
)
FetchContent_MakeAvailable(spirv-headers)

FetchContent_Declare(
  spirv-tools
  GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Tools.git
  GIT_TAG        sdk-1.3.268.0
)
set(SPIRV_SKIP_TESTS ON CACHE BOOL "")
FetchContent_MakeAvailable(spirv-tools)
# 使用: SPIRV-Tools-opt 等
```

多数场景下 glslang 的 `FetchContent_MakeAvailable(glslang)` 会通过 `BUILD_EXTERNAL` 自动处理 SPIRV-Tools。

## 引用方式（自动集成）

- **变量**：随 glslang 启用；`TENENGINE_USE_GLSLANG=ON` 时由 glslang 依赖链拉取。  
- **清单**：`spirv-tools`（可选显式声明，用于自定义构建或调试）。

## 可选配置

- `SPIRV_SKIP_TESTS ON`：跳过 SPIRV-Tools 自身测试，加快构建。  
- 版本须与 glslang、Vulkan-Headers 对齐，避免 ABI 不兼容。

## 使用模块

010-Shader（经 glslang 间接）；glslang 构建依赖。
