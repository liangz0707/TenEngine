# Feature Specification: 012-Mesh 完整模块

**Feature Branch**: `012-mesh-full-module-001`  
**Created**: 2025-02-05  
**Status**: Draft  
**Input**: 本 feature 的完整模块规约见 `docs/module-specs/012-mesh.md`，契约见 `specs/_contracts/012-mesh-public-api.md`；**本 feature 实现完整模块内容**。

## 规约与契约引用 *(模块切片时必填)*

- **完整模块规约**：`docs/module-specs/012-mesh.md`（012-Mesh 网格数据与几何：顶点/索引缓冲、LOD、蒙皮、顶点格式；Mesh 为可加载资产，013 解析 .mesh 后交本模块 CreateMesh；EnsureDeviceResources 时由本模块调用 008-RHI 创建顶点/索引缓冲）。
- **本模块范围**（本 feature 实现完整模块内容）：
  1. **VertexIndex**：VertexFormat、IndexFormat、BufferLayout；与 RenderCore 格式映射。
  2. **Submesh**：SubmeshCount、GetSubmesh、MaterialSlot、DrawCall 批次。
  3. **LOD**：LODCount、SelectLOD、StreamingRequest；与 Resource 对接。
  4. **Skinning**：BoneIndices、Weights、BindPose；与 Animation 骨骼矩阵对接。
  5. **Mesh 生命周期**：CreateMesh(vertexData, indexData, layout, submeshes) 仅接受内存（入参由 013 传入）；EnsureDeviceResources(handle, device) 时对依赖链先 Ensure 再调用 008 CreateBuffer 创建顶点/索引缓冲；ReleaseMesh(handle)。
  6. **MeshAssetDesc**：归属 012；.mesh 为序列化/磁盘格式；formatVersion、vertexLayout、vertexData、indexData、indexFormat、submeshes、可选 LOD/蒙皮。

实现时只使用**本 feature 依赖的上游契约**（`specs/_contracts/001-core-public-api.md`、`008-rhi-public-api.md`、`009-rendercore-public-api.md`、`013-resource-public-api.md`）中已声明的类型与 API；不实现本规约未列出的能力。

- **ABI 与构建规约**（`.specify/memory/constitution.md` §VI）：本模块须实现其 **ABI 文件**（`specs/_contracts/012-mesh-ABI.md`）中的**全部**符号与能力；构建须通过**引入真实子模块源码**（如 CMake `add_subdirectory`）满足依赖，**禁止**使用 stub、mock 或与契约不一致的代替实现作为长期方案。**契约更新**：接口变更须在对应 **ABI 文件**中**增补或替换**对应的 ABI 条目；下游所需接口须在**上游模块的 ABI 文件**中以 **TODO** 登记。详见 `specs/_contracts/README.md`「契约更新流程」。

- **第三方依赖**：第三方库引入说明在契约 `specs/_contracts/012-mesh-public-api.md` 中声明；本 spec 引用该契约即可，不在 spec 中重复列出。详见 `docs/third_party-integration-workflow.md`。

## User Scenarios & Testing *(mandatory)*

### User Story 1 - 从内存数据创建网格 (Priority: P1)

013-Resource 解析 .mesh 得到顶点/索引与布局、子网格等内存数据后，调用 012 CreateMesh 创建网格句柄；渲染管线或地形等下游通过 MeshHandle 获取几何用于绘制。

**Why this priority**: 创建网格是模块的核心入口，无此则无法向 Pipeline/Terrain/Animation 提供网格数据。

**Independent Test**: 给定符合契约的顶点/索引内存与布局、子网格描述，调用 CreateMesh 得到有效 MeshHandle；可查询 SubmeshCount、GetSubmesh、顶点/索引元数据。

**Acceptance Scenarios**:

1. **Given** 013 已解析 .mesh 得到 vertexData、indexData、layout、submeshes，**When** 调用 CreateMesh(…)，**Then** 返回有效 MeshHandle，且 SubmeshCount、GetSubmesh 与入参一致。
2. **Given** 无效或空顶点/索引数据，**When** 调用 CreateMesh，**Then** 行为符合契约约束（错误码或空句柄，不崩溃）。

---

### User Story 2 - 确保设备侧顶点/索引缓冲 (Priority: P1)

