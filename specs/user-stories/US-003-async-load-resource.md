# US-003：异步加载资源（已迁移至领域）

- **状态**：已迁移至 **resource** 领域，新编号 **US-resource-001**。
- **正式文档**：[domains/resource/US-resource-001-async-load-resource.md](./domains/resource/US-resource-001-async-load-resource.md)
- **领域索引**：[domains/resource/index.md](./domains/resource/index.md)

以下为原内容备份，以领域内文档为准。

---

# US-resource-001：异步加载资源（原 US-003）

- **标题**：异步加载资源并在回调后继续操作
- **编号**：US-003（现 US-resource-001）

---

## 1. 角色/触发

- **角色**：游戏逻辑或 Editor
- **触发**：需要加载某资源（如纹理、网格、材质），不阻塞主线程；加载完成后在回调中拿到资源句柄并继续使用（挂到实体、显示等）。

---

## 2. 端到端流程

1. 调用方向 **Resource** 模块请求异步加载，传入资源路径（或 ResourceId）、可选优先级、**完成回调**。
2. **Resource**：将请求加入加载队列；内部可能依赖 **Core** 分配器与 **Object** 类型信息。
3. 加载在后台线程或分帧进行；完成后在指定线程（主线程或调用线程）调用**完成回调**，传入资源句柄或错误码。
4. 回调内调用方使用资源（如交给 **Pipeline/Entity** 使用）；若加载失败，根据错误码重试或降级。
5. 资源生命周期由 Resource 模块或调用方约定（引用计数、显式释放等）。

---

## 3. 涉及模块

| 模块 | 职责摘要 |
|------|----------|
| 001-Core | 分配器、线程（Resource 内部使用） |
| 002-Object | 类型/反射（可选，资源反序列化时可能用到） |
| 013-Resource | 加载队列、异步加载、完成回调、ResourceId、资源句柄 |

---

## 4. 每模块职责与 I/O

### 013-Resource

- **职责**：提供异步加载 API；管理加载队列与后台加载；加载完成后调用用户回调并传入资源句柄或错误码。
- **输入**：路径或 ResourceId；可选优先级；`LoadCompleteCallback`（句柄、错误码、用户数据）。
- **输出**：`requestLoadAsync(...)`；回调中返回 `IResource*` 或句柄 + `LoadResult`；可选 `releaseResource()`。

---

## 5. 派生接口（ABI 条目）

### 013-Resource

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 013-Resource | TenEngine::resource | IResourceManager | 抽象接口 | 异步加载入口 | TenEngine/resource/ResourceManager.h | IResourceManager::requestLoadAsync | void requestLoadAsync(char const* path, LoadCompleteCallback on_done, void* user_data); 不阻塞，完成后在约定线程调用 on_done |
| 013-Resource | TenEngine::resource | — | 回调类型 | 加载完成回调 | TenEngine/resource/ResourceManager.h | LoadCompleteCallback | void (*)(IResource* resource, LoadResult result, void* user_data); result 表示成功/失败/取消 |
| 013-Resource | TenEngine::resource | IResource | 抽象接口 | 资源句柄 | TenEngine/resource/Resource.h | IResource | 不直接构造；由 requestLoadAsync 回调返回；release 由 Manager 或引用计数管理 |
| 013-Resource | TenEngine::resource | — | 枚举 | 加载结果 | TenEngine/resource/ResourceManager.h | LoadResult | enum class LoadResult { Ok, NotFound, Error, Cancelled }; |
| 013-Resource | TenEngine::resource | — | 自由函数 | 获取全局 ResourceManager | TenEngine/resource/ResourceManager.h | getResourceManager | IResourceManager* getResourceManager(); 由 Subsystems 注册或单例；调用方不拥有指针 |

---

*本故事派生出的 ABI 条目将同步到 `specs/_contracts/013-resource-ABI.md`。*
