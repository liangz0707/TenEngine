# 012-Mesh 模块 ABI

- **契约**：[012-mesh-public-api.md](./012-mesh-public-api.md)（能力与类型描述）
- **本文件**：012-Mesh 对外 ABI 显式表。

## ABI 表

列定义：**模块名 | 命名空间 | 类名 | 接口说明 | 头文件 | 符号 | 说明**

| 模块名 | 命名空间 | 类名 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|--------|------|------|
| （待补充） | 见本模块契约 | — | — | — | — | 由本模块契约与实现填入 |

---

## 数据相关 TODO

（依据 [docs/assets/013-resource-data-model.md](../../docs/assets/013-resource-data-model.md)、[resource-loading-flow.md](../../docs/assets/resource-loading-flow.md)。）

- [ ] **.mesh 二进制格式**：与 013 约定 formatVersion、debugDescription、vertexLayout、vertexData、indexData、indexFormat、submeshes（及可选 LOD、蒙皮）；013 解析 .mesh 后交本模块，本模块不直接读文件。
- [ ] **CreateMesh(vertexData, indexData, layout, submeshes)**：013 在 Load 或按需阶段调用；内部调 **008-RHI CreateBuffer**（Usage=Vertex/Index）创建顶点/索引缓冲（**DResource**），组装 **MeshHandle**（内持 DResource）返回 013。
- [ ] **MeshHandle**：支持“先有 MeshHandle、后填 DResource”（按需阶段）；013 调用 EnsureDeviceResourcesAsync 时，012 创建 DResource 并填入已有 MeshHandle。
- [ ] **Unload**：013 Unload(IMeshResource*) 时通知 012 释放对应 MeshHandle；012 释放顶点/索引缓冲（008 资源），与 013 协调。
