# 3. 资源设计其他说明

本文档补充引擎资源设计的**其他说明**：统一 IResource 接口（加载、卸载、导入）、状态检查与回调、层级引用、内部统一接口创建 GPU 资源、与 `000-module-dependency-map` 的对应关系及约定。

---

## 3.1 设计原则回顾（五项）

1. **所有资源依 GUID 进行管理**（见 [01-asset-data-structure](./01-asset-data-structure.md)）。
2. **所有资源继承自 IResource**，需要统一的**加载、卸载、导入**接口。
3. **磁盘资源结构**：一个目录存储一个资源——描述（.material / .mesh / .texture / .model / .level 等）、实际数据（.texdata / .meshdata 等，如需要）、导入前源数据（.png / .obj 等）。
4. **内存结构**：所有资源通过 IResource 统一加载；支持**异步、同步加载**，**状态检查**，**回调**，**层级引用**（见 [02-asset-loading-flow](./02-asset-loading-flow.md)）。
5. **内部有统一接口创建对应的 GPU 资源**（见下文 §3.3）。

---

## 3.2 统一接口（IResource）

### 3.2.1 加载 / 卸载 / 导入

| 能力 | 说明 |
|------|------|
| **Load** | LoadSync、LoadAsync；013 为唯一入口；依赖由 013 主动递归加载。 |
| **Unload** | Release、GC、UnloadPolicy；与各模块句柄协调。 |
| **Import** | RegisterImporter、DetectFormat、Convert、Metadata、Dependencies；导入器注册与依赖记录。 |

所有可加载资产类型（IMaterialResource、IMeshResource、ITextureResource、IModelResource、IShaderResource、ILevelResource 等）均继承 IResource；上述操作针对 **IResource*** 或对 ResourceId/句柄解析到 IResource。013 用同一套逻辑管理，**不按资源类型分叉**。

### 3.2.2 状态检查与回调

- **同步**：LoadSync 返回时资源已就绪（含依赖）。
- **异步**：LoadAsync 返回 LoadHandle/AsyncResult；支持 **IsCompleted**、**GetResult**、**完成回调**、**Cancel**；ResolveDependencies、Queue、Priority 由 013 内部管理。
- 下游不长期持有 IResource*，仅持 **ResourceId** 或**不透明句柄**；通过 013 的 GetCached/LoadSync 解析。

### 3.2.3 层级引用

- **FResource** 间仅 **GUID** 引用；**RResource** 间为**指针**引用，由 013 在依赖加载完成后组装。
- 典型层级：Level → Model → Mesh + Material；Material → Shader + Texture。013 保证 Load 完成（含异步回调）时，返回的句柄对应的 RResource 及其依赖均已就绪。

---

## 3.3 内部统一接口创建 GPU 资源（DResource）

### 3.3.1 统一入口

- **IResource** 暴露 **EnsureDeviceResources** / **EnsureDeviceResourcesAsync**。
- 下游（如 020-Pipeline）在提交绘制前对资源句柄/ResourceId 调用上述接口；013 将调用**转发**给具体 RResource，013 自身不创建 GPU 对象。

### 3.3.2 实际创建方（对齐 000-module-dependency-map）

| 资源类型 | 创建 DResource 的模块 | 说明 |
|----------|------------------------|------|
| GPU 纹理 | **028-Texture**（调用 008-RHI） | EnsureDeviceResources 时由 028 调用 008 CreateTexture；013 加载后交 028 CreateTexture（内存），028 负责 DResource。 |
| 顶点/索引缓冲 | **012-Mesh**（调用 008-RHI） | EnsureDeviceResources 时由 012 调用 008 CreateBuffer 等。 |
| 材质相关绑定、采样器等 | **011-Material**（结合 008-RHI） | 材质绑定贴图、Uniform 等时，由 011 在 EnsureDeviceResources 时调用 008。 |
| PSO、ShaderModule | **008-RHI**（字节码由 010 提供） | 010 产出字节码，008 创建 ShaderModule/PSO；时机通常在 EnsureDeviceResources 或 Pipeline 首次绑定时。 |

