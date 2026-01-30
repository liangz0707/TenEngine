# Vulkan Headers

## 名称与简介

**Vulkan Headers**：Vulkan 官方头文件与注册表，被 Volk、glslang、SPIRV-Tools 等依赖。008-RHI（Vulkan 后端）及 Shader 工具链需要。

## 仓库/来源

- **URL**：https://github.com/KhronosGroup/Vulkan-Headers  
- **推荐版本**：与 Vulkan SDK 或目标驱动兼容的 tag（如 `v1.3.268`）

## 许可证

Apache-2.0。

## CMake 集成

```cmake
include(FetchContent)
FetchContent_Declare(
  vulkan-headers
  GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers.git
  GIT_TAG        v1.3.268
)
FetchContent_MakeAvailable(vulkan-headers)
# 使用: Vulkan::Headers（INTERFACE，仅 include）
```

## 引用方式（自动集成）

- **变量**：`TENENGINE_USE_VULKAN_HEADERS=ON`（当使用 Volk/glslang 或 Vulkan 后端时，通常 ON）。  
- **清单**：`vulkan-headers`。  
- 可与系统/SDK 的 Vulkan 头并存；FetchContent 便于固定版本与 CI。

## 可选配置

- 若本机已安装 Vulkan SDK，也可 `find_package(Vulkan)` 使用 SDK 头；本集成用于无 SDK 或版本锁定场景。

## 使用模块

008-RHI、volk、glslang、SPIRV-Cross 等 Vulkan 相关集成。
