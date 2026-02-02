# US-lifecycle-006：引擎支持 Android/iOS 等平台、Vulkan/Metal/GLSL/DXIL 等接口、通过宏选择代码路径

- **标题**：引擎支持 **Android、iOS** 等平台；支持 **Vulkan**、**Metal（MTL）**、**GLSL**、**DXIL** 等图形/Shader 接口。**可以通过宏来判断执行哪一段代码**（平台与后端相关实现路径）。
- **编号**：US-lifecycle-006

---

## 1. 角色/触发

- **角色**：引擎与游戏侧程序员、构建/移植工程师
- **触发**：需要在 **Android、iOS** 等平台上运行引擎；需要支持 **Vulkan**、**Metal（MTL）**、**GLSL**、**DXIL**（及 HLSL、MSL 等）图形与 Shader 接口；需要**通过宏来判断执行哪一段代码**（平台相关、后端相关），编译时选择对应实现路径。

---

## 2. 端到端流程与约定

1. **引擎支持 Android、iOS 等平台**：001-Core 平台抽象支持 **Android**、**iOS** 及 Windows/Linux/macOS 等；平台检测与路径（文件、输入、窗口等）与具体 OS 解耦。**可以通过宏来判断执行哪一段代码**（如 `TE_PLATFORM_ANDROID`、`TE_PLATFORM_IOS`、`TE_PLATFORM_WIN` 等），编译时选择平台相关实现。
2. **支持 Vulkan、Metal（MTL）、GLSL、DXIL 等接口**：008-RHI 支持 **Vulkan**、**Metal（MTL）**、**D3D12/DXIL** 等图形后端；Shader 涉及 **GLSL**、**HLSL/DXIL**、**MSL** 等接口（与 010-Shader 对接）。**可以通过宏来判断执行哪一段代码**（如 `TE_RHI_VULKAN`、`TE_RHI_METAL`、`TE_RHI_D3D12`），编译时选择后端相关实现路径。
3. **宏选择代码路径**：平台与后端相关代码使用预定义宏（TE_PLATFORM_*、TE_RHI_*）在编译期选择执行哪一段代码，不依赖运行时多态或大量 if-else；构建时通过宏定义选择目标平台与图形后端。

---

## 3. 涉及模块

| 模块 | 职责摘要 |
|------|----------|
| 001-Core | 引擎支持 **Android、iOS** 等平台；平台检测与抽象；**通过宏选择平台代码路径**（TE_PLATFORM_ANDROID、TE_PLATFORM_IOS 等） |
| 008-RHI | 支持 **Vulkan**、**Metal（MTL）**、**D3D12/DXIL** 等图形接口；**GLSL**、**HLSL/DXIL**、**MSL** 等 Shader 接口（与 010-Shader 对接）；**通过宏选择后端代码路径**（TE_RHI_VULKAN、TE_RHI_METAL、TE_RHI_D3D12） |
| 010-Shader | Shader 源码/字节码涉及 GLSL、HLSL/DXIL、MSL；与 RHI 后端一一对应 |
| 文档 | docs/engine-abi-interface-generation-spec.md：平台与后端**可通过宏判断执行哪一段代码** |

---

## 4. 每模块职责与 I/O

### 001-Core

- **职责**：引擎支持 **Android、iOS** 等平台；平台抽象（文件、目录、时间、环境、路径、平台检测）；**可以通过宏来判断执行哪一段代码**（TE_PLATFORM_ANDROID、TE_PLATFORM_IOS、TE_PLATFORM_WIN 等），编译时选择平台相关实现。
- **输入**：编译期宏定义（目标平台）；运行时平台检测（可选）。
- **输出**：平台 API、平台宏约定；与具体 OS 解耦的抽象。

### 008-RHI

- **职责**：支持 **Vulkan**、**Metal（MTL）**、**D3D12/DXIL** 等图形接口；与 **GLSL**、**HLSL/DXIL**、**MSL** 等 Shader 接口对接（010-Shader 产出字节码）；**可以通过宏来判断执行哪一段代码**（TE_RHI_VULKAN、TE_RHI_METAL、TE_RHI_D3D12），编译时选择后端实现路径；多后端统一接口。
- **输入**：编译期宏定义（目标后端）；DeviceDesc 后端类型；Shader 字节码（SPIR-V/DXIL/MSL）。
- **输出**：IDevice、ICommandList、ISwapChain 等统一接口；BackendType 或 TE_RHI_* 宏约定。

---

## 5. 派生 ABI（与契约/ABI 对齐）

- **001-core-ABI**：平台支持 Android、iOS 等；**通过宏选择平台代码路径**（TE_PLATFORM_*）。
- **008-rhi-ABI**：支持 Vulkan、Metal（MTL）、D3D12/DXIL 及 GLSL、HLSL/DXIL、MSL；**通过宏选择后端代码路径**（TE_RHI_*）；DeviceDesc、BackendType 或宏约定。
- **docs/engine-abi-interface-generation-spec.md**：平台与后端**可通过宏判断执行哪一段代码**。

---

## 6. 验收要点

- 引擎支持 **Android、iOS** 等平台；平台相关代码**可以通过宏来判断执行哪一段代码**（如 TE_PLATFORM_ANDROID、TE_PLATFORM_IOS）。
- RHI 支持 **Vulkan**、**Metal（MTL）**、**D3D12/DXIL** 等图形接口，及 **GLSL**、**HLSL/DXIL**、**MSL** 等 Shader 接口；后端相关代码**可以通过宏来判断执行哪一段代码**（如 TE_RHI_VULKAN、TE_RHI_METAL、TE_RHI_D3D12）。
- 构建时通过宏定义选择目标平台与图形后端，编译期选择对应实现路径。
