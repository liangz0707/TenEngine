# libjpeg-turbo

## 引入方式

**source**（源码纳入工程，FetchContent 或 add_subdirectory 拉取并随主工程编译）

## 名称与简介

**libjpeg-turbo**：高性能 JPEG 编解码库（SIMD 加速）。用于贴图管线中 JPEG 的读取与写出、照片/缩略图等。

## 仓库/来源

- **URL**：https://github.com/libjpeg-turbo/libjpeg-turbo  
- **推荐版本**：最新 stable 如 3.0.x

## 许可证

IJG（类似 BSD）+ 可选 Turbo 条款。

## CMake 集成

```cmake
include(FetchContent)
FetchContent_Declare(
  libjpeg-turbo
  GIT_REPOSITORY https://github.com/libjpeg-turbo/libjpeg-turbo.git
  GIT_TAG        3.0.0
)
set(CMAKE_INSTALL_CMAKEDIR "lib/cmake" CACHE STRING "")
FetchContent_MakeAvailable(libjpeg-turbo)
# 使用: jpeg-static 或 libjpeg-turbo::jpeg
```

## 引用方式（自动集成）

- **变量**：`TENENGINE_USE_JPEG_TURBO=ON`。  
- **清单**：`libjpeg-turbo`。

## 可选配置

- 静态/动态库按项目选择；SIMD 自动检测，可按平台关闭以减小二进制。

## 使用模块

013-Resource（贴图管线、JPEG 导入/导出）；024-Editor、照片与缩略图。
