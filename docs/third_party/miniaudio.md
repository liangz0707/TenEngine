# miniaudio

## 名称与简介

**miniaudio**：单文件、跨平台音频采集与播放库，无额外依赖。可作为 016-Audio 的后端实现之一。

## 仓库/来源

- **URL**：https://github.com/mackron/miniaudio  
- **推荐版本**：最新 release 或固定 commit（无版本号时可取 tag）

## 许可证

MIT / Public Domain（见仓库）。

## CMake 集成

miniaudio 多为单头文件 + 可选单实现文件。可 FetchContent 后以 INTERFACE 或 OBJECT 库形式加入 include，在一处定义 `MINIAUDIO_IMPLEMENTATION` 后 include：

```cmake
include(FetchContent)
FetchContent_Declare(
  miniaudio
  GIT_REPOSITORY https://github.com/mackron/miniaudio.git
  GIT_TAG        master
)
FetchContent_Populate(miniaudio)
add_library(miniaudio INTERFACE)
target_include_directories(miniaudio INTERFACE ${miniaudio_SOURCE_DIR})
# 在 016-Audio 的一个 .c/.cpp 中 #define MINIAUDIO_IMPLEMENTATION 后 #include "miniaudio.h"
```

## 引用方式（自动集成）

- **变量**：`TENENGINE_USE_MINIAUDIO=ON`（016-Audio 选用 miniaudio 后端时）。  
- **清单**：`miniaudio`。

## 可选配置

- 按平台启用对应后端（WASAPI/ALSA/CoreAudio 等由 miniaudio 内部选择）。

## 使用模块

016-Audio（可选后端）；与 FMOD/Wwise/OpenAL 等并列可选。
