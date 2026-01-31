# Zstandard（zstd）

## 引入方式

**source**（源码纳入工程，FetchContent 或 add_subdirectory 拉取并随主工程编译）

## 名称与简介

**Zstandard**：高性能压缩/解压库。用于 Resource 资产压缩、网络包、管线中间数据等。

## 仓库/来源

- **URL**：https://github.com/facebook/zstd  
- **推荐版本**：`v1.5.5` 或当前稳定 tag

## 许可证

BSD 3-Clause。

## CMake 集成

```cmake
include(FetchContent)
FetchContent_Declare(
  zstd
  GIT_REPOSITORY https://github.com/facebook/zstd.git
  GIT_TAG        v1.5.5
)
set(ZSTD_BUILD_PROGRAMS OFF CACHE BOOL "")
set(ZSTD_BUILD_SHARED OFF CACHE BOOL "")
FetchContent_MakeAvailable(zstd)
# 使用: libzstd_static 或 zstd::libzstd_static
```

## 引用方式（自动集成）

- **变量**：`TENENGINE_USE_ZSTD=ON`。  
- **清单**：`zstd`。

## 可选配置

- `ZSTD_BUILD_PROGRAMS OFF`：不构建命令行工具。  
- 静态库便于链接到各模块。

## 使用模块

013-Resource、026-Networking、资产管线、包体与传输压缩。
