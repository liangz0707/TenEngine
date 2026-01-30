# Quickstart: 008-RHI 完整功能

**Feature**: 008-rhi-fullversion-001 | **Date**: 2026-01-29

## Prerequisites

- CMake 3.16+
- C++17 编译器
- **001-Core** 源码（同 worktree，如 `TenEngine-001-core`），经 `tenengine_resolve_my_dependencies` 引入。
- Vulkan SDK / D3D12 / Metal 等按后端与平台配置（见实现与构建选项）。

## Build

在 worktree 根 `TenEngine-008-rhi` 下：

```bash
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build .
```

依赖 001-Core 按 `plan.md` **源码** 引入；`TENENGINE_CMAKE_DIR` 指向 `../TenEngine-001-core/cmake`（或 `TENENGINE_ROOT` 等）。详见 `docs/build-module-convention.md`。

## Run Tests

```bash
cd build
ctest
```

或直接运行 `te_rhi_test`（或 plan 中定义的测试可执行文件）。

## Minimal Usage

```cpp
#include "te/rhi/device.hpp"
#include "te/rhi/command_list.hpp"
#include "te/rhi/types.hpp"

using namespace te::rhi;

SelectBackend(Backend::Vulkan);
IDevice* dev = CreateDevice();
if (!dev) return;

IQueue* gq = dev->GetQueue(QueueType::Graphics, 0);
ICommandList* cmd = dev->CreateCommandList();
if (!cmd) { DestroyDevice(dev); return; }

Begin(cmd);
// cmd->Draw(...) / Dispatch(...) / ResourceBarrier(...)
End(cmd);
Submit(cmd, gq);

dev->DestroyCommandList(cmd);
DestroyDevice(dev);
```

## References

- `plan.md` — 依赖引入、项目结构、契约更新（API 雏形）
- `specs/_contracts/008-rhi-public-api.md` — 对外 API 契约
- `specs/_contracts/pipeline-to-rci.md` — Pipeline↔RHI 提交约定
- `docs/build-module-convention.md` — 构建规约
