# US-resource-001：异步加载资源并在回调后继续操作

- **标题**：异步加载资源并在回调后继续操作
- **编号**：US-resource-001（原 US-003）

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

（同原 US-003，略。）

---

## 5. 派生接口（ABI 条目）

（见原 US-003 文档；已同步到 `specs/_contracts/013-resource-ABI.md`。）

---

## 6. 参考（可选）

- Unity：Addressables.LoadAssetAsync、ResourceRequest。
- Unreal：FStreamableManager、RequestAsyncLoad。

*ABI 条目已同步到 `specs/_contracts/013-resource-ABI.md`。*
