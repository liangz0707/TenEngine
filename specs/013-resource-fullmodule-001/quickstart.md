# Quickstart: 013-Resource 模块

**Feature**: 013-resource-fullmodule-001 | **Plan**: [plan.md](./plan.md) | **Spec**: [spec.md](./spec.md)

## 目标读者

需要在 TenEngine 中使用统一资源加载、缓存、寻址与导入/存盘的开发者；或为 013 注册 Loader/Importer/反序列化器的下游模块开发者。

## 前置条件

- 已引入 001-Core、002-Object、028-Texture 源码构建（见 plan.md「依赖引入方式」）。
- 契约与 ABI 以 `specs/_contracts/013-resource-public-api.md`、`specs/_contracts/013-resource-ABI.md` 为准。

## 5 分钟上手

### 1. 获取 ResourceManager

```cpp
#include <te/resource/ResourceManager.h>

IResourceManager* mgr = GetResourceManager();
```

### 2. 同步加载资源

```cpp
IResource* res = mgr->LoadSync("/assets/textures/hero.png", ResourceType::Texture);
if (res) {
    // 可向下转型为 ITextureResource（028 定义）
    res->Release();  // 与本次「获取」对应
}
```

### 3. 异步加载与回调

```cpp
LoadRequestId id = mgr->RequestLoadAsync(
    "/assets/models/level.fbx",
    ResourceType::Model,
    [](IResource* resource, LoadResult result, void* user_data) {
        if (result == LoadResult::Ok && resource)
            /* 根及递归依赖均已就绪 */;
        if (resource) resource->Release();
    },
    nullptr);
// 可选：mgr->GetLoadStatus(id); mgr->GetLoadProgress(id); mgr->CancelLoad(id);
```

### 4. 缓存查询（不触发加载）

```cpp
ResourceId id = /* 从某处获得 */;
IResource* cached = mgr->GetCached(id);
if (cached) {
    /* 使用 */;
    cached->Release();
}
// 未命中时 cached == nullptr，不会自动加载
```

### 5. 寻址与存盘

```cpp
char const* path = mgr->ResolvePath(resourceId);  // GUID → 路径
bool ok = mgr->Save(resource, "/out/saved.asset"); // 各模块产出内容，013 写盘
```

## 下游模块：注册 Loader / 反序列化器 / Importer

- **Loader**：实现 `IResourceLoader::CreateFromPayload(ResourceType, void* payload, IResourceManager*)`，在引擎/子系统初始化时调用 `mgr->RegisterResourceLoader(ResourceType::Texture, myTextureLoader);`。
- **反序列化器**：实现 `IDeserializer::Deserialize(buffer, size)` 返回 opaque payload；注册 `mgr->RegisterDeserializer(ResourceType::Texture, myDeserializer);`；013 读文件后按 type 调用反序列化器，再把 payload 传给对应 Loader。
- **Importer**：实现 `IResourceImporter`（DetectFormat、Convert 等），注册 `mgr->RegisterImporter(ResourceType::Texture, myImporter);`；`mgr->Import(path, type, out_metadata)` 时 013 按 type 分发。

## 关键约定（摘要）

| 约定 | 说明 |
|------|------|
| 同一 ResourceId 多次 Load | 返回同一 IResource*，引用计数增加；Release 次数须与获取次数匹配。 |
| GetCached 未命中 | 仅返回 nullptr，不触发加载。 |
| 循环引用 | 禁止；检测到则加载失败（LoadResult::Error）。 |
| 异步回调 | 仅在「根及递归依赖均加载完成」时调用一次 LoadCompleteCallback。 |
| EnsureDeviceResources | 由下游触发并转发给 IResource 实现；013 不创建 DResource、不调用 008。 |

## 下一步

- 实现细节与全量 ABI：见 [plan.md](./plan.md) 与 `specs/_contracts/013-resource-ABI.md`。
- 数据模型与状态：见 [data-model.md](./data-model.md)。
- 任务拆分与实现顺序：见 [tasks.md](./tasks.md)（由 /speckit.tasks 产出）。
