# fmt

## 名称与简介

**fmt**：C++ 格式化库，被 spdlog 等使用。可单独用于字符串格式化或与 spdlog 配套。

## 仓库/来源

- **URL**：https://github.com/fmtlib/fmt  
- **推荐版本**：`10.2.0` 或当前 10.x 稳定 tag

## 许可证

MIT（或 fmt 许可证）。

## CMake 集成

```cmake
include(FetchContent)
FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG        10.2.0
)
set(FMT_DOC OFF CACHE BOOL "")
set(FMT_TEST OFF CACHE BOOL "")
FetchContent_MakeAvailable(fmt)
# 使用: fmt::fmt
```

## 引用方式（自动集成）

- **变量**：`TENENGINE_USE_FMT=ON`（可选；若 spdlog 使用 bundled fmt 可 OFF）。  
- **清单**：`fmt`。  
- 当同时启用 spdlog 与 fmt 时，可设置 spdlog 使用外部 fmt 以统一版本。

## 可选配置

- `FMT_DOC` / `FMT_TEST` OFF：减少配置与编译量。

## 使用模块

与 spdlog 配套；或 Core/工具中直接使用格式化时。
