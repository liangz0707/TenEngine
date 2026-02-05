# Quickstart: 001-Core 完整功能集

**Branch**: `001-core-fullversion-001`  
**Purpose**: 构建、测试与示例调用本 feature 全部 7 个子模块。

## 前置条件

- C++17 编译器（MSVC 2017+ / GCC 7+ / Clang 5+）
- CMake 3.16+
- 目标平台：Windows / Linux / macOS

## 构建

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

（具体 CMake 配置以仓库根 `CMakeLists.txt` 为准；本 feature 对应 001-Core 完整库。）

## 运行单元测试

```bash
ctest
# 或分别运行各子模块测试
./tests/unit/test_alloc
./tests/unit/test_thread
./tests/unit/test_platform
./tests/unit/test_log
./tests/unit/test_math
./tests/unit/test_containers
./tests/unit/test_module_load
```

测试覆盖：契约规定的成功路径与边界行为；可衡量性能/规模目标由 plan 产出并纳入验收。

## 初始化与调用顺序

1. 主工程先完成 Core 初始化（若以动态库形式则加载并调用初始化接口）。
2. 按需调用 Memory、Thread、Platform、Log、Math、Containers、ModuleLoad 各子能力。
3. 卸载前释放所有由 Core 分配的资源并停止使用句柄；具体顺序与 ABI 由实现与主工程约定。

## 示例（示意）

```cpp
#include "te/core/alloc.h"
#include "te/core/log.h"
#include "te/core/platform.h"
// ... 其他子模块头文件

// 初始化后
void* p = te::core::Alloc(64, 16);
te::core::Log(te::core::LogLevel::Info, "started");
// Platform、Thread、Math、Containers、ModuleLoad 按契约使用
te::core::Free(p);
// 卸载前释放资源
```

（命名空间与头文件路径以实际实现为准；与 plan 中「契约更新」一致。）

## 下一步

- 将 plan.md 末尾「契约更新」写回 `specs/_contracts/001-core-public-api.md` 的「API 雏形」小节（可与既有 001-core-minimal 雏形合并或替换）。
- 执行 `/speckit.tasks` 生成 tasks.md，再按 tasks 实现；仅暴露契约中声明的类型与 API。
