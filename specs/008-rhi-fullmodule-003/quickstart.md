# Quickstart: 008-RHI 完整模块 (008-rhi-fullmodule-003)

**Build root**: worktree root (e.g. `TenEngine-008-rhi`)  
**Dependency**: 001-Core **源码** (e.g. `../TenEngine-001-core`)

## Configure

```bash
cd <worktree-root>
cmake -B build -DTE_RHI_VULKAN=OFF   # stub build without Vulkan fetch
# Or with Vulkan: cmake -B build -DTE_RHI_VULKAN=ON
# Or Vulkan + D3D12 (Windows): cmake -B build -DTE_RHI_VULKAN=ON -DTE_RHI_D3D12=ON
```

**Quickstart validation (T083)**：在至少一个后端下执行 `cmake -B build`、`cmake --build build --config Release`、`ctest -C Release -R te_rhi`；6 个 RHI 测试应全部通过。

## Build

```bash
cmake --build build --config Release
```

## Test

```bash
cd build
ctest -C Release --output-on-failure
```

RHI tests: `te_rhi_test`, `te_rhi_command_list_test`, `te_rhi_resources_test`, `te_rhi_pso_test`, `te_rhi_sync_test`, `te_rhi_swapchain_test`.

## Options

- `TE_RHI_VULKAN`: Enable Vulkan backend (FetchContent volk + Vulkan-Headers). Default ON.
- `TE_RHI_D3D12`: Enable D3D12 backend (Windows only). Default ON.
- `TE_RHI_METAL`: Enable Metal backend (macOS). Default OFF.
- `TE_RHI_VALIDATION`: Enable Vulkan Validation Layer. Default OFF.
- `TE_RHI_DEBUG_LAYER`: Enable D3D12 Debug Layer. Default OFF.

### Validation & Debug (T082)

- **Vulkan Validation Layer**: Configure with `-DTE_RHI_VALIDATION=ON`. At runtime, `vkCreateInstance` enables `VK_LAYER_KHRONOS_validation` when this option is set. Use for debugging Vulkan API usage.
- **D3D12 Debug Layer**: Configure with `-DTE_RHI_DEBUG_LAYER=ON`. Typically used with Debug build (`--config Debug`). At runtime, when defined and `_DEBUG`, `D3D12GetDebugInterface` enables the debug layer. Use for debugging D3D12 API usage.
