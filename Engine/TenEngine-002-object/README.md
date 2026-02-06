# TenEngine-002-object

002-Object 模块：反射、类型注册、序列化、属性系统。契约见 `specs/_contracts/002-object-public-api.md`，ABI 见 `002-object-ABI.md`。

## 构建

**构建根目录**：在本目录（`Engine/TenEngine-002-object`）下执行；001-core 以源码方式从 `../TenEngine-001-core` 解析。

```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

Windows + Visual Studio：`cmake .. -G "Visual Studio 17 2022" -A x64` 后 `cmake --build . --config Release`。

## 测试

```bash
ctest -C Release --output-on-failure
```

## 依赖

- **001-core**：源码（`../TenEngine-001-core` 或 `TENENGINE_001_CORE_DIR`）；仅使用契约声明的 Alloc/Free、容器等。

## 使用

1. 先调用 `te::core::Init(nullptr)`。
2. `TypeRegistry::RegisterType(desc)` 注册类型。
3. `GetTypeByName` / `GetTypeById` 查询，`CreateInstance(typeId)` 分配实例（Core Alloc）。
4. 实现 `ISerializer` 做序列化/反序列化；可选 `IVersionMigration`。
5. 实现 `PropertyBag` 做属性读写。

头文件：`include/te/object/*.hpp`（TypeId、TypeDescriptor、TypeRegistry、Serializer、PropertyBag 等）。
