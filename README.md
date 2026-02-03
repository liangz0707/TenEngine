# Branch: T0-009-render-core

This branch contains only **constraint files**, **module description**, and **global dependency** for module **009-render-core**.

- **Constraints**: `specs/_contracts/`
- **Module description**: `docs/module-specs/009-render-core.md`
- **Global dependency**: `docs/dependency-graph-full.md`, `specs/_contracts/000-module-dependency-map.md`

## Build and usage (009-RenderCore implementation)

- **No placeholders**: Build **must** introduce upstream by source (per `docs/engine-build-module-convention.md` and `docs/agent-build-guide.md`). Missing 008-RHI causes CMake **FATAL_ERROR**.
- **Upstream**: 009-RenderCore adds **008-RHI** via `add_subdirectory`; 008-RHI pulls in **001-Core** via TenEngineHelpers. Required: **008-RHI** as sibling directory (`TenEngine-008-rhi` or `008-rhi`) or set `TENENGINE_RHI_DIR`. 001-Core must be sibling of 008-RHI (e.g. `TenEngine-001-core`) so 008-RHI’s `tenengine_resolve_my_dependencies` can find it.
- **Configure**: `cmake -B build -DTENENGINE_RENDERCORE_BUILD_TESTS=ON`
- **Build**: `cmake --build build --config Release`
- **Tests**: `build/tests/Release/test_render_core.exe` (unit), `build/tests/Release/test_rhi_integration.exe` (contract). Unit test skips UniformBuffer when no RHI device; contract test verifies descriptors. CreateUniformBuffer/Update/Bind call RHI CreateBuffer/UpdateBuffer/SetUniformBuffer (no no-op).
- **Usage**: `#include <te/rendercore/api.hpp>`, namespace `te::rendercore`. Target: `te_rendercore`. See `specs/009-render-core-full-003/quickstart.md`. ResourceDesc, UniformLayout, PassProtocol, UniformBuffer. Single-thread use; caller ensures same-handle operations on same thread.
