# Quickstart: 001-Core

**Feature**: 001-core-fullmodule

## 构建

```bash
# 在仓库根目录（TenEngine-001-core 或当前 worktree 根）
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

或使用 Ninja：

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## 运行单元测试

```bash
cd build
ctest -C Release
# 或直接运行各 test_* 可执行文件
```

（Windows 下路径可能为 `build\tests\Release\test_alloc.exe` 等。）

## 调用顺序与约束（契约）

1. **进程启动**：建议先调用 `te::core::Init(nullptr)` 或 `Init(&params)`；失败则返回 false，成功后可重复调用（幂等）。
2. **使用 Core 能力**：在 Init 成功后使用内存、线程、平台、日志、数学、容器、模块加载等 API。
3. **进程退出前**：调用 `te::core::Shutdown()` 一次；之后不再使用 Core 分配的资源或句柄。
4. **内存**：通过 `Alloc`/`Free` 或 `GetDefaultAllocator()` 得到的 Allocator 进行分配/释放；`Free(nullptr)` 与 double-free 为 no-op。
5. **模块加载**：需要初始化/关闭回调时，先 `RegisterModuleInit`/`RegisterModuleShutdown`，在加载模块后调用 `RunModuleInit()`，卸载前调用 `RunModuleShutdown()`。

## 头文件与链接

- 包含头文件：`#include "te/core/alloc.h"`、`te/core/engine.h` 等（见 `specs/_contracts/001-core-ABI.md`）。
- 链接：链接 te_core 库（静态或动态，由 CMake 配置决定）。

## 规约与契约

- 规约：`docs/module-specs/001-core.md`
- 契约与 ABI：`specs/_contracts/001-core-public-api.md`、`specs/_contracts/001-core-ABI.md`
