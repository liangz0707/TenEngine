# Quickstart: 002-Object 完整模块

**Feature**: 002-object-fullmodule-001

## 前置条件

- C++17 编译器（MSVC 2017+ / GCC 7+ / Clang 5+）
- CMake 3.16+
- 001-Core 源码可用（同级 `Engine/TenEngine-001-core` 或通过 TENENGINE_001_CORE_DIR 指定）

## 构建根目录

本仓库为**单仓**布局；**构建根目录为仓库根**。在仓库根执行：

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

Windows + Visual Studio 2022 示例：

```bash
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

002-Object 的 target 名为 **te_object**；依赖 **te_core**（001-Core）由 CMake 通过 TenEngineHelpers / `tenengine_resolve_my_dependencies("002-object")` 以源码方式解析。

## 运行测试

在 `build` 目录下：

```bash
ctest
```

或按配置运行（如 Windows 多配置）：

```bash
ctest -C Release
```

单元测试覆盖：TypeRegistry（RegisterType、GetTypeByName、GetTypeById、CreateInstance）、Serializer 往返、PropertyBag 读写。测试仅 link te_object（依赖 te_core 传递）；不构建上游模块的 tests。

## 使用 002-Object API

1. **确保 Core 已初始化**：主工程或上层先调用 001-Core 的 Init。
2. **注册类型**：使用 `te::object::TypeRegistry::RegisterType(desc)` 注册 TypeDescriptor。
3. **查询与创建**：`GetTypeByName` / `GetTypeById` 获取描述；`CreateInstance(typeId)` 分配实例（使用 Core Alloc）。
4. **序列化**：实现 ISerializer，调用 Serialize/Deserialize；可选 SetVersionMigration 处理旧版本数据。
5. **属性**：对实现 PropertyBag 的对象使用 GetProperty/SetProperty/FindProperty。

头文件路径与 ABI 一致：`te/object/TypeId.hpp`、`te/object/TypeRegistry.hpp`、`te/object/Serializer.hpp` 等；include 目录为 `Engine/TenEngine-002-object/include`（或安装后的 include）。

## 依赖说明

- **001-core**：源码引入；见 `docs/engine-build-module-convention.md`。若 001-core 不存在或未解析，CMake 配置阶段应报错并退出。

## 参考

- 规约：`docs/module-specs/002-object.md`
- 契约与 ABI：`specs/_contracts/002-object-public-api.md`、`specs/_contracts/002-object-ABI.md`
- 本 feature 全量 ABI：`specs/002-object-fullmodule-001/plan.md`「全量 ABI 内容（实现参考）」
