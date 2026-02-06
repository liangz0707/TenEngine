# Implementation Plan: 012-Mesh 完整模块

**Branch**: `012-mesh-full-module-001` | **Date**: 2025-02-05 | **Spec**: [spec.md](./spec.md)  
**Input**: Feature specification from `specs/012-mesh-full-module-001/spec.md`。根据 **asset 文档**（`docs/asset/`）与 **013-Resource** 契约补充本模块需完成的接口与功能。

**Note**: 本 plan 根据 `docs/asset/01-asset-data-structure.md`、`02-asset-loading-flow.md`、`03-asset-misc.md` 及 `specs/_contracts/013-resource-public-api.md`、013-ABI 中 Load/Loader/Deserialize/Save 约定，补充 012 与 013 的对接接口（Loader、Deserializer、Save 产出、MeshAssetDesc、一目录一资源）。

## Summary

- **主要需求**：实现 012-Mesh 完整模块，提供网格数据与几何（顶点/索引缓冲、LOD、蒙皮、顶点格式）；Mesh 为可加载资产，013 解析 .mesh 后通过 012 的 Loader/CreateFromPayload 创建 RResource；EnsureDeviceResources 时由 012 调用 008-RHI 创建顶点/索引缓冲。
- **技术路线**：遵循契约与 asset 设计：012 拥有 MeshAssetDesc、.mesh 格式；向 013 注册 IResourceLoader、IDeserializer；实现 CreateMesh、EnsureDeviceResources、ReleaseMesh 及子网格/LOD/蒙皮查询；仅使用 001/008/009/013 契约已声明类型与 API。

## 实现范围（TenEngine：实现全量 ABI 内容）

- 本 feature 实现 **012-mesh-public-api** 与 **012-mesh-ABI** 中全部能力；并依 **asset + 013** 补充：**MeshAssetDesc** 归属与字段、**IResourceLoader::CreateFromPayload**（Mesh）、**IDeserializer**（Mesh）、**Save 时产出内存内容**、**一目录一资源**（.mesh + .meshdata）与 002 注册。
- 全量 ABI 见下方「全量 ABI 内容（实现参考）」；契约更新小节仅含相对现有 ABI 的**新增/修改**（当前 012-mesh-ABI 为「待补充」，故全量即新增）。

### 全量 ABI 内容（实现参考）

列定义：**模块名 | 命名空间 | 类名 | 接口说明 | 头文件 | 符号 | 说明**

