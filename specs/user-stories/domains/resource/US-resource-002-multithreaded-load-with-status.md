# US-resource-002：程序以多线程方式运行，资源在单独线程加载，任意线程可提交任务，有回调和状态提示

- **标题**：程序以多线程方式运行；资源加载在单独线程（或专用加载线程）执行；其他线程可向加载线程提交加载任务；加载完成有回调；可查询加载状态与进度。
- **编号**：US-resource-002

---

## 1. 角色/触发

- **角色**：游戏逻辑、渲染线程、主线程或其它工作线程
- **触发**：需要在**多线程环境**下加载资源：任意线程可**提交**加载任务到资源加载模块，不阻塞调用线程；加载在**专用加载线程**（或线程池）执行；加载完成通过**回调**通知；调用方可通过**状态与进度**查询当前加载情况（如 Pending、Loading、进度百分比）。

---

## 2. 端到端流程

1. **程序以多线程方式运行**：引擎支持多线程；主循环、渲染、逻辑、资源加载等可在不同线程执行；**Core** 可提供线程池或任务队列（可选），供各模块提交后台任务。
2. **资源加载在单独线程**：**Resource** 模块内部使用**专用加载线程**（或从 Core 线程池获取工作线程），所有实际 I/O 与解压/反序列化在该线程执行，不阻塞主线程或渲染线程。
3. **任意线程提交加载任务**：**任意线程**可调用 **requestLoadAsync**（或 **submitLoadTask**）；该 API **线程安全**，将请求加入加载队列后立即返回，返回 **LoadRequestId** 供后续查询状态与进度。
4. **回调**：加载完成后，在**约定线程**（如主线程或调用线程）调用 **LoadCompleteCallback**，传入资源句柄、LoadResult、user_data；与 US-resource-001 一致。
5. **状态提示**：在加载完成前，调用方可通过 **getLoadStatus(LoadRequestId)** 查询当前状态（Pending、Loading、Completed、Failed、Cancelled）；可通过 **getLoadProgress(LoadRequestId)** 查询进度（0～1 或百分比），便于 UI 显示进度条或日志。
6. 可选：**cancelLoad(LoadRequestId)** 取消未完成的请求；取消后回调仍会触发，result 为 Cancelled。

---

## 3. 涉及模块

| 模块 | 职责摘要 |
|------|----------|
| 001-Core | 线程池或任务队列（可选）；供 Resource 等模块提交后台任务，支撑「程序以多线程方式运行」 |
| 013-Resource | 专用加载线程（或使用 Core 线程池）；线程安全的 submit/requestLoadAsync；LoadRequestId；getLoadStatus、getLoadProgress；回调；可选 cancelLoad |

---

## 4. 每模块职责与 I/O

### 001-Core

- **职责**：提供**线程池**或**任务队列**（**submitTask**、**runOnWorker** 等），使各模块可将工作提交到后台线程执行；Resource 模块可使用该能力实现「加载在单独线程」，其它模块也可复用（如物理、动画等）。若 Resource 自管加载线程，Core 可不暴露通用线程池，仅保证线程相关基础（如原子、互斥）可用。
- **输入**：各模块调用 submitTask(callback, user_data) 或类似。
- **输出**：**ITaskQueue** 或 **IThreadPool**：submitTask、可选 waitForTask；或仅文档约定「Resource 内部使用专用加载线程」。

### 013-Resource

- **职责**：**requestLoadAsync**（或 **submitLoadTask**）**线程安全**，可从任意线程调用；返回 **LoadRequestId**；内部在**专用加载线程**执行实际加载；完成后在约定线程调用 **LoadCompleteCallback**；提供 **getLoadStatus(LoadRequestId)**、**getLoadProgress(LoadRequestId)** 供状态与进度查询；可选 **cancelLoad(LoadRequestId)**。
- **输入**：任意线程调用 requestLoadAsync(path, callback, user_data)；其它线程调用 getLoadStatus、getLoadProgress、cancelLoad。
- **输出**：LoadRequestId；getLoadStatus -> LoadStatus（Pending/Loading/Completed/Failed/Cancelled）；getLoadProgress -> float 或 LoadProgressInfo；cancelLoad；回调语义同 US-resource-001。

