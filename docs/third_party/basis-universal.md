# Basis Universal

## 引入方式

**source**（源码纳入工程，FetchContent 或 add_subdirectory 拉取并随主工程编译）

## 名称与简介

**Basis Universal**：GPU 友好纹理压缩（UASTC/ETC1S），输出 .basis 或 KTX2。用于贴图管线中压缩纹理的生成与运行时解码、减少显存与带宽。

## 仓库/来源

- **URL**：https://github.com/BinomialLLC/basis_universal  
- **推荐版本**：最新 release 或固定 tag

## 许可证

Apache-2.0（见仓库）。

## CMake 集成

```cmake
include(FetchContent)
FetchContent_Declare(
  basis_universal
  GIT_REPOSITORY https://github.com/BinomialLLC/basis_universal.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(basis_universal)
# 使用: basisu 编码/解码库；可选 basisu 命令行工具
```

## 引用方式（自动集成）

- **变量**：`TENENGINE_USE_BASISU=ON`（013-Resource、贴图压缩时）。  
- **清单**：`basis-universal`。

## 可选配置

- 仅需解码时可只链接解码端；编码端用于离线/管线生成 KTX2/Basis 资源。

## 使用模块

013-Resource（贴图压缩与 KTX2 产出）；008-RHI、009-RenderCore（上传前解码或 GPU 解码）。