| 模块名 | 命名空间 | 类名 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|--------|------|------|
| 012-Mesh | te::mesh | — | 网格句柄 | te/mesh/Mesh.h | MeshHandle | 不透明句柄；CreateMesh 返回，ReleaseMesh 释放；内部持顶点/索引 DResource 槽位及子网格、LOD、蒙皮元数据 |
| 012-Mesh | te::mesh | — | 网格描述（归属 012） | te/mesh/MeshAssetDesc.h | MeshAssetDesc | formatVersion, debugDescription, vertexLayout, vertexData, indexData, indexFormat, submeshes, 可选 LOD/蒙皮；.mesh 为其序列化格式 |
| 012-Mesh | te::mesh | — | 子网格描述 | te/mesh/Mesh.h | SubmeshDesc | offset, count, materialSlotIndex；DrawCall 批次 |
| 012-Mesh | te::mesh | — | LOD 级别/句柄 | te/mesh/Mesh.h | LODLevel / LODHandle | LOD 级别、与 Resource 流式对接 |
| 012-Mesh | te::mesh | — | 蒙皮数据 | te/mesh/Mesh.h | SkinningData | BoneIndices, Weights, BindPose；与 015-Animation 对接 |
| 012-Mesh | te::mesh | — | 创建网格（仅内存） | te/mesh/MeshFactory.h | CreateMesh | `MeshHandle CreateMesh(MeshAssetDesc const* desc);` 入参由 013 传入；013 Loader 内调用 |
| 012-Mesh | te::mesh | — | 确保设备缓冲 | te/mesh/MeshDevice.h | EnsureDeviceResources | `bool EnsureDeviceResources(MeshHandle h, IDevice* device);` 对依赖链先 Ensure 再调 008 CreateBuffer |
| 012-Mesh | te::mesh | — | 释放网格 | te/mesh/MeshFactory.h | ReleaseMesh | `void ReleaseMesh(MeshHandle h);` 释放顶点/索引 DResource 与句柄 |
| 012-Mesh | te::mesh | — | 子网格数量 | te/mesh/Mesh.h | GetSubmeshCount | `uint32_t GetSubmeshCount(MeshHandle h);` |
| 012-Mesh | te::mesh | — | 取子网格 | te/mesh/Mesh.h | GetSubmesh | `SubmeshDesc const* GetSubmesh(MeshHandle h, uint32_t index);` |
| 012-Mesh | te::mesh | — | LOD 数量 | te/mesh/Mesh.h | GetLODCount | `uint32_t GetLODCount(MeshHandle h);` |
| 012-Mesh | te::mesh | — | 选择 LOD | te/mesh/Mesh.h | SelectLOD | `uint32_t SelectLOD(MeshHandle h, float distanceOrScreenSize);` 与契约策略一致 |
| 012-Mesh | te::mesh | — | 流式请求 | te/mesh/Mesh.h | RequestStreaming | 与 013 RequestStreaming/StreamingHandle 对接（可选） |
| 012-Mesh | te::mesh | — | 蒙皮数据 | te/mesh/Mesh.h | GetSkinningData | `SkinningData const* GetSkinningData(MeshHandle h);` 无蒙皮返回 nullptr |
| 012-Mesh | te::mesh | — | 顶点/索引缓冲句柄 | te/mesh/MeshDevice.h | GetVertexBufferHandle, GetIndexBufferHandle | EnsureDeviceResources 后可用；类型与 008 契约一致 |
| 012-Mesh | te::mesh | IResourceLoader | Mesh 类型 Loader | te/mesh/MeshLoader.h | MeshResourceLoader::CreateFromPayload | `IResource* CreateFromPayload(ResourceType type, void* payload, IResourceManager* manager);` type==Mesh 时将 payload 解释为 MeshAssetDesc*，CreateMesh 后包装为 IResource* 返回 |
| 012-Mesh | te::mesh | IDeserializer | Mesh 反序列化 | te/mesh/MeshDeserializer.h | MeshDeserializer::Deserialize | `void* Deserialize(void const* buffer, size_t size);` 产出 MeshAssetDesc*（payload），013 不解析 |
| 012-Mesh | te::mesh | — | Save 时产出内存 | te/mesh/MeshSerialize.h | SerializeMeshToBuffer | `bool SerializeMeshToBuffer(MeshHandle h, void* buffer, size_t* size);` 或等价；013 Save 时按类型调用，012 产出可写盘内容 |
| 012-Mesh | te::mesh | — | 与 002 注册描述类型 | te/mesh/MeshAssetDesc.h | — | MeshAssetDesc 与 .mesh 格式向 002 注册；一目录一资源（.mesh + .meshdata + 可选 .obj/.fbx） |

*VertexFormat、IndexFormat、BufferLayout 使用 009-RenderCore 契约类型；CreateBuffer/DestroyBuffer 使用 008-RHI 契约。*

## Technical Context

**Language/Version**: C++17 或更高（与 constitution 一致）  
**Primary Dependencies**: 001-Core（内存、容器）、008-RHI（CreateBuffer、DestroyBuffer、IDevice）、009-RenderCore（VertexFormat、IndexFormat、BufferDesc）、013-Resource（IResource、IResourceManager、IResourceLoader、IDeserializer、ResourceType）  
**Storage**: 网格数据仅内存与 GPU 缓冲；.mesh/.meshdata 由 013 读/写，012 提供 Deserialize/SerializeMeshToBuffer  
**Testing**: 单元测试（CreateMesh/ReleaseMesh、GetSubmesh、SelectLOD）；集成测试（与 013 Load + EnsureDeviceResources 对接）  
**Target Platform**: Windows/Linux/macOS，与 008/009 目标一致  

**Performance Goals**: CreateMesh/EnsureDeviceResources 符合帧预算；大网格 LOD 选择与流式无卡顿。  
**Constraints**: 仅使用上游契约已声明类型与 API；蒙皮与 015-Animation 骨骼名称/索引约定一致。  
**Scale/Scope**: 单模块 012，与 013/008/009 边界清晰。

## 依赖引入方式（TenEngine 构建规约）

| 依赖模块 | 引入方式 | 说明 |
|----------|----------|------|
| 001-core | **源码** | 通过 TenEngineHelpers 引入上游源码构建。 |
| 008-rhi | **源码** | 同上；EnsureDeviceResources 时调用 CreateBuffer/DestroyBuffer。 |
| 009-rendercore | **源码** | 同上；VertexFormat、IndexFormat、BufferDesc。 |
| 013-resource | **源码** | 同上；IResource、IResourceManager、IResourceLoader、IDeserializer、RegisterResourceLoader、RegisterDeserializer。 |

