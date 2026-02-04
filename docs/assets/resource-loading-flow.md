# 各模块与 Resource 的资源加载流程

本文档细化 **013-Resource** 按资源类型的加载流程，以及 **011-Material、012-Mesh、004-Scene、005-Entity、020-Pipeline** 等模块与 013 的衔接方式。

---

## 1. 013-Resource 的加载流程（按资源类型）

所有资源统一通过 **IResourceManager::RequestLoadAsync(ResourceId, type, ...)** 或 **LoadSync(ResourceId, type)** 请求；入参为 **ResourceId（GUID）** 与 **ResourceType**，**不传入文件路径**。013 内部先根据 **GUID 解析为资源路径**（见 [resource-serialization.md §3.1 引擎如何通过 GUID 找到资源路径](./resource-serialization.md#31-引擎如何通过-guid-找到资源路径addressing)），定位到该资源的子目录后，按类型执行对应加载分支，最后通过回调或返回值交出 **IResource***（或失败）。

**DResource 异步创建与按需创建**：**DResource 采用异步创建**，且**仅当场景/管线收集到该 Model 时才进行实际创建**。Load 阶段只建立 **RResource**（内持描述/源数据），**不在此阶段创建 GPU 资源**（DResource 槽位为空）；当 020-Pipeline 或场景**收集到该 Model** 时，由 013 触发 **EnsureDeviceResourcesAsync(IModelResource*)**，对该 Model 及其下所有 Mesh/Material/Texture **异步**创建 DResource 并填入对应 RResource。**设备层可能暂无实际 DResource**：RResource/MeshHandle/MaterialHandle 在创建完成前可能不持有 DResource；所有使用 DResource 的路径须**兼容“无 DResource”**（通过 **IsDeviceReady()** / **HasDResource()** 查询，未就绪时跳过绘制、等待或占位）。详见 [013-resource-data-model.md §RResource 到 DResource 的流程](./013-resource-data-model.md#rresource-到-dresource-的流程)。

### 1.1 Texture

1. **由 ResourceId 解析出资源路径** → 按 §3.1 约定（约定派生或查注册表）得到该资源的子目录（如 `assets/textures/<GUID>/`）。
2. **若无引擎格式则先 Import + Save** → 若该目录下**不存在 .texture 或 .texdata**，013 在该目录内查找原始图片（.png/.jpg/.webp 等），**执行 Import**（解码并写出 .texdata、写出 .texture）→ **Save**，再继续下列步骤；否则直接下一步。
3. **读入 .texture** → 使用 002-Object 的 **Deserialize** 得到 **TextureDescriptor**（若需版本迁移则先 Migrate）。
4. **按 sourceImageGuid 定位 .texdata** → 在同一资源根下或按 013 的 Addressing 约定找到对应 .texdata 文件（若 sourceImageGuid 指向同目录内 .texdata，则可在同一子目录或约定命名）。
5. **读入 .texdata** → 按 013 约定的二进制布局解析，得到像素/块数据。
6. **创建 GPU 纹理** → 按 TextureDescriptor 的采样设置（wrapU/V、filterMin/Mag、mipCount）与 .texdata 的格式，通过 008-RHI 创建纹理对象（DResource）。
7. **组装 RResource** → 013 创建实现 **ITextureResource** 的 RResource，内部持有 DResource；做引用计数与缓存登记。
8. **返回** → 通过回调或 LoadSync 返回 **ITextureResource***（IResource*）。

### 1.2 Mesh

1. **由 ResourceId 解析出资源路径** → 按 §3.1 约定得到该资源的子目录（如 `assets/meshes/<GUID>/`）。
2. **若无引擎格式则先 Import + Save** → 若该目录下**不存在 .mesh**，013 在该目录内查找原始几何（.obj/.fbx/.gltf 等），**执行 Import**（用外部库解析并写出 .mesh）→ **Save**，再继续下列步骤；否则直接下一步。
3. **读入 .mesh** → 按 013/012 约定的二进制格式解析（formatVersion、vertexLayout、vertexData、indexData、submeshes 等）；**不读 .obj/.fbx**（若上一步刚生成 .mesh 则读该文件）。
4. **调用 012-Mesh** → 013 将解析出的顶点/索引/子网格数据交给 012；**012 调用 008-RHI**（CreateBuffer 等）创建顶点/索引缓冲（DResource），再创建 **MeshHandle**（内持上述 DResource）与 SubmeshDesc 等。
5. **组装 RResource** → 013 创建实现 **IMeshResource** 的 RResource，内部持有或引用 012 的 MeshHandle；做引用计数与缓存登记。
6. **返回** → 返回 **IMeshResource***（IResource*）。卸载时 013 与 012 协调释放 MeshHandle。

### 1.3 Material

1. **由 ResourceId 解析出资源路径** → 按 §3.1 约定得到该资源的子目录（如 `assets/materials/<GUID>/`）。
2. **读入 .material** → 使用 002-Object 的 **Deserialize** 得到 **MaterialAssetDesc**（若需版本迁移则先 Migrate）。
3. **调用 011-Material** → 013 将 MaterialAssetDesc 交给 011；011 根据 shaderGuid、textureSlots、scalarParams 等创建材质。011 内部需要贴图时**向 013 请求加载**（传入 textureSlots[].textureGuid），需要 Shader 时**通过 shaderGuid 请求加载**（向 013 的 RequestLoadAsync/LoadSync(ResourceId, Shader) 或 010 的按 GUID 加载接口），013/010 按 GUID 解析路径并返回 ITextureResource/Shader 句柄，011 再绑定到 MaterialHandle。
4. **组装 RResource** → 013 创建实现 **IMaterialResource** 的 RResource，内部持有或引用 011 的 **MaterialHandle**；做引用计数与缓存登记。
5. **返回** → 返回 **IMaterialResource***（IResource*）。卸载时 013 与 011 协调释放 MaterialHandle。

### 1.4 Model

1. **由 ResourceId 解析出资源路径** → 按 §3.1 约定得到该资源的子目录（如 `assets/models/<GUID>/`）。
2. **读入 .model** → 使用 002-Object 的 **Deserialize** 得到 **ModelAssetDesc**（meshGuids、materialGuids、submeshMaterialIndices 等）。
3. **按依赖加载 Mesh 与 Material** → 013 根据 meshGuids、materialGuids 依次调用自身的 **LoadSync 或 RequestLoadAsync**（ResourceType::Mesh、ResourceType::Material），得到 **IMeshResource***、**IMaterialResource*** 列表；若为异步则等待依赖完成。
4. **组装 RResource** → 013 创建实现 **IModelResource** 的 RResource，内部持有上述 IMeshResource*、IMaterialResource* 及 submeshMaterialIndices 映射；做引用计数与缓存登记（对依赖的 Mesh/Material 增加引用或登记依赖关系）。
5. **返回** → 返回 **IModelResource***（IResource*）。卸载时 013 释放对 Mesh/Material 的引用，按策略 Unload。

### 1.5 Level / Scene

Level 的“加载”通常由 **004-Scene** 发起（如 LoadLevel(path)），013 可只负责“读入 .level 字节并反序列化”，或 004 直接读文件后反序列化；场景图与 World 的创建由 004 完成。两种约定均可，以下以 **004 主导、013 提供反序列化与依赖加载** 为例：

1. **004-Scene** 调用加载接口（如 LoadLevel(ResourceId) 或 LoadLevel(path)）；若入参为 ResourceId，004 或 013 按 §3.1 约定解析出 Level 资源子目录；若入参为 path，则直接使用该路径定位到资源子目录。
2. **读入 .level** → 004 或 013 读入 .level 文件，使用 002-Object 的 **Deserialize** 得到 **LevelAssetDesc**（rootNodes、debugDescription 等）。
3. **004 创建场景图** → 004 根据 LevelAssetDesc 创建 World/Scene、根节点与 **SceneNodeDesc** 树；对每个节点的 **modelGuid**（及可选的 entityPrefabGuid），004 向 **013 请求加载** Model（或 Prefab），得到 **IModelResource***（或实体树），挂到节点或实体上。
4. **场景数据保存位置** → 加载后的场景图、节点、实体由 **004-Scene** 持有（SceneRef/WorldRef）；013 不返回 IResource* 给 Level 本身，只负责反序列化与为节点提供 Model 等依赖的加载。

若 Level 也通过 **RequestLoadAsync(..., Level, ...)** 由 013 统一入口加载，则 013 可反序列化 .level 后把 LevelAssetDesc 交给 004，004 创建场景并请求 Model 等依赖，013 再返回一个“Level 句柄”或由 004 直接返回 SceneRef；具体由 004 与 013 契约约定。

---

## 2. 011-Material 与 013-Resource 的衔接

- **输入**：013 将 **MaterialAssetDesc**（反序列化 .material 得到）交给 011；011 不直接读文件。
- **011 内部流程**：011 根据描述中的 shaderGuid、textureSlots[].textureGuid、scalarParams 创建材质。需要贴图时，011 调用 **013 的 LoadSync(ResourceId) 或 RequestLoadAsync**（ResourceType::Texture），传入 textureGuid，013 返回 **ITextureResource***；011 将贴图绑定到 MaterialHandle 的对应槽位。**需要 Shader 时，011 通过 shaderGuid 请求加载**：若 Shader 由 013 管理则调用 013 的 LoadSync(shaderGuid) 或 RequestLoadAsync(..., ResourceType::Shader)，013 按 §3.1 解析路径并返回 Shader 资源/句柄；若由 010 管理则调用 010 的按 shaderGuid 加载接口；011 将 Shader 绑定到 MaterialHandle。
- **输出**：011 向 013 返回 **MaterialHandle**；013 用其实现 **IMaterialResource** 并做缓存。
- **卸载**：013 在 Unload(IMaterialResource*) 时通知 011 释放对应 MaterialHandle；011 释放贴图与 Shader 引用（不直接释放 ITextureResource* 等，由 013/010 引用计数或缓存管理）。

---

## 3. 012-Mesh 与 013-Resource 的衔接

- **输入**：013 读入 **.mesh** 二进制并解析出顶点布局、vertexData、indexData、submeshes 等后，将这批数据交给 012；012 不直接读 .mesh 文件。
- **012 内部流程**：012 根据顶点布局**调用 008-RHI**（CreateBuffer，Usage=Vertex/Index）创建顶点/索引缓冲（DResource），再创建与 RenderCore/RHI 兼容的 **MeshHandle**（内持 DResource）与 **SubmeshDesc**，可选创建 LOD/蒙皮数据结构。
- **输出**：012 向 013 返回 **MeshHandle**（或 013 可读的句柄/结构）；013 用其实现 **IMeshResource** 并做缓存。
- **卸载**：013 在 Unload(IMeshResource*) 时通知 012 释放对应 MeshHandle；012 释放顶点/索引缓冲等。

---

## 4. 004-Scene 与 013-Resource 的衔接

- **Level 加载**：004 提供 **LoadLevel(path)** 或类似接口；004 或 013 读入 .level 并反序列化得到 **LevelAssetDesc**，004 根据 rootNodes 创建场景图与节点树。对每个 **SceneNodeDesc.modelGuid**，004 调用 **013 的 LoadSync(modelGuid) 或 RequestLoadAsync**（ResourceType::Model），取得 **IModelResource***，将结果挂到节点（或挂到该节点下实体的 Renderable 组件等）；005-Entity 与 004 的节点/实体结构由 004 与 005 约定。
- **场景数据持有**：加载后的 World、节点树、实体树由 **004-Scene** 持有（SceneRef/WorldRef）；013 仅负责提供 Model（及可选 Prefab）的加载。
- **卸载**：004 在 UnloadLevel 时销毁场景图与节点，并**释放对 IModelResource* 的引用**（调用 IResource::Release() 或 013 的 Unload）；013 根据引用计数决定是否卸载 Model 及下游 Mesh/Material。

---

## 5. 005-Entity 与 013-Resource 的衔接

- **组件内引用**：Entity 的 Component 中对资源的引用以 **ResourceId（GUID）** 存储（见 [013-resource-data-model.md §Entity/Component 如何引用资源](./013-resource-data-model.md)）；不长期持有 IResource*，避免与 013 生命周期耦合。
- **需要时解析**：当系统（如 020-Pipeline 渲染、024-Editor 预览）需要实际资源时，通过 **IResourceManager::LoadSync(ResourceId)** 或 **GetCached(ResourceId)**（若 013 提供）取得 **IResource***，用毕可释放或由 013 缓存；005 不负责加载，只存 ResourceId。
- **预制体 / 实例化**：若节点或实体挂载的是“预制体”资源（entityPrefabGuid），004 或 005 向 013 请求加载预制体资源（若定义为一种 ResourceType），得到实体树或模板后再实例化到场景；013 与 005 的预制体格式与加载接口由两者约定。

---

## 6. 020-Pipeline 与 013-Resource 的衔接

- **渲染时取资源**：Pipeline 从 **004-Scene / 005-Entity** 拿到待渲染的节点或实体列表，每个节点/实体可能挂有 **Model**（或 Mesh+Material）引用（ResourceId 或已解析的 IModelResource*）。若为 ResourceId，Pipeline 或上层系统先通过 **013 的 LoadSync/GetCached** 取得 **IModelResource***、**IMaterialResource***、**ITextureResource***，再提交 DrawCall（从 IModelResource 取 IMeshResource* 与材质槽，从 IMaterialResource 取贴图与参数）。
- **流式 / LOD**：若 013 提供 **RequestStreaming、SetStreamingPriority**，Pipeline 可根据视距或重要性向 013 请求流式加载或调整优先级，013 再按需加载或卸载；Pipeline 仅使用 013 返回的 IResource*，不直接管理磁盘。

---

## 7. 流程小结表

**说明**：默认 **Load 仅建 RResource**（内持描述/源数据，不创建 DResource）；**DResource 在按需阶段**（场景收集到该 Model 时）**异步创建**。下表中“创建 DResource / 包装为 ITextureResource 等”对应**按需阶段或可选同步实现**的等价步骤。

| 资源类型 | 013 内部步骤 | 依赖模块 | 下游使用方式 |
|----------|--------------|----------|--------------|
| **Texture** | 读 .texture → Deserialize → 读 .texdata → 创建 DResource → 包装为 ITextureResource | 002-Object, 008-RHI | 011 向 013 要贴图；Pipeline 从 Material 取贴图 |
| **Mesh** | 读 .mesh → 解析二进制 → 交 012；012 调 008-RHI 建顶点/索引缓冲（DResource）→ 建 MeshHandle → 包装为 IMeshResource | 012-Mesh, 008-RHI | Model 聚合；Pipeline 从 IModelResource 取 Mesh |
| **Material** | 读 .material → Deserialize → 交 011 建 MaterialHandle（011 内部通过 textureGuid/shaderGuid 向 013/010 要贴图与 Shader）→ 包装为 IMaterialResource | 002-Object, 011-Material, 010-Shader(可选) | Model 聚合；节点/实体引用；Pipeline 绑定材质 |
| **Model** | 读 .model → Deserialize → 按 meshGuids/materialGuids 加载 Mesh/Material → 聚合为 IModelResource | 自身递归加载 | 004 为节点加载 Model；Pipeline 从节点取 IModelResource |
| **Level** | 读 .level → Deserialize → 交 004 建场景；004 按 modelGuid 向 013 要 Model | 002-Object, 004-Scene | 004 持有 SceneRef；节点挂 IModelResource* |

契约与格式依据：`specs/_contracts/013-resource-ABI.md`、[013-resource-data-model.md](./013-resource-data-model.md)（含 [§RResource 到 DResource 的流程](./013-resource-data-model.md#rresource-到-dresource-的流程)）、[resource-serialization.md](./resource-serialization.md)。
