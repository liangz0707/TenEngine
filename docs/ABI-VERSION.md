# ABI versioning (001-Core)

Public API follows **MAJOR.MINOR.PATCH** per Constitution and contract (specs/_contracts/001-core-public-api.md).

- **MAJOR**: Breaking changes (removed or incompatible API); must include migration notes.
- **MINOR**: New backward-compatible API.
- **PATCH**: Backward-compatible fixes only.

Current implementation matches the contract API sketch (001-core-fullversion-001). Exact version numbers are set at release; the contract is the source of truth for public types and functions.

## Init and shutdown order (调用顺序与约束)

- Main application must complete Core initialization before calling any submodule.
- Before shutdown, release all resources allocated by Core and stop using handles.
- Exact init/shutdown order and ABI are agreed between the implementation and the main application; see specs/_contracts/001-core-public-api.md.