### 第三方依赖（本 feature 涉及模块所需）

| 第三方 ID | 引入方式 | 文档 | 说明 |
|-----------|----------|------|------|
| 本 feature 无第三方依赖 | — | — | 运行时 Load 仅读 .mesh 引擎格式；Import 阶段由 013/工具链使用外部库（如 assimp）产出 .mesh，012 不直接依赖。 |

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

| 原则 | 状态 | 说明 |
|------|------|------|
| §VI 契约与 ABI 对齐 | 通过 | 对外 API 以 012-mesh-public-api 与上述全量 ABI 为准；无未声明暴露。 |
| §VI 实现仅用契约类型与 API | 通过 | 仅使用 001/008/009/013 契约中已声明类型与接口。 |
| §VI 全量 ABI 实现 | 通过 | 实现上表全部符号与能力；无长期 stub。 |
| §VI 构建引入真实子模块 | 通过 | 依赖以源码 add_subdirectory/FetchContent 引入，禁止 stub 代替上游。 |
| §VI 契约更新 ABI 先行 | 通过 | 接口变更在 012-mesh-ABI 中增补；下游所需在上游 ABI 以 TODO 登记。 |

## Project Structure

### Documentation (this feature)

```text
specs/012-mesh-full-module-001/
├── plan.md
├── research.md
├── data-model.md
├── quickstart.md
├── contracts/
└── tasks.md          # /speckit.tasks 产出
```

### Source Code (repository root)

```text
include/te/mesh/
├── Mesh.h             # MeshHandle, SubmeshDesc, LODLevel, SkinningData
├── MeshAssetDesc.h    # MeshAssetDesc
├── MeshFactory.h      # CreateMesh, ReleaseMesh
├── MeshDevice.h       # EnsureDeviceResources, GetVertexBufferHandle, GetIndexBufferHandle
├── MeshLoader.h       # MeshResourceLoader (IResourceLoader)
├── MeshDeserializer.h # MeshDeserializer (IDeserializer)
└── MeshSerialize.h   # SerializeMeshToBuffer

src/
├── mesh/
│   ├── MeshFactory.cpp
│   ├── MeshDevice.cpp
│   ├── MeshLoader.cpp
│   ├── MeshDeserializer.cpp
│   └── MeshSerialize.cpp
└── ...

tests/
├── unit/              # CreateMesh, GetSubmesh, SelectLOD, ReleaseMesh
├── integration/       # 与 013 Load + EnsureDeviceResources 对接
└── contract/          # 契约行为测试
```

**Structure Decision**: 单模块库；公开 API 与契约一致，实现置于 src/mesh。

## 契约更新（TenEngine，仅新增/修改部分 - 用于写回）

当前 `specs/_contracts/012-mesh-ABI.md` 为「待补充」，故**全量 ABI 表**即新增内容。写回时将 plan 中「全量 ABI 内容（实现参考）」表整体写入 012-mesh-ABI.md 的 ABI 表，并清除 public-api 中已实现的 TODO、增加变更记录。

| 操作 | 模块名 | 命名空间 | 类名 | 接口说明 | 头文件 | 符号 | 说明 |
|------|--------|----------|------|----------|--------|------|------|
| 新增 | 012-Mesh | te::mesh | — | MeshHandle, MeshAssetDesc, SubmeshDesc, LODLevel, SkinningData | 见全量表 | 见全量 ABI 表 | 同上 |
| 新增 | 012-Mesh | te::mesh | — | CreateMesh, ReleaseMesh, EnsureDeviceResources, GetSubmeshCount, GetSubmesh, GetLODCount, SelectLOD, GetSkinningData, GetVertexBufferHandle, GetIndexBufferHandle | 见全量表 | 同上 | 同上 |
| 新增 | 012-Mesh | te::mesh | IResourceLoader | MeshResourceLoader::CreateFromPayload | te/mesh/MeshLoader.h | CreateFromPayload | 013 调用，payload=MeshAssetDesc* |
| 新增 | 012-Mesh | te::mesh | IDeserializer | MeshDeserializer::Deserialize | te/mesh/MeshDeserializer.h | Deserialize | 013 反序列化 .mesh 得 payload |
| 新增 | 012-Mesh | te::mesh | — | SerializeMeshToBuffer | te/mesh/MeshSerialize.h | SerializeMeshToBuffer | Save 时 013 按类型调用 |

## Complexity Tracking

（无违规需豁免。）
