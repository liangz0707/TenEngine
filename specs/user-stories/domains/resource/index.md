# resource 领域 · 用户故事索引

| 编号 | 标题 | 涉及模块 | 文档 |
|------|------|----------|------|
| US-resource-001 | 异步加载资源并在回调后继续操作 | 001-Core, 002-Object, 013-Resource | [US-resource-001-async-load-resource.md](./US-resource-001-async-load-resource.md) |
| US-resource-002 | 多线程运行，资源在单独线程加载，任意线程提交任务，回调和状态提示 | 001-Core, 013-Resource | [US-resource-002-multithreaded-load-with-status.md](./US-resource-002-multithreaded-load-with-status.md) |
| US-resource-003 | 所有资源使用统一加载接口（Mesh、Texture、Material、Model、Effect、Terrain 等） | 013-Resource, 002-Object | [US-resource-003-multi-type-loading.md](./US-resource-003-multi-type-loading.md) |
| US-resource-004 | 资源三态（FResource/RResource/DResource）；F 用 GUID 引用、R 用指针引用、D 保存在 R 内部；部分资源仅某一形态 | 013-Resource | [US-resource-004-resource-three-forms-f-r-d.md](./US-resource-004-resource-three-forms-f-r-d.md) |
| US-resource-005 | 资源卸载与引用计数/GC（显式卸载、依赖解除、GC 策略） | 013-Resource | [US-resource-005-unload-and-gc.md](./US-resource-005-unload-and-gc.md) |

*与顶层索引的关系：`specs/user-stories/000-user-stories-index.md` 仅做领域导航；各领域详情见本表。*