- **013 不创建、不持有、不调用 008**；DResource 槽位在 **RResource 内部**，由 **028/011/012/008** 在 **EnsureDeviceResources** 时填充。
- 各模块通过 **008-RHI** 的公开接口（CreateBuffer、CreateTexture、CreateSampler、CreateGraphicsPSO 等）创建 GPU 资源，后端（Vulkan/D3D12/Metal 等）差异由 008 屏蔽。

### 3.3.3 层级与依赖

- 若 RResource 依赖其它 RResource（如 Material 依赖 Texture），EnsureDeviceResources 需保证**依赖链上的资源先完成** DResource 创建，再创建当前资源的 DResource，以便绑定视图、描述符等。

---

## 3.4 与 000-module-dependency-map 的对应关系

| 模块 | 在资源设计中的角色 |
|------|---------------------|
| **013-Resource** | 唯一加载入口；依赖 Core、Object、**028-Texture**；调用 010/011/012/**028**/**029** 的 Create*/Loader；不创建 DResource。 |
| **028-Texture** | 013 调用 **028 CreateTexture**；028 在 EnsureDeviceResources 时创建 RHI 纹理；011/020/021/022/023/024 使用贴图。 |
| **029-World** | Level 加载时使用 013；029 调 004 CreateSceneFromDesc；020/024 可选依赖 029 获取 Level 句柄/SceneRef。 |
| **010-Shader** | 013 加载 Shader 后调用 010 CreateShader/Compile。 |
| **011-Material** | 013 加载材质后调用 011 CreateMaterial；011 依赖 028（贴图引用）。 |
| **012-Mesh** | 013 加载网格后调用 012 CreateMesh；012 在 EnsureDeviceResources 时创建顶点/索引缓冲。 |
| **004-Scene** | 不负责资源与关卡描述，只负责场景结构、场景管理；029 调 004 CreateSceneFromDesc，004 仅根据传入描述初始化场景图；004 不依赖 013，不持有 IResource*。 |
| **008-RHI** | 028/011/012 在 EnsureDeviceResources 时调用 008 创建 DResource。 |

---

## 3.5 契约与规格索引

- **013-Resource**：`specs/_contracts/013-resource-public-api.md`、`docs/module-specs/013-resource.md`
- **028-Texture**：`specs/_contracts/028-texture-public-api.md`
- **029-World**：`specs/_contracts/029-world-public-api.md`
- **模块依赖图**：`specs/_contracts/000-module-dependency-map.md`
- **002-Object**：序列化、GUID/引用解析、*Desc 类型注册
- **008-RHI**：CreateBuffer、CreateTexture、CreateSampler、CreateGraphicsPSO 等

---

## 3.6 其他约定

- **数据归属**：ModelAssetDesc、TextureAssetDesc 归属 013；ShaderAssetDesc→010，MaterialAssetDesc→011，**LevelAssetDesc/SceneNodeDesc→029-World**（004 只负责场景结构与管理，不负责资源），MeshAssetDesc→012。各模块向 002 注册自己的描述类型；013 反序列化后交对应模块或自身组装。
- **Entity / Scene / ModelComponent**：节点或组件挂 ResourceId（modelGuid）或句柄；004/005 不持有 IModelResource*；渲染时由 Pipeline 经 013 LoadSync/GetCached 解析。
- **可寻址与打包**：ResourceId、GUID、Address、BundleMapping 由 013 Addressing 子模块负责；与磁盘“一目录一资源”结构可并存（打包时按 GUID 映射到包内路径或偏移）。

以上与 [01-asset-data-structure](./01-asset-data-structure.md)、[02-asset-loading-flow](./02-asset-loading-flow.md) 及模块规格、契约保持一致，共同构成引擎资源依赖、存储、加载与数据格式的完整设计文档。
