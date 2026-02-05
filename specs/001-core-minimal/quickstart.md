# Quickstart: 001-Core 最小切片（Alloc/Free、Log）

**Branch**: `001-core-minimal`  
**Purpose**: 构建、测试与示例调用本切片 Alloc/Free、Log。

## 前置条件

- C++17 编译器（MSVC 2017+ / GCC 7+ / Clang 5+）
- CMake 3.16+

## 构建

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

（具体 CMake 配置以仓库根 `CMakeLists.txt` 为准；本切片对应 001-Core 库目标。）

## 运行单元测试

```bash
ctest
# 或
./tests/unit/test_alloc
./tests/unit/test_log
```

测试覆盖：Alloc 成功/失败（size=0、非法 alignment）、Free(nullptr)、double-free、Log 各级别与过滤、线程安全写入。

## 示例：Alloc/Free

```cpp
#include "te/core/alloc.h"

void* p = te::core::Alloc(64, 16);
if (p) {
  // 使用 p
  te::core::Free(p);
}
te::core::Free(nullptr);  // no-op
```

## 示例：Log

```cpp
#include "te/core/log.h"

te::core::Log(te::core::LogLevel::Info, "started");
te::core::Log(te::core::LogLevel::Error, "failed");
// 配置级别过滤 / stderr 阈值后，低于阈值的级别不输出或输出到 stdout
```

（命名空间与头文件路径以实际实现为准；此处与 plan 中「契约更新」一致。）

## 下一步

- 将 plan.md 末尾「契约更新」写回 `specs/_contracts/001-core-public-api.md` 的「API 雏形」小节。
- 执行 `/speckit.tasks` 生成 tasks.md，再按 tasks 实现。
