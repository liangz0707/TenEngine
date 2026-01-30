# SPIRV-Cross

## 名称与简介

**SPIRV-Cross**：将 SPIR-V 反编译/转译为 HLSL、MSL、GLSL 等。用于 010-Shader 跨后端（Vulkan/D3D12/Metal）的 shader 产出。

## 仓库/来源

- **URL**：https://github.com/KhronosGroup/SPIRV-Cross  
- **推荐版本**：与 glslang/SPIR-V 版本兼容的 tag（如 `sdk-1.3.268.0` 或当前 sdk-*）

## 许可证

Apache-2.0（见仓库）。

## CMake 集成

```cmake
include(FetchContent)
FetchContent_Declare(
  spirv-cross
  GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Cross.git
  GIT_TAG        sdk-1.3.268.0
)
set(SPIRV_CROSS_CLI OFF CACHE BOOL "")
FetchContent_MakeAvailable(spirv_cross)
# 使用: spirv-cross-core, spirv-cross-glsl, spirv-cross-hlsl, spirv-cross-msl 等
```

## 引用方式（自动集成）

- **变量**：`TENENGINE_USE_SPIRV_CROSS=ON`。  
- **清单**：`spirv-cross`。

## 可选配置

- `SPIRV_CROSS_CLI OFF`：不构建命令行工具，仅库。  
- 按后端只启用需要的 target（glsl/hlsl/msl）。

## 使用模块

010-Shader、008-RHI 多后端 shader 转译。
