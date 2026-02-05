# Quickstart: 013-Resource 最小切片（ResourceId / LoadSync / Release）

**Feature**: 013-resource-fullversion-001 | **Date**: 2026-01-29

## 1. 包含与依赖

- 链接本模块（如 `te_resource`）及上游 001-Core、002-Object（由 CMake `tenengine_resolve_my_dependencies("013-resource" ...)` 解析）。
- 包含公开头文件（路径以实际仓库为准），例如：

```cpp
#include <te_resource/resource.hpp>  // ResourceId, LoadHandle, LoadResult, LoadSync, Release
```

## 2. 基本用法

### 2.1 构造 ResourceId 并同步加载

```cpp
namespace tr = te::resource;

tr::ResourceId id;
id.kind = tr::ResourceId::Kind::Path;  // 或 Kind::Guid
id.value = "assets/textures/hero.png"; // 或 GUID 字符串

tr::LoadResult result = tr::LoadSync(id);
if (!result.success) {
    // 处理失败：result.error_code
    return;
}
tr::LoadHandle handle = result.handle;
// 使用 handle（如交给渲染/音频等下游）...
```

### 2.2 释放句柄

```cpp
tr::Release(handle);  // 句柄失效，资源可回收
// 再次 Release(handle) 为幂等，无操作
```

### 2.3 同一 ResourceId 多次加载

```cpp
tr::LoadResult a = tr::LoadSync(id);
tr::LoadResult b = tr::LoadSync(id);
// a.handle 与 b.handle 为两个独立句柄，须分别 Release
tr::Release(a.handle);
tr::Release(b.handle);
```

## 3. 错误处理

- LoadSync 永不抛异常；通过 `LoadResult.success` 与 `error_code` 表示失败。
- Release 永不抛异常；对无效或已释放句柄调用为幂等。

## 4. 调用顺序约束

- 须在 Core、Object 初始化之后调用 LoadSync/Release。
- ResourceId.value 的格式（路径或 GUID 字符串）须与 Object 序列化/引用解析约定一致。

## 5. 构建（CMake）

在模块根目录（TenEngine-013-resource）下：

```bash
cmake -B build -DTENENGINE_CMAKE_DIR="%cd%/cmake"
cmake --build build
```

依赖 001-core、002-object 时，通过 `TENENGINE_001_CORE_DIR`、`TENENGINE_002_OBJECT_DIR` 或 TENENGINE_ROOT 指定上游路径；未指定时按 `docs/build-module-convention.md` 默认解析。
