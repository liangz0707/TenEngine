# Quickstart: 008-RHI 完整模块（含 DX11、DXR、ABI TODO）

**Feature**: 008-rhi-fullmodule-004 | **Branch**: `008-rhi-fullmodule-004`

## 1. 规约与契约

- **规约**：`docs/module-specs/008-rhi.md`
- **契约**：`specs/_contracts/008-rhi-public-api.md`
- **ABI**：`specs/_contracts/008-rhi-ABI.md`（本 feature 增量见 `specs/008-rhi-fullmodule-004/contracts/008-rhi-ABI-delta.md`）

## 2. 构建根目录与依赖

- **构建根目录**：当前 worktree 根（如 `G:\AIHUMAN\WorkSpaceSDD\TenEngine-008-rhi`）。
- **依赖**：001-Core 通过 TenEngineHelpers / `tenengine_resolve_my_dependencies` 以**源码**引入（同级 worktree 或 `TENENGINE_CMAKE_DIR`）。
- **out-of-source**：推荐 `build/` 在 worktree 根下。

## 3. 配置与构建

```bash
# 在 worktree 根目录
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
```

### 后端选项

| 选项 | 默认 | 说明 |
|------|------|------|
| TE_RHI_VULKAN | ON | Vulkan 后端 |
| TE_RHI_D3D12 | ON | D3D12 后端（含 DXR，Windows） |
| TE_RHI_D3D11 | OFF→ON（本 feature） | D3D11 后端（Windows） |
| TE_RHI_METAL | OFF | Metal 后端（macOS） |
| TE_RHI_VALIDATION | OFF | Vulkan Validation Layer |
| TE_RHI_DEBUG_LAYER | OFF | D3D12 Debug Layer |

示例（启用 D3D11 + D3D12 + Vulkan，开启 D3D12 调试层）：

```bash
cmake .. -DCMAKE_BUILD_TYPE=Release -DTE_RHI_D3D11=ON -DTE_RHI_DEBUG_LAYER=ON
cmake --build .
```

## 4. 运行测试

```bash
ctest
# 或单独运行
./te_rhi_test
./te_rhi_command_list_test
./te_rhi_resources_test
./te_rhi_pso_test
./te_rhi_sync_test
./te_rhi_swapchain_test
```

本 feature 完成后，测试应覆盖：

- CreateDevice(Backend::D3D11)、GetQueue、GetFeatures、GetLimits（若已实现）
- CreateFence(true/false)、Submit 全参（signalFence/waitSem/signalSem）
- DrawIndexed、SetViewport、SetScissor（若已实现）
- D3D12 下 DXR 接口（BuildAccelerationStructure、DispatchRays）可选测试

## 5. 使用示例（伪代码）

```cpp
#include "te/rhi/device.hpp"
#include "te/rhi/command_list.hpp"
#include "te/rhi/types.hpp"

te::rhi::SelectBackend(te::rhi::Backend::D3D11);  // 或 D3D12, Vulkan, Metal
te::rhi::IDevice* dev = te::rhi::CreateDevice();
if (!dev) { /* 后端不可用 */ return; }
te::rhi::IQueue* queue = dev->GetQueue(te::rhi::QueueType::Graphics, 0);
te::rhi::ICommandList* cmd = dev->CreateCommandList();
te::rhi::IFence* fence = dev->CreateFence(false);

te::rhi::Begin(cmd);
// cmd->SetViewport(...); cmd->SetScissor(...); cmd->DrawIndexed(...);
te::rhi::End(cmd);
te::rhi::Submit(cmd, queue, fence, nullptr, nullptr);  // 全参
te::rhi::Wait(fence);

dev->DestroyCommandList(cmd);
dev->DestroyFence(fence);
te::rhi::DestroyDevice(dev);
```

## 6. 本 feature 产出

- **Plan**：`specs/008-rhi-fullmodule-004/plan.md`
- **Research**：`specs/008-rhi-fullmodule-004/research.md`
- **Data Model**：`specs/008-rhi-fullmodule-004/data-model.md`
- **契约增量**：`specs/008-rhi-fullmodule-004/contracts/008-rhi-ABI-delta.md`
- **Tasks**：由 `/speckit.tasks` 生成 `specs/008-rhi-fullmodule-004/tasks.md`

实现须基于**全量 ABI**（现有 ABI 表 + 增量）进行，禁止长期 stub。
