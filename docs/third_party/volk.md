# Volk（Vulkan 加载器）

## 名称与简介

**Volk**：轻量级 Vulkan 加载器，在运行时加载 Vulkan 函数指针，可替代静态链接 Vulkan Loader。用于 008-RHI 的 Vulkan 后端。

## 仓库/来源

- **URL**：https://github.com/zeux/volk  
- **推荐版本**：最新 release tag 或固定 commit

## 许可证

MIT。

## CMake 集成

```cmake
include(FetchContent)
FetchContent_Declare(
  volk
  GIT_REPOSITORY https://github.com/zeux/volk.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(volk)
# 使用: volk::volk；需 Vulkan Headers，可配合 vulkan-headers 第三方
```

## 引用方式（自动集成）

- **变量**：`TENENGINE_USE_VOLK=ON`（当 RHI Vulkan 后端启用时）。  
- **清单**：`volk`。  
- 与 `vulkan-headers` 一起集成，RHI 模块 `target_link_libraries(te_rhi PRIVATE volk::volk)`。

## 可选配置

- 平台：需本机 Vulkan SDK 或驱动提供的 Vulkan 运行时；Volk 仅负责加载符号。

## 使用模块

008-RHI（Vulkan 后端）、009-RenderCore（若直接使用 Volk）。
