# libwebp

## 引入方式

**source**（源码纳入工程，FetchContent 或 add_subdirectory 拉取并随主工程编译）

## 名称与简介

**libwebp**：Google WebP 图像编解码库（支持有损/无损、动图）。用于贴图管线、Web 资源与带宽敏感场景。

## 仓库/来源

- **URL**：https://github.com/webmproject/libwebp  
- **推荐版本**：最新 release 如 1.4.x

## 许可证

BSD 3-Clause。

## CMake 集成

```cmake
include(FetchContent)
FetchContent_Declare(
  libwebp
  GIT_REPOSITORY https://github.com/webmproject/libwebp.git
  GIT_TAG        v1.4.0
)
FetchContent_MakeAvailable(libwebp)
# 使用: libwebp::libwebp 等（libwebpdemux 等按需）
```

## 引用方式（自动集成）

- **变量**：`TENENGINE_USE_WEBP=ON`。  
- **清单**：`libwebp`。

## 可选配置

- 按需启用动图（demux）、编码器；仅解码时可最小化链接目标。

## 使用模块

013-Resource（贴图管线、WebP 导入/导出）；Web 或带宽敏感资源。
