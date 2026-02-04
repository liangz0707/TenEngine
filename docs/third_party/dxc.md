# DirectX Shader Compiler（DXC）

## 引入方式

**sdk**（预编译库，vcpkg 或官方发布；源码构建依赖完整 LLVM，复杂度高，推荐 sdk）

## 名称与简介

**DirectX Shader Compiler（DXC）**：Microsoft 的 HLSL 编译器，产出 DXIL（DirectX 中间语言），用于 D3D12 后端。010-Shader 的 HLSL→DXIL 编译路径。基于 LLVM/Clang。

## 仓库/来源

- **URL**：https://github.com/microsoft/DirectXShaderCompiler  
- **推荐版本**：vcpkg `directx-dxc` 或 tag `v1.7.2212`、`v1.8.2403` 等与 D3D12 驱动兼容的版本

## 许可证

UIUC (University of Illinois/NCSA) + 第三方（见仓库 LICENSE）。

## CMake 集成

**vcpkg（推荐）**：

```cmake
# vcpkg install directx-dxc
find_package(directx-dxc CONFIG REQUIRED)
target_link_libraries(te_shader PRIVATE Microsoft::DirectXShaderCompiler)
```

**预编译 SDK / 手动**：

```cmake
find_path(DXC_INCLUDE_DIR dxcapi.h)
find_library(DXC_DXCOMPILER_LIB dxcompiler)
target_include_directories(te_shader PRIVATE ${DXC_INCLUDE_DIR})
target_link_libraries(te_shader PRIVATE ${DXC_DXCOMPILER_LIB})
```

## 引用方式（自动集成）

- **变量**：`TENENGINE_USE_DXC=ON`（当 010-Shader 需要 HLSL→DXIL 且 RHI 启用 D3D12 后端时）。  
- **清单**：`dxc`。

## 可选配置

- **平台**：Windows x64、Linux x64；不支持 ARM32、UWP、Xbox。  
- **后端选择**：按 `TE_RHI_D3D12` 宏启用；非 D3D12 后端可省略 DXC。  
- **源码构建**：可自行构建 DXC（依赖 LLVM），见官方 BuildingAndTestingDXC.rst；工程内通常采用 vcpkg 或预编译。

## 使用模块

010-Shader（HLSL 编译为 DXIL，D3D12 后端）；008-RHI 通过 Shader 间接使用。
