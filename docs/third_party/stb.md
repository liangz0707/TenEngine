# stb（单头文件库）

## 引入方式

**header-only**（单头文件，需在某一 .cpp 中 `#define STB_*_IMPLEMENTATION` 后 include）

## 名称与简介

**stb**：一系列单头文件 C 库的合集，常用有 `stb_image`、`stb_image_write`、`stb_image_resize` 等。用于图像加载/写入及工具。

## 仓库/来源

- **URL**：https://github.com/nothings/stb  
- **推荐版本**：最新 master 或按需固定 commit（无正式版本号时可取 tag/commit）

## 许可证

MIT / Public Domain（各文件头注明）。

## CMake 集成

stb 为单头文件，通常不通过 FetchContent 编译为库，而是将 `stb` 目录加入 include，在需要处 `#define STB_IMAGE_IMPLEMENTATION` 等后 `#include "stb_image.h"`。若需统一管理：

```cmake
include(FetchContent)
FetchContent_Declare(
  stb
  GIT_REPOSITORY https://github.com/nothings/stb.git
  GIT_TAG        master
)
FetchContent_Populate(stb)
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${stb_SOURCE_DIR})
# 使用: target_link_libraries(本模块 PRIVATE stb)，并在一个 .cpp 中定义 STB_*_IMPLEMENTATION 后 include
```

## 引用方式（自动集成）

- **变量**：`TENENGINE_USE_STB=ON`。  
- **清单**：`stb`。  
- 拉取后将 `stb_SOURCE_DIR` 加入本模块 `target_include_directories`，并在单一翻译单元内完成 `STB_*_IMPLEMENTATION` 的 include。

## 可选配置

- 仅启用需要的头文件（如 stb_image、stb_image_write），避免未使用实现增大二进制。

## 使用模块

013-Resource（图像加载）、024-Editor、025-Tools、纹理/贴图工具。
