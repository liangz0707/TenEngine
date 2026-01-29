# TenEngine 001-Core

001-Core is the **foundation root module** of TenEngine. It provides memory, thread, platform, log, math, containers, and module loading. No other engine modules depend on it. Public API is defined by **specs/_contracts/001-core-public-api.md**.

## Prerequisites

- C++17 compiler (MSVC 2017+ / GCC 7+ / Clang 5+)
- CMake 3.16+
- Windows / Linux / macOS

## Build

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

On Windows with Visual Studio 2022:

```bash
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

## Run unit tests

From the `build` directory:

```bash
ctest
```

On Visual Studio multi-config builds, tests may need to be run by config (e.g. run executables from `build/tests/Release/` on Windows).

Or run each test executable (paths relative to `build`):

- `tests/Release/test_alloc.exe` (Windows) or `tests/test_alloc` (Unix)
- `tests/Release/test_thread.exe` / `tests/test_thread`
- `tests/Release/test_platform.exe` / `tests/test_platform`
- `tests/Release/test_log.exe` / `tests/test_log`
- `tests/Release/test_math.exe` / `tests/test_math`
- `tests/Release/test_containers.exe` / `tests/test_containers`
- `tests/Release/test_module_load.exe` / `tests/test_module_load`

## Init and shutdown order (调用顺序与约束)

1. The main application must complete Core initialization before calling any submodule (e.g. if using a dynamic library, load it and call the init entry point).
2. Use Memory, Thread, Platform, Log, Math, Containers, ModuleLoad as needed; all public types and APIs are declared in **specs/_contracts/001-core-public-api.md**.
3. Before shutdown, release all resources allocated by Core and stop using handles; exact order and ABI are agreed between the implementation and the main application.

See **specs/_contracts/001-core-public-api.md** for the full contract and init/shutdown constraints.

## ABI versioning

Public API follows **MAJOR.MINOR.PATCH** (Constitution). Breaking changes increment MAJOR and require migration notes. Current implementation aligns with the contract API sketch (001-core-fullversion-001).

## Contract compliance

All public symbols under `include/te/core/*.h` are listed in **specs/_contracts/001-core-public-api.md** (API 雏形 or capability list). No extra public API is exposed.

## Module description and dependency

- **Module spec**: `docs/module-specs/001-core.md`
- **Contract**: `specs/_contracts/001-core-public-api.md`
- **Dependency map**: `specs/_contracts/000-module-dependency-map.md`
