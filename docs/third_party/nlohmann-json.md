# nlohmann/json

## 名称与简介

**nlohmann/json**：仅头文件的 C++ JSON 解析与序列化库。用于配置、元数据、Resource/Tools 等。

## 仓库/来源

- **URL**：https://github.com/nlohmann/json  
- **推荐版本**：`v3.11.3` 或当前 v3.x 稳定 tag

## 许可证

MIT.

## CMake 集成

```cmake
include(FetchContent)
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG        v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)
# 使用: nlohmann_json::nlohmann_json
```

## 引用方式（自动集成）

- **变量**：`TENENGINE_USE_NLOHMANN_JSON=ON`。  
- **清单**：`nlohmann-json`。

## 可选配置

- 仅头文件，无额外编译选项；可关闭测试与文档以加快配置。

## 使用模块

013-Resource（元数据、配置）、025-Tools、配置与序列化、Editor 配置。
