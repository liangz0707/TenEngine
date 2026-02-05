# Data Model: 013-Resource 完整模块

**Feature**: 013-resource-fullmodule-001  
**Spec**: [spec.md](./spec.md)

## 1. 核心实体

### 1.1 ResourceId / GUID

| 属性 | 说明 |
|------|------|
| 语义 | 资源全局唯一标识；缓存键、FResource 间引用、可寻址路径、与 002 Object 引用解析对接 |
| 身份/唯一性 | 全局唯一；同一 ResourceId 对应同一逻辑资源；多次 Load 返回同一 IResource*（引用计数） |
| 生命周期 | 与资源绑定；ResolvePath(ResourceId) 可解析为路径或包内引用 |

### 1.2 IResource（抽象接口）

| 属性 | 说明 |
|------|------|
| 语义 | 可加载资源统一基类；013 缓存与返回均为 IResource* |
| 关键操作 | GetResourceType() → ResourceType；Release()；下游可向下转型为 ITextureResource、IMeshResource 等 |
| 生命周期 | 由 013 缓存或下游持有；Release 次数与「获取」次数匹配后资源才完全释放 |
| 状态 | 无显式状态机；加载完成后即可用；EnsureDeviceResources 由下游触发并转发给实现体 |

### 1.3 ResourceType

| 属性 | 说明 |
|------|------|
| 语义 | 资源类型枚举（Texture、Mesh、Material、Model、Effect、Terrain、Shader、Audio、Custom 等） |
| 用途 | RequestLoadAsync/LoadSync 的 type 参数；IResource::GetResourceType() 返回值；按 type 分发 Loader/Importer/反序列化器 |

### 1.4 IResourceManager（抽象接口）

| 属性 | 说明 |
|------|------|
| 语义 | 统一加载、缓存、卸载、加载工具入口 |
| 关键操作 | RequestLoadAsync、LoadSync、GetCached、Unload、GetLoadStatus、GetLoadProgress、CancelLoad、RequestStreaming、SetStreamingPriority、RegisterResourceLoader、RegisterImporter、Import、Save、ResolvePath（或等价） |
| 生命周期 | 由 Subsystems 或单例提供；调用方不拥有指针 |

### 1.5 加载请求与回调

| 实体 | 说明 |
|------|------|
| LoadRequestId | 不透明句柄；由 RequestLoadAsync 返回；用于 GetLoadStatus、GetLoadProgress、CancelLoad |
| LoadStatus | Pending、Loading、Completed、Failed、Cancelled |
| LoadResult | Ok、NotFound、Error、Cancelled；回调参数 |
| LoadCompleteCallback | (IResource* resource, LoadResult result, void* user_data)；在根及递归依赖均加载完成后调用一次 |
| StreamingHandle | 流式请求句柄；RequestStreaming、SetStreamingPriority 使用 |

### 1.6 资源三态（概念）

| 形态 | 说明 |
|------|------|
| FResource | 硬盘形态；引用仅通过 GUID；013 读文件得到 buffer |
| RResource | 运行时/内存形态；即实现 IResource 的对象；013 仅创建 RResource |
| DResource | GPU 形态；由 011/012/028 在 EnsureDeviceResources 时创建，013 不创建、不持有 |

### 1.7 注册与扩展

| 实体 | 说明 |
|------|------|
| IResourceLoader | 各模块实现；CreateFromPayload(type, payload, ...) 接收不透明 payload，创建 IResource 实现体并返回 |
| IResourceImporter | 各模块实现；DetectFormat、Convert、产出描述/数据、Metadata、Dependencies |
| 反序列化器 | 各模块按 ResourceType 注册；buffer → opaque payload；013 按 ResourceType 调用后把 payload 传给 Loader |

## 2. 关系与约束

- **缓存**：键为 ResourceId，值为 IResource*；同一 ResourceId 仅对应一份 RResource；引用计数或句柄计数与 Release 匹配。
- **依赖图**：无环；检测到循环引用则加载失败（LoadResult::Error）。
- **GetCached**：仅查询缓存；未命中返回 nullptr，不触发加载。
- **Save**：各模块从 IResource 产出内存内容，013 调用统一写接口落盘；各模块不直接写文件。
- **Load**：013 读文件 → 按 ResourceType 调反序列化器得 payload → 按 ResourceType 调 Loader 传入 payload；013 不解析 *Desc。

## 3. 状态与生命周期（摘要）

- **IResource***：获取（LoadSync/RequestLoadAsync 或 GetCached 命中）→ 使用 → Release/Unload（次数与获取次数匹配）→ 资源可回收。
- **LoadRequestId**：RequestLoadAsync 返回 → GetLoadStatus/GetLoadProgress/CancelLoad → 回调触发后请求结束。
- **StreamingHandle**：RequestStreaming 返回 → SetStreamingPriority → 流式生命周期内有效。
