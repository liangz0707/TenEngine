# Research: 012-Mesh 完整模块

**Branch**: `012-mesh-full-module-001` | **Date**: 2025-02-05

## 1. 根据 asset 与 013 补充的接口与功能

### 1.1 决策：012 与 013 的 Load/Loader/Deserialize/Save 边界

- **Decision**: 012 实现 **IResourceLoader**（Mesh 类型）与 **IDeserializer**（Mesh）；013 读 .mesh 后先调 012 的 Deserialize 得到 opaque payload（MeshAssetDesc*），再调 012 的 Loader CreateFromPayload(Mesh, payload, manager) 得到 IResource* 并缓存。
- **Rationale**: 013 契约明确「Loader 由拥有 *Desc 的模块实现」「*Desc 对 013 不可见，通过 payload 传递」；asset 文档明确 MeshAssetDesc 归属 012、.mesh 为 012 的序列化格式。
- **Alternatives considered**: 013 自行反序列化 .mesh 再调 012 CreateMesh(desc)——违反「013 不解析 *Desc」的契约约定。

### 1.2 决策：MeshAssetDesc 字段与 .mesh 格式

- **Decision**: MeshAssetDesc 包含：formatVersion（uint32）、debugDescription（string/UTF-8）、vertexLayout（与 009 顶点格式一致）、vertexData（原始指针或 span）、indexFormat（与 009 IndexFormat 一致）、indexData、submeshes（SubmeshDesc 数组）、可选 LOD 信息、可选蒙皮数据（BoneIndices、Weights、BindPose）；.mesh 为上述结构的序列化形式，与 002 注册。
- **Rationale**: 与 `docs/assets/013-resource-data-model.md` §Mesh 资源形态、`docs/asset/01-asset-data-structure.md` 一目录一资源（.mesh + .meshdata）一致。
- **Alternatives considered**: 将 vertexData/indexData 单独放在 .meshdata 文件——采用同目录 .mesh（描述）+ .meshdata（大块数据）可接受，由 013 读入后一并传入 payload 或 CreateMesh。

### 1.3 决策：EnsureDeviceResources 与 DResource 创建顺序

- **Decision**: 012 的 EnsureDeviceResources(MeshHandle, IDevice*) 内部：对依赖链先 Ensure（若 Mesh 依赖其他资源，经 013 先 Ensure 依赖），再调用 008-RHI CreateBuffer 创建顶点/索引缓冲并填入 MeshHandle 内部槽位；幂等，重复调用不重复创建。
- **Rationale**: asset 02-asset-loading-flow §2.8、03-asset-misc §3.3.3 规定「依赖链上的资源先完成 DResource 创建」。
- **Alternatives considered**: 012 不管理依赖链、由 013 统一 Ensure——013 契约规定 Ensure 由 IResource 子类实现并转发，依赖顺序由实现体（012）保证更清晰。

### 1.4 决策：Save 时 012 的职责

- **Decision**: 013 在 Save(IResource*, path) 时根据 GetResourceType() 分发到各模块；012 提供 **SerializeMeshToBuffer(MeshHandle, buffer, size)** 或等价接口，从 IMeshResource 背后 MeshHandle 产出可写盘的内存内容（序列化后的 .mesh 布局），013 负责将 buffer 写入 path。
- **Rationale**: 013 契约「各模块从 IResource 产出可存盘的内存内容，返回给 013；013 再调用统一保存接口将内容写入磁盘」。
- **Alternatives considered**: 012 直接写文件——违反「各模块不直接写文件」的契约。

### 1.5 决策：IMeshResource 与 012 的 RResource 实现

- **Decision**: 012 提供的 Loader 返回的 IResource* 为 012 实现的 **Mesh RResource**（实现 IResource 与 IMeshResource）；该对象内部持有 MeshHandle，EnsureDeviceResources/Release 委托 012 的 EnsureDeviceResources/ReleaseMesh；013 仅缓存该 IResource*，不包装一层。
- **Rationale**: 013 ABI 中 IMeshResource 为「网格资源视图」，由 RequestLoadAsync(..., Mesh, ...) 返回；契约「Loader 据此创建 IResource 实现体（RResource），返回 IResource*」。
- **Alternatives considered**: 013 持有 MeshRResource 包装 012 MeshHandle——若 013 实现为「013 的 MeshRResource 内持 MeshHandle、委托 012 API」，则 012 仅需提供 CreateMesh/Ensure/Release 与查询 API，不实现 IResource；当前 research 采用「012 实现 RResource」以最小化 013 对 012 内部类型的依赖。

### 1.6 一目录一资源与 002 注册

- **Decision**: Mesh 资源磁盘形态：一目录一资源，目录内 .mesh（MeshAssetDesc 序列化）+ .meshdata（可选，顶点/索引大块）+ 可选 .obj/.fbx；012 向 002 注册 MeshAssetDesc 类型及 .mesh 扩展名，用于反序列化与引用解析。
- **Rationale**: `docs/asset/01-asset-data-structure.md` §1.5、§1.7；013 反序列化时按 002 注册与类型分发。

## 2. 技术选型与最佳实践

- **顶点/索引格式**: 使用 009-RenderCore 的 VertexFormat、IndexFormat、BufferDesc；与 008 CreateBuffer 参数一致，避免 012 自定义格式导致与 Pipeline 不一致。
- **LOD 与流式**: SelectLOD 策略（距离/屏幕尺寸）由 012 契约约定；StreamingRequest 与 013 的 RequestStreaming、StreamingHandle 对接，013 负责队列与优先级，012 提供 LOD 级别与数据请求接口。
- **蒙皮与 015**: SkinningData 的骨骼索引/名称与 015-Animation 契约约定一致；012 不解析动画，仅提供数据视图。

## 3. 风险与缓解

- **风险**: 008/009 的 Buffer/VertexFormat 具体签名尚未在 ABI 中完全固定。  
  **缓解**: 实现时仅使用 008/009 public-api 与 ABI 已声明符号；若缺项则在上游 ABI 以 TODO 登记。
- **风险**: 013 的 RegisterDeserializer/RegisterResourceLoader 调用时机与 012 初始化顺序。  
  **缓解**: 012 在模块初始化或首次使用前向 013 注册；依赖 013 的 GetResourceManager() 或注入的 IResourceManager*。
