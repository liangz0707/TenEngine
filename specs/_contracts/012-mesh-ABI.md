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

（依据 [docs/assets/013-resource-data-model.md](../../docs/assets/013-resource-data-model.md) §Mesh；本模块上游：001-Core、009-RenderCore。）

### 数据

- [ ] **Mesh 输入格式约定**：与 013 约定 .mesh 解析后的结构（formatVersion、vertexLayout、vertexData、indexData、indexFormat、submeshes、可选 LOD/蒙皮）
- [ ] **MeshHandle**：内持顶点/索引 IBuffer*（DResource）、子网格信息

### 需提供的对外接口

| 接口 | 说明 |
|------|------|
| [ ] `CreateMesh(vertexData, indexData, layout, submeshes, device) → MeshHandle*` | 创建 Mesh；device 由调用方（013）传入 |
| [ ] `EnsureDeviceResources(handle, device) → bool` | 按需创建 DResource；返回是否就绪 |
| [ ] `ReleaseMesh(handle)` | 释放 MeshHandle 及 GPU 缓冲 |

### 需调用上游

| 场景 | 调用 009 / 设备接口 |
|------|---------------------|
| CreateMesh | 使用 009.VertexFormat、IndexFormat、BufferDesc 描述；调用设备 CreateBuffer(Vertex/Index) |
| ReleaseMesh | 调用设备 DestroyBuffer |

### 调用流程

1. 013 解析 .mesh → 得到 vertexData、indexData、layout、submeshes → 012.CreateMesh(..., device)
2. 013 EnsureDeviceResourcesAsync → 012.EnsureDeviceResources(handle, device)
3. 013 Unload → 012.ReleaseMesh(handle)
