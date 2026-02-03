# Quickstart: 008-RHI 完整模块实现

**Branch**: `008-rhi-fullmodule-005`

## 构建根目录

- **Worktree 根目录**即构建根目录（如 `G:\AIHUMAN\WorkSpaceSDD\TenEngine-008-rhi`）。
- 在该目录下执行 CMake 配置与构建；推荐 out-of-source 构建目录 `build/`。

## 依赖

- **001-Core**：通过 TenEngineHelpers 以源码方式解析（同级 worktree 或 `TENENGINE_001_CORE_DIR`）。
- **Vulkan 后端**：volk + vulkan-headers 由 CMake FetchContent 拉取；需本机 Vulkan 驱动或 Vulkan SDK。
- **D3D11/D3D12**：Windows SDK（随 Visual Studio 或独立安装）；仅 Windows 平台。
- **Metal**：Xcode 与 macOS/iOS SDK；仅 Apple 平台。

## 配置选项（CMake）

| 选项 | 默认 | 说明 |
|------|------|------|
| TE_RHI_VULKAN | ON | 启用 Vulkan 后端 |
| TE_RHI_D3D12 | ON（Windows） | 启用 D3D12 后端 |
| TE_RHI_D3D11 | OFF | 启用 D3D11 后端 |
| TE_RHI_METAL | OFF（非 Apple 为 OFF） | 启用 Metal 后端（Apple 下可按需 ON） |
| TE_RHI_VALIDATION | OFF | Vulkan 校验层 |
| TE_RHI_DEBUG_LAYER | OFF | D3D12 调试层 |

## 构建步骤（示例）

```bash
# 在 worktree 根目录
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

如需仅启用 Vulkan 与 D3D12（Windows）：

```bash
cmake .. -DCMAKE_BUILD_TYPE=Release -DTE_RHI_VULKAN=ON -DTE_RHI_D3D12=ON -DTE_RHI_D3D11=OFF
```

## 运行测试

```bash
cd build
ctest
# 或单独运行
./te_rhi_test           # 设备创建等
./te_rhi_command_list_test
./te_rhi_resources_test
./te_rhi_pso_test
./te_rhi_sync_test
./te_rhi_swapchain_test
```

测试会按编译进的后端（TE_RHI_VULKAN 等）调用 CreateDevice(对应 Backend)；不支持的 Backend 会返回 nullptr，测试可跳过或仅断言不崩溃。

## 使用示例（最小）

```cpp
#include "te/rhi/device.hpp"
#include "te/rhi/command_list.hpp"
#include "te/rhi/queue.hpp"

te::rhi::SelectBackend(te::rhi::Backend::Vulkan);
te::rhi::IDevice* dev = te::rhi::CreateDevice();
if (!dev) { /* 后端不可用 */ return; }
te::rhi::IQueue* queue = dev->GetQueue(te::rhi::QueueType::Graphics, 0);
te::rhi::ICommandList* cmd = dev->CreateCommandList();
te::rhi::Begin(cmd);
cmd->Draw(3, 1);
te::rhi::End(cmd);
te::rhi::Submit(cmd, queue);
dev->DestroyCommandList(cmd);
te::rhi::DestroyDevice(dev);
```

## 规约与契约

- **规约**：`docs/module-specs/008-rhi.md`
- **契约**：`specs/_contracts/008-rhi-public-api.md`
- **ABI**：`specs/_contracts/008-rhi-ABI.md`
- **上游契约**：`specs/_contracts/001-core-public-api.md`

实现仅使用上述契约与 ABI 中声明的类型与 API；四后端均为真实实现，禁止 stub 与未文档化 no-op。