---

## 5. 派生接口（ABI 条目）

以下按 `docs/engine-abi-interface-generation-spec.md` 书写。

### 001-Core

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 001-Core | TenEngine::core | IThreadPool | 抽象接口 | 线程池 | TenEngine/core/ThreadPool.h | IThreadPool::submitTask | void submitTask(TaskCallback callback, void* user_data); 将任务提交到工作线程执行；线程安全，可从任意线程调用 |
| 001-Core | TenEngine::core | — | 回调类型 | 任务回调 | TenEngine/core/ThreadPool.h | TaskCallback | void (*)(void* user_data); 在工作线程中执行 |
| 001-Core | TenEngine::core | — | 自由函数/单例 | 获取全局线程池 | TenEngine/core/ThreadPool.h | getThreadPool | IThreadPool* getThreadPool(); 可选；Resource 等模块用于后台加载；调用方不拥有指针 |

（若 Resource 自管加载线程且不依赖 Core 线程池，本表可省略；仅保留「可从任意线程调用 Resource 的 requestLoadAsync」的约定。）

### 013-Resource

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 013-Resource | TenEngine::resource | IResourceManager | 抽象接口 | 异步加载（线程安全） | TenEngine/resource/ResourceManager.h | IResourceManager::requestLoadAsync | LoadRequestId requestLoadAsync(char const* path, LoadCompleteCallback on_done, void* user_data); **线程安全**，可从任意线程调用；加载在专用线程执行；返回请求 ID 供查询状态与进度 |
| 013-Resource | TenEngine::resource | IResourceManager | 抽象接口 | 查询加载状态 | TenEngine/resource/ResourceManager.h | IResourceManager::getLoadStatus | LoadStatus getLoadStatus(LoadRequestId id) const; 线程安全；返回 Pending/Loading/Completed/Failed/Cancelled |
| 013-Resource | TenEngine::resource | IResourceManager | 抽象接口 | 查询加载进度 | TenEngine/resource/ResourceManager.h | IResourceManager::getLoadProgress | float getLoadProgress(LoadRequestId id) const; 返回 0.f～1.f；线程安全；Completed/Failed/Cancelled 后语义由实现约定 |
| 013-Resource | TenEngine::resource | IResourceManager | 抽象接口 | 取消加载 | TenEngine/resource/ResourceManager.h | IResourceManager::cancelLoad | void cancelLoad(LoadRequestId id); 取消未完成的请求；完成后回调仍会触发，result 为 Cancelled |
| 013-Resource | TenEngine::resource | — | 类型别名/句柄 | 加载请求 ID | TenEngine/resource/ResourceManager.h | LoadRequestId | 不透明句柄，由 requestLoadAsync 返回；用于 getLoadStatus、getLoadProgress、cancelLoad |
| 013-Resource | TenEngine::resource | — | 枚举 | 加载状态 | TenEngine/resource/ResourceManager.h | LoadStatus | enum class LoadStatus { Pending, Loading, Completed, Failed, Cancelled }; |

（**LoadCompleteCallback**、**LoadResult**、**IResource** 同 US-resource-001；requestLoadAsync 签名增加返回值 LoadRequestId，并明确线程安全与专用加载线程。）

---

## 6. 参考（可选）

- **Unity**：Addressables 异步加载、Progress 与 Task；多线程下加载请求与回调线程约定。
- **Unreal**：FStreamableManager 异步加载、FStreamableHandle 与 GetProgress、CancelHandle；游戏线程与异步加载线程分离。
- **通用**：专用 I/O 线程 + 线程安全队列 + 回调在指定线程派发。

---

*本故事派生出的 ABI 条目将同步到 `specs/_contracts/001-core-ABI.md`（可选）、`specs/_contracts/013-resource-ABI.md`。与 US-resource-001 互补：001 为异步加载与回调，002 明确多线程、专用加载线程、状态与进度查询及可选取消。*
