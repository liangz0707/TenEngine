# Google Test（gtest）

## 名称与简介

**Google Test**：C++ 单元测试框架，与 Google Mock 同仓。用于 TenEngine 各模块 `tests/` 下的单元测试。

## 仓库/来源

- **URL**：https://github.com/google/googletest  
- **推荐版本**：`v1.14.0` 或当前稳定 tag（如 `v1.15.x`）

## 许可证

BSD 3-Clause。

## CMake 集成

```cmake
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
# 使用: GTest::gtest, GTest::gtest_main, GMock::gmock, GMock::gmock_main
```

## 引用方式（自动集成）

- **变量**：`TENENGINE_USE_GTEST=ON`（默认 ON 当 `BUILD_TESTS=ON` 时）。  
- **清单**：在 `third_party_manifest.cmake` 或模块依赖列表中加入 `gtest`。  
- 构建脚本检测到 `gtest` 时执行上述 `FetchContent`，并对外提供 `GTest::gtest` / `GTest::gtest_main`。

## 可选配置

- `gtest_force_shared_crt ON`：Windows 下与主工程 CRT 一致。  
- 仅需 gtest 不需 gmock 时，可只 `target_link_libraries(测试目标 GTest::gtest_main)`。

## 使用模块

各模块 tests/（001-Core～027-XR 等），仅本模块测试可执行文件 link，不引入子分支的测试工程。
