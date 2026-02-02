# Quickstart: 008-RHI 完整模块实现（含 ABI TODO）

**Branch**: `008-rhi-fullmodule-006`

## 构建根目录

- **Worktree 根目录**即构建根目录（如 `G:\AIHUMAN\WorkSpaceSDD\TenEngine-008-rhi`）。
- 在该目录下执行 CMake 配置与构建；推荐 out-of-source 构建目录 `build/`。

## 依赖

- **001-Core**：通过 TenEngineHelpers 以源码方式解析（同级 worktree 或 `TENENGINE_001_CORE_DIR`）。
- **Vulkan 后端**：volk + vulkan-headers 由 CMake FetchContent 拉取；需本机 Vulkan 驱动或 Vulkan SDK。
- **D3D11/D3D12**：Windows SDK；仅 Windows 平台。
- **Metal**：Xcode 与 macOS/iOS SDK；仅 Apple 平台。

## 配置选项（CMake）

与 008-rhi-fullmodule-005 一致：TE_RHI_VULKAN、TE_RHI_D3D12、TE_RHI_D3D11、TE_RHI_METAL、TE_RHI_VALIDATION、TE_RHI_DEBUG_LAYER。

## 构建步骤（示例）

```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

## 运行测试

```bash
cd build
ctest
```

## 本次 feature 新增用法示例（概念）

- **创建 Uniform 缓冲并写入**：`CreateBuffer(BufferDesc{ size, BufferUsage::Uniform })` → `UpdateBuffer(buf, 0, data, size)`。
- **绑定到命令列表**：`cmd->Begin(); cmd->SetUniformBuffer(slot, buffer, offset); cmd->Draw*(...); cmd->End(); Submit(cmd, queue);`

全量 ABI 与实现约束见 **`specs/008-rhi-fullmodule-006/contracts/008-rhi-ABI-full.md`**。