下游在渲染前对 MeshHandle 调用 EnsureDeviceResources(handle, device)；012 在需要时调用 008-RHI 创建顶点/索引缓冲（DResource），并完成依赖链 Ensure。

**Why this priority**: 无设备缓冲则无法在 GPU 上绘制；与 RHI 的边界由契约约定。

**Independent Test**: 在 RHI/设备已初始化前提下，对已 CreateMesh 的 handle 调用 EnsureDeviceResources，可获取到可用于提交绘制的 VertexBufferHandle/IndexBufferHandle（或等价能力）。

**Acceptance Scenarios**:

1. **Given** 有效 MeshHandle 与已初始化 device，**When** 调用 EnsureDeviceResources(handle, device)，**Then** 顶点/索引缓冲已创建并可被 Pipeline/RenderCore 使用。
2. **Given** 同一 handle 多次调用 EnsureDeviceResources，**Then** 幂等，不重复创建或泄漏。

---

### User Story 3 - LOD 选择与流式请求 (Priority: P2)

运行时按距离或屏幕尺寸选择 LOD 级别；需要时通过 StreamingRequest 与 013-Resource 配合流式加载 LOD 数据。

**Why this priority**: 大世界与性能依赖 LOD；与 Resource 的对接由契约约定。

**Independent Test**: 对多 LOD 网格调用 SelectLOD(handle, distanceOrScreenSize) 得到当前应使用的 LODLevel；可发起 StreamingRequest 与 013 配合。

**Acceptance Scenarios**:

1. **Given** 多 LOD 网格与当前视距/屏幕占比，**When** SelectLOD(…)，**Then** 返回对应 LODLevel，且与契约定义的策略一致。
2. **Given** 需要流式加载的 LOD，**When** 发起 StreamingRequest，**Then** 与 013 的句柄与加载流程对接正确。

---

### User Story 4 - 子网格与 DrawCall 批次 (Priority: P2)

Pipeline 或渲染逻辑通过 GetSubmesh、MaterialSlot、SubmeshDesc 获取每段几何的绘制范围与材质槽位，用于生成 DrawCall 批次。

**Why this priority**: 多子网格/多材质是常见需求，子网格划分与批次由本模块与契约定义。

**Independent Test**: 对 MeshHandle 迭代 SubmeshCount、GetSubmesh(i)，得到 SubmeshDesc（偏移、数量、MaterialSlot）；可与 RenderCore/Pipeline 的顶点/索引绑定一致。

**Acceptance Scenarios**:

1. **Given** 含多子网格的 MeshHandle，**When** GetSubmesh(i)，**Then** 返回的 SubmeshDesc 与 CreateMesh 时传入的 submeshes 一致，且可用于生成正确 DrawCall。
2. **Given** 无效索引 i >= SubmeshCount，**When** GetSubmesh(i)，**Then** 行为符合契约（断言或明确错误，不越界）。

---

### User Story 5 - 蒙皮数据与 Animation 对接 (Priority: P3)

蒙皮网格提供 BoneIndices、Weights、BindPose；与 015-Animation 的骨骼矩阵对接，用于 GPU 蒙皮或软件蒙皮。

**Why this priority**: 角色/骨骼动画依赖蒙皮数据；与 Animation 的约定由契约统一。

**Independent Test**: 对带蒙皮的 MeshHandle 可获取 SkinningData（骨骼索引与权重、BindPose）；命名/索引与 Animation 骨骼约定一致时可正确驱动蒙皮。

**Acceptance Scenarios**:

1. **Given** 含蒙皮数据的 CreateMesh 入参，**When** 创建并查询 SkinningData，**Then** BoneIndices、Weights、BindPose 可用且与契约一致。
2. **Given** 无蒙皮网格，**When** 查询 SkinningData，**Then** 返回空或“无蒙皮”表示，不崩溃。

---

### Edge Cases

