# OpenAL Soft（openal）

## 名称与简介

**OpenAL Soft**：跨平台 3D 音频 API 实现。可作为 016-Audio 的另一种后端（空间音效、多源混音等）。

## 仓库/来源

- **URL**：https://github.com/kcat/openal-soft  
- **推荐版本**：最新 release tag（如 `1.23.0`）

## 许可证

LGPL 2+（注意动态链接与分发合规）。

## CMake 集成

```cmake
include(FetchContent)
FetchContent_Declare(
  openal
  GIT_REPOSITORY https://github.com/kcat/openal-soft.git
  GIT_TAG        1.23.0
)
FetchContent_MakeAvailable(openal)
# 使用: OpenAL::OpenAL
```

## 引用方式（自动集成）

- **变量**：`TENENGINE_USE_OPENAL=ON`（016-Audio 选用 OpenAL 后端时）。  
- **清单**：`openal`。

## 可选配置

- 静态/动态链接与 LGPL 合规需按项目策略选择。

## 使用模块

016-Audio（可选后端）；与 miniaudio、FMOD、Wwise 等并列可选。
