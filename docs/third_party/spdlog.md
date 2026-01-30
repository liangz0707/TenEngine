# spdlog

## 名称与简介

**spdlog**：高性能 C++ 日志库，支持多 sink、异步、格式化。用于 TenEngine 001-Core 及通用日志输出。

## 仓库/来源

- **URL**：https://github.com/gabime/spdlog  
- **推荐版本**：`v1.13.0` 或当前 v1.x 稳定 tag

## 许可证

MIT。

## CMake 集成

```cmake
include(FetchContent)
FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG        v1.13.0
)
set(SPDLOG_BUILD_SHARED OFF CACHE BOOL "")
FetchContent_MakeAvailable(spdlog)
# 使用: spdlog::spdlog
```

## 引用方式（自动集成）

- **变量**：`TENENGINE_USE_SPDLOG=ON`（默认 ON）。  
- **清单**：在第三方清单中加入 `spdlog`。  
- 本模块 `target_link_libraries(te_core PRIVATE spdlog::spdlog)` 等。

## 可选配置

- `SPDLOG_BUILD_SHARED OFF`：静态链接，避免 DLL 依赖。  
- 若需 fmt 独立：可同时启用 `fmt` 第三方，spdlog 可配置为使用外部 fmt。

## 使用模块

001-Core（及所有依赖 Core 的模块通过 Core 间接使用）、通用日志。