- 顶点/索引格式与 RenderCore、RHI 不一致时：行为按契约与 ABI 约定（验证失败或明确错误）。
- EnsureDeviceResources 在 RHI 未初始化或 device 无效时：返回错误或空，不崩溃。
- LOD 流式未就绪时：SelectLOD 可返回当前已加载的最高 LOD 或占位；StreamingRequest 异步完成后再更新。
- 释放顺序：ReleaseMesh 后不再使用 handle；依赖的 008 缓冲由本模块在 ReleaseMesh 内或按契约约定释放。

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: 模块 MUST 提供 CreateMesh(vertexData, indexData, layout, submeshes) 且仅接受内存数据，入参由 013 传入；返回 MeshHandle。
- **FR-002**: 模块 MUST 提供 EnsureDeviceResources(handle, device)；在需要时调用 008-RHI 创建顶点/索引缓冲（DResource），对依赖链先 Ensure 再创建。
- **FR-003**: 模块 MUST 提供 ReleaseMesh(handle)，释放网格及本模块负责的 008 缓冲。
- **FR-004**: 模块 MUST 提供 VertexIndex 能力：VertexFormat、IndexFormat、BufferLayout；与 RenderCore 格式映射一致。
- **FR-005**: 模块 MUST 提供 Submesh 能力：SubmeshCount、GetSubmesh、MaterialSlot、DrawCall 批次信息（SubmeshDesc）。
- **FR-006**: 模块 MUST 提供 LOD 能力：LODCount、SelectLOD、StreamingRequest；与 013-Resource 对接。
- **FR-007**: 模块 MUST 提供 Skinning 能力：BoneIndices、Weights、BindPose；与 015-Animation 骨骼矩阵对接的约定一致。
- **FR-008**: MeshAssetDesc 归属 012；.mesh 格式与 002 注册；CreateMesh 入参与 MeshAssetDesc 对应。
- **FR-009**: 实现仅使用 001-Core、008-RHI、009-RenderCore、013-Resource 各契约中已声明的类型与 API。

### Key Entities

- **MeshHandle**：网格句柄；创建后直至显式 ReleaseMesh；持有顶点/索引 DResource 引用及子网格、LOD、蒙皮元数据。
- **SubmeshDesc**：子网格划分、材质槽位、DrawCall 批次范围。
- **LODHandle / LODLevel**：LOD 级别、SelectLOD、StreamingRequest 与 Resource 对接。
- **SkinningData**：骨骼索引与权重、BindPose；与 Animation 骨骼矩阵对接。
- **MeshAssetDesc**：网格描述（formatVersion、顶点/索引布局、子网格、可选 LOD/蒙皮）；.mesh 为其序列化格式；013 解析后交 012 CreateMesh。

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: 在 Core、RHI、RenderCore、Resource 已初始化前提下，从内存数据创建网格并在设备侧完成缓冲创建，可用于单次绘制，成功率 100%（符合契约的合法输入）。
- **SC-002**: 多子网格网格的 GetSubmesh 与 CreateMesh 入参一致，Pipeline 可据此生成正确 DrawCall 批次。
- **SC-003**: 多 LOD 网格的 SelectLOD 与契约定义的策略一致；StreamingRequest 与 013 对接无数据错位或泄漏。
- **SC-004**: 蒙皮网格的 SkinningData 与 Animation 骨骼命名/索引约定一致时，可正确驱动蒙皮渲染或计算。
- **SC-005**: 实现通过 012-mesh-ABI 中声明的全部符号与能力；构建通过引入真实子模块源码满足依赖，无长期 stub/mock。

## Interface Contracts *(multi-agent sync)*

- **本模块实现的契约**：`specs/_contracts/012-mesh-public-api.md`
- **本模块依赖的契约**：见下方 Dependencies；实现时只使用各契约中声明的类型与接口。
- **ABI/构建**：须实现 `specs/_contracts/012-mesh-ABI.md` 中全部符号；构建须引入真实子模块代码，禁止长期使用 stub 或代替方案。接口变更须在 ABI 文件中更新；下游所需接口须在上游 ABI 中以 TODO 登记（constitution §VI、`specs/_contracts/README.md`）。

## Dependencies

- **001-Core**：`specs/_contracts/001-core-public-api.md`（内存、容器、平台等）
- **008-RHI**：`specs/_contracts/008-rhi-public-api.md`（EnsureDeviceResources 时由 012 调用 008 创建顶点/索引缓冲 DResource）
- **009-RenderCore**：`specs/_contracts/009-rendercore-public-api.md`（顶点格式、缓冲描述、格式映射）
- **013-Resource**：`specs/_contracts/013-resource-public-api.md`（013 加载并解析 .mesh 后交 012 CreateMesh；LOD 流式通过句柄；Mesh 为可加载资产）

依赖关系总览见 `specs/_contracts/000-module-dependency-map.md`。
