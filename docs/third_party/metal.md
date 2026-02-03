# Metal（Apple）

## 引入方式

**system**（Apple 平台系统框架，随 Xcode/macOS SDK 提供）

## 名称与简介

**Metal**：Apple 的底层图形与计算 API，用于 008-RHI 的 Metal 后端。支持 macOS、iOS、tvOS；对应 Unreal Metal RHI、Unity Metal 实现。

## 仓库/来源

- **来源**：Apple 系统框架，随 Xcode 和平台 SDK 提供  
- **框架**：`Metal.framework`、`MetalKit.framework`（可选）、`CoreGraphics.framework`、`QuartzCore.framework`（CAMetalLayer 等）  
- **头文件**：`<Metal/Metal.h>`、`<MetalKit/MetalKit.h>` 等  
- **平台**：macOS、iOS、tvOS

## 许可证

Apple proprietary；随 Xcode 与系统 SDK 授权使用。

## CMake 集成

```cmake
if(APPLE)
  find_library(METAL_LIBRARY Metal)
  find_library(QUARTZCORE_LIBRARY QuartzCore)
  find_library(COREGRAPHICS_LIBRARY CoreGraphics)
  target_link_libraries(te_rhi PRIVATE
    ${METAL_LIBRARY}
    ${QUARTZCORE_LIBRARY}
    ${COREGRAPHICS_LIBRARY}
  )
  # 头文件 <Metal/Metal.h> 等由系统 SDK 提供
endif()
```

若使用 MetalKit（可选，便于 SwapChain/View 集成）：

```cmake
find_library(METALKIT_LIBRARY MetalKit)
target_link_libraries(te_rhi PRIVATE ${METALKIT_LIBRARY})
```

## 引用方式（自动集成）

- **变量**：`TENENGINE_RHI_METAL=ON`（当 RHI Metal 后端启用时，默认 Apple 平台下可选）。  
- **清单**：`metal`。  
- **宏**：编译 Metal 后端代码时定义 `TE_RHI_METAL`。

## 可选配置

- **平台**：macOS 10.14+、iOS 12+（或按目标版本）；需安装 Xcode 及对应平台 SDK。  
- **Shader**：MSL（Metal Shading Language）；由 010-Shader 通过 SPIRV-Cross 或 glslang + spirv-cross 转译；或 Xcode 内建 `metal` 编译器离线编译。  
- **特性**：支持 Argument Buffers、Tile Shading、Ray Tracing（Metal 3）等；按需检测 `MTLDevice.supports*`。  
- **SwapChain**：使用 `CAMetalLayer` + `MTKView` 或自定义 `CAMetalDrawable` 获取每帧渲染目标。

## 使用模块

008-RHI（Metal 后端）；009-RenderCore、020-Pipeline、024-Editor（macOS/iOS 视图）等通过 RHI 间接使用。
