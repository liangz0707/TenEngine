# libpng

## 引入方式

**source**（源码纳入工程，FetchContent 或 add_subdirectory 拉取并随主工程编译；依赖 zlib）

## 名称与简介

**libpng**：官方 PNG 图像编解码库。用于贴图管线中 PNG 的读取与写出、截图/导出等。

## 仓库/来源

- **URL**：https://github.com/glennrp/libpng  
- **推荐版本**：v1.6.x 或当前稳定 tag（如 1.6.43）

## 许可证

libpng 许可证（类似 BSD）。

## CMake 集成

```cmake
include(FetchContent)
FetchContent_Declare(
  libpng
  GIT_REPOSITORY https://github.com/glennrp/libpng.git
  GIT_TAG        v1.6.43
)
set(PNG_BUILD_ZLIB ON)
set(PNG_TESTS OFF CACHE BOOL "")
FetchContent_MakeAvailable(libpng)
# 使用: PNG::PNG（依赖 ZLIB::ZLIB，可同仓或系统）
```

## 引用方式（自动集成）

- **变量**：`TENENGINE_USE_LIBPNG=ON`。  
- **清单**：`libpng`。  
- 依赖 zlib；若未单独拉取，FetchContent 可同时拉取 zlib 或使用系统 zlib。

## 可选配置

- `PNG_TESTS OFF` 减少构建；ARM/NEON 等按平台启用可加速解码。

## 使用模块

013-Resource（贴图管线、PNG 导入/导出）；028-Texture（可选 LibPngImporter）；024-Editor、截图与资源工具。
