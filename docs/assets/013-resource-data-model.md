# 013-Resource 资源数据模型

本文档定义由 013-Resource 拥有的**引擎自有格式** FResource 描述：Model、Texture、Mesh。贴图采用**方案 B**（TextureDescriptor）；网格采用**方案二**（.mesh 引擎缓存）。**Import 与 Load/Save 分离**：Mesh 与 Texture 的图片数据在 **Import 阶段**由外部格式（.obj/.fbx、.png/.jpg 等）**一次性转换为引擎内部统一格式**（.mesh、.texdata）；**Load/Save 仅操作引擎内部格式**，运行时不再依赖外部库（如 assimp、libpng）解析，流程统一、简化。

- **格式归属**：013-Resource
- **序列化**：002-Object（ISerializer + 已注册类型，用于 .texture/.material/.model/.level）；.mesh、.texdata 为 013 约定的二进制格式。
- **磁盘目录**：每个资源单独一个子目录；贴图目录内放 .texture、.texdata 及导入前原始资源（如 .png），Mesh 目录内放 .mesh 及导入前原始资源（如 .obj）；导入时须把原始资源一并放入该目录。详见 [resource-serialization.md §7 磁盘目录保存方式](./resource-serialization.md#7-磁盘目录保存方式)。
- **GUID → 路径**：引擎通过 ResourceId（GUID）加载资源时，由 013 将 GUID 解析为资源子目录路径；解析方式（约定派生或注册表）见 [resource-serialization.md §3.1 引擎如何通过 GUID 找到资源路径](./resource-serialization.md#31-引擎如何通过-guid-找到资源路径addressing)。
- **加载流程**：013 按资源类型的加载步骤及各模块（011/012/004/005/020）与 013 的衔接见 [resource-loading-flow.md](./resource-loading-flow.md)。

---

## ModelAssetDesc

用于 `.model` 资源文件，描述「用哪些 Mesh、用哪些 Material」及子网格与材质的对应关系。

| 字段 | 类型 | 说明 |
|------|------|------|
| formatVersion | uint32 | 格式版本，用于迁移 |
| debugDescription | string | 明文描述，用于 Debug（日志/dump/编辑器展示）；UTF-8 |
| meshGuids | array of GUID | Mesh 资源引用列表（可多个 LOD 或子网格来源） |
| materialGuids | array of GUID | Material 资源引用列表 |
| submeshMaterialIndices | array of uint32 | 每个子网格使用的材质索引（下标对应 materialGuids） |
| name | string | 可选；资源/模型名称 |
| bounds | 见下 | 可选；包围盒等，按需扩展 |

**子网格与材质对应**：若 Mesh 有 N 个子网格，则 `submeshMaterialIndices` 长度为 N；`submeshMaterialIndices[i]` 为第 i 个子网格使用的材质在 `materialGuids` 中的下标。

- **文件扩展名**：`.model`

---

## TextureDescriptor（贴图采用方案 B）

贴图资源使用 **TextureDescriptor**（Object 可序列化）。**sourceImageGuid 引用的是引擎内部图片资源**（Import 阶段由 .png/.jpg 等转换得到的 .texdata），Load 时仅加载该内部格式，不再解码外部图片。

| 字段 | 类型 | 说明 |
|------|------|------|
| formatVersion | uint32 | 格式版本，用于迁移 |
| debugDescription | string | 明文描述，用于 Debug（日志/dump/编辑器展示）；UTF-8 |
| sourceImageGuid | GUID | **引擎内部图片资源**引用（.texdata，由 Import 从 .png/.jpg 等转换）；Load 时只读 .texdata，不调外部库 |
| wrapU, wrapV | enum 或 uint8 | 纹理包装方式（Repeat / Clamp / Mirror 等） |
| filterMin, filterMag | enum 或 uint8 | 缩小/放大过滤（Linear / Nearest 等） |
| mipCount | uint32 | 可选；mip 层数，0 表示由实现自动生成 |

- **文件扩展名**：`.texture`
- **Import**：外部图片（.png/.jpg 等）→ 解码 → 写出**引擎内部图片格式 .texdata**；写出 .texture，sourceImageGuid 指向该 .texdata。
- **Load**：013 反序列化 .texture → 按 sourceImageGuid 加载 **.texdata**（无外部解码）→ 按采样设置创建 ITextureResource（DResource）。

### 引擎内部图片格式 .texdata

由 **Import** 从 .png/.jpg/.webp/.ktx2 等解码后写出的**统一二进制格式**，013 约定布局；**Load 时仅读取 .texdata**，不再调用 stb/libpng 等。建议包含：formatVersion、debugDescription（可选）、像素格式（与 RHI 纹理格式对应）、宽/高/深度、mip 层数、层数据（紧密存储）。具体字段与字节序由 013 与 008-RHI 在实现中约定。扩展名：**.texdata**（或 013 指定）。

---

## Texture 资源形态（具体是什么）

- **磁盘上（仅引擎内部格式）**：
  - **贴图资源** = **.texture 文件**（TextureDescriptor 的 Object 序列化）+ **.texdata 文件**（引擎内部图片格式，由 Import 从 .png/.jpg 等一次性转换）。sourceImageGuid 指向 .texdata；运行时 **Load 不再读取或解码 .png/.jpg**。
  - 外部图片（.png/.jpg 等）仅在 **Import** 时使用，转换后以 .texdata 存在；若未导入则无 .texdata，贴图不可用。
- **加载后**：013 反序列化 .texture → 按 sourceImageGuid 加载 **.texdata**（统一格式，无外部库）→ 按采样设置创建 ITextureResource（DResource）。材质中引用的“贴图”是 .texture 的 GUID。

---

## Mesh 资源形态（采用方案二）

Mesh 统一使用**方案二**：**Mesh 资源 = 引擎缓存格式 .mesh 文件**。外部格式（.obj / .fbx / .gltf）**仅在 Import 阶段**由 013/012 用外部库解析，**一次性转换为 .mesh**；**Load 仅读取 .mesh**，不再解析任何外部几何格式，流程统一。

- **磁盘上（仅引擎内部格式）**：Mesh 资源 = 一个 **.mesh** 二进制文件，013 与 012 约定的缓存格式。建议包含：
  - **formatVersion**（uint32）：格式版本，用于迁移。
  - **debugDescription**（string，UTF-8）：明文描述，用于 Debug。
  - **vertexLayout**：顶点布局标识或描述（与 RenderCore/RHI 顶点格式一致）。
  - **vertexData**：顶点缓冲数据（紧密存储）。
  - **indexFormat**：索引格式（uint16 / uint32）。
  - **indexData**：索引缓冲数据。
  - **submeshes**：子网格数组（每项含 offset、count、可选 materialSlotIndex 等）。
  - **可选**：LOD 信息、蒙皮数据（骨骼索引/权重、BindPose）等；由实现约定。
- **Import**：.obj / .fbx / .gltf 等 → 外部库解析 → 写出 **.mesh**；此后该 Mesh 资源仅以 .mesh 存在。
- **加载后**：013 只读 **.mesh** 二进制，按 formatVersion 与约定解析，交 012 创建 **IMeshResource**；**不在此阶段调用 assimp/cgltf 等**。

---

## 贴图与 Mesh 在引擎内部的格式及保存位置

**原则**：Import 将外部数据转为**引擎内部统一格式**；Load/Save **仅**读写这些内部格式，运行时**不依赖外部库**。

### 贴图（Texture）

| 层次 | 引擎内部格式 | 保存在哪里 |
|------|----------------|------------|
| **描述层** | TextureDescriptor（formatVersion、debugDescription、sourceImageGuid、采样设置等） | **.texture 文件**（Object 序列化）；格式见 §TextureDescriptor。 |
| **源数据层** | 引擎内部图片格式（像素/块布局由 013 约定，便于直接上传 GPU 或少量处理） | **.texdata 文件**（或 013 约定的其它扩展名/包内块）。由 Import 从 .png/.jpg 等**一次性**生成；**Load 只读 .texdata**，不解码 .png/.jpg。 |
| **GPU 层** | DResource（纹理对象、格式与 RHI 一致） | **不持久化**；运行时由 013 根据 TextureDescriptor + .texdata 创建，格式见 008-RHI。 |

### Mesh

| 层次 | 引擎内部格式 | 保存在哪里 |
|------|----------------|------------|
| **磁盘缓存** | .mesh 二进制（formatVersion、debugDescription、顶点布局、vertexData、indexData、submeshes 等） | **.mesh 文件**；格式见 §Mesh 资源形态。由 Import 从 .obj/.fbx/.gltf **一次性**转换；**Load 只读 .mesh**，不解析外部几何。 |
| **内存/运行时** | IMeshResource、012 MeshHandle（顶点/索引缓冲、子网格、可选 LOD/蒙皮） | **不持久化**；由 013 读 .mesh 后交 012 创建，见 012-Mesh 与 013-Resource 契约。 |

**格式定义文档归属**：本文档约定 .texture 描述、.texdata 与 .mesh 的磁盘布局；GPU 纹理格式、顶点格式等由 008-RHI、009-RenderCore、012-Mesh 契约补充。

---

## 加载进内存后：Mesh、Material、Texture、Model、Scene 保存在哪里（哪个类/对象）

加载完成后，**资源类（Mesh/Material/Texture/Model）统一由 013-Resource 以 RResource 形态持有**，通过接口指针返回；**Scene/Level** 由 004-Scene 持有，暴露场景图与 World。DResource（GPU 资源）按契约**保存在 RResource 内部**。

| 资源类型 | 加载后保存位置（概念） | 暴露给调用方的类型（接口/句柄） | 说明 |
|----------|------------------------|----------------------------------|------|
| **Texture** | 013-Resource 的 **RResource**（内部持有 DResource：GPU 纹理） | **ITextureResource***（IResource）；`RequestLoadAsync(..., Texture, ...)` 返回 | 013 解析 .texture + .texdata 后创建 RResource；生命周期由 013 管理，`Release()` 交还 013。 |
| **Mesh** | 013-Resource 的 **RResource**（内部或委托 012 的 **MeshHandle** 持有几何） | **IMeshResource***（IResource）；`RequestLoadAsync(..., Mesh, ...)` 返回 | 013 解析 .mesh 后交 012 创建数据，013 的 RResource 实现 IMeshResource；Unload 时与 012 协调。 |
| **Material** | 013-Resource 的 **RResource**（内部持有/引用 011 的 **MaterialHandle**） | **IMaterialResource***（IResource）；`RequestLoadAsync(..., Material, ...)` 返回 | 013 反序列化 .material 后交 011 创建 MaterialHandle，013 包装为 IMaterialResource；Unload 时与 011 协调。 |
| **Model** | 013-Resource 的 **RResource**（内部聚合若干 **IMeshResource***、**IMaterialResource*** 及子网格材质映射） | **IModelResource***（IResource）；`RequestLoadAsync(..., Model, ...)` 返回 | 013 反序列化 .model 后按 meshGuids/materialGuids 加载 Mesh/Material，组装为 IModelResource；内部持有对 Mesh/Material 的引用，生命周期由 013 管理。 |
| **Scene / Level** | 004-Scene 的 **场景图 / World**（节点树、根实体、层级；与 005-Entity 根实体挂接） | **SceneRef / WorldRef**（004-Scene）；Level 由 `LoadLevel`/`LoadScene` 等与 013 约定加载 | 013 或 004 反序列化 .level 得到 LevelAssetDesc，004 创建场景图与节点、按 modelGuid 等向 013 请求加载并挂到节点；场景数据保存在 004 的 World/Scene 对象中，不实现 IResource。 |

**统一约定**：

- **013 持有的资源**：Mesh、Material、Texture、Model 均以 **013-Resource 的 RResource** 形式存在；具体实现类由 013 定义（如 `MeshRResource`、`MaterialRResource`、`ModelRResource` 等）。调用方通过 **IResourceManager::RequestLoadAsync / LoadSync** 取得 **IResource***，按 ResourceType 向下转型使用；生命周期由 **IResourceManager** 管理，**IResource::Release()** 或 **Unload(IResource*)** 交还 013。
- **004 持有的场景**：Scene/Level 加载后由 **004-Scene** 的 World/Scene 对象持有（场景图、NodeId、与 Entity 根挂接）；暴露 **SceneRef / WorldRef**。Level 资源句柄与 013 的加载约定由 004 与 013 在实现中对接。

---

## RResource 到 DResource 的流程

**DResource 的含义**：**DResource** 即**渲染资源**，对应 **GPU 显存**中的对象（纹理、缓冲等），由 008-RHI 创建并管理；不单独作为跨模块引用，仅作为 RResource（或 011 的 MaterialHandle、012 的 MeshHandle）内部的成员存在。

**关系**：**RResource** 是 013 的运行时资源对象（实现 IResource/ITextureResource 等）；对需要占用 GPU 的资源类型，RResource **内部可持有** **DResource**（或持有 012/011 的句柄，该句柄内部可含 DResource）。**设备层在创建完成前可能尚未持有实际 DResource**，见下「DResource 的异步创建与按需创建」与「设备层可能无 DResource 的兼容」。DResource 由 008-RHI 或 012（经 RHI）创建，**不单独作为跨模块引用对象**，仅作为 RResource 的成员存在。

### DResource 的异步创建与按需创建

**原则**：**DResource 采用异步创建**，且**仅当场景/管线收集到该 Model 时才进行实际创建**（按需创建）。Load 阶段只建立 RResource 与描述/源数据，不在此阶段创建 GPU 资源。

**两阶段**：

1. **Load 阶段（仅 RResource）**
   - 013 解析路径与引擎格式（.texture/.texdata、.mesh、.material、.model），得到**描述或源数据**（TextureDescriptor、vertexData/indexData、MaterialAssetDesc、ModelAssetDesc 等）。
   - 013 创建 **RResource**（及可选的 012 MeshHandle、011 MaterialHandle），此时**不调用 008-RHI**，不创建顶点/索引缓冲、纹理、UniformBuffer 等 DResource。RResource/MeshHandle/MaterialHandle 内**仅保存“可创建 DResource 所需的数据”**（描述、源数据），**DResource 槽位为空或占位**。
   - 返回 **IResource*** 给调用方；上层可正常持有 IModelResource* 等，但设备层尚无实际 GPU 资源。

2. **按需创建阶段（当场景收集到该 Model 时）**
   - **触发时机**：当**场景/管线收集到该 Model**（例如该 Model 被加入可见集合、被提交绘制、或首次被引用到渲染路径）时，由 020-Pipeline 或 013 触发 **EnsureDeviceResourcesAsync(IModelResource*)**（或等价接口），对该 Model 及其内部**所有** Mesh、Material、Texture **异步**创建 DResource。
   - **创建过程**：013/012/011 按「创建时的调用流程」**异步**执行：013 调 008-RHI 创建纹理、012 调 008-RHI 创建顶点/索引缓冲、011 调 008/009 创建 UniformBuffer 等；完成后将 DResource **填入**对应 RResource/MeshHandle/MaterialHandle。
   - **粒度**：可按 Model 为单位触发（递归创建其下所有 Mesh/Material/Texture 的 DResource），也可按单资源触发；由 013 与 020-Pipeline 契约约定。

**小结**：Load 只产生 RResource（无 DResource）；**当场景收集到该 Model 时才异步创建其内部全部设备资源**；设备层在创建完成前可能没有实际 DResource，上层须兼容（见下节）。

### 设备层可能无 DResource 的兼容

**前提**：RResource、MeshHandle、MaterialHandle 在 DResource 未创建或尚未创建完成时，**内部可能不持有实际 DResource**（为空或占位）。所有依赖设备资源的路径必须**兼容“无 DResource”**。

**约定**：

- **查询接口**：013/012/011 提供**是否已具备设备资源**的查询（如 **IsDeviceReady()**、**HasDResource()**、或 IResource::GetDeviceState()）。Pipeline、绑定、上传等逻辑在访问 DResource 前应先查询，若未就绪则采取下述行为之一。
- **绘制前**：020-Pipeline 在提交 DrawCall 前，若发现 Model/Mesh/Material/Texture 的 **IsDeviceReady() 为 false**，可选择：**跳过该次绘制**（本帧不画）、**等待异步完成**（阻塞或下一帧再试）、或**使用占位资源**（如默认白贴图、简单几何）；具体策略由 Pipeline 与 013 约定。
- **绑定/上传**：009-RenderCore、011、012 在绑定纹理/缓冲或上传 Uniform 时，若内部 DResource 为空，应**安全返回或跳过**（不崩溃、不提交无效句柄）；可选返回“未就绪”状态供上层处理。
- **异步完成回调**：EnsureDeviceResourcesAsync 完成后，013 可通知 Pipeline 或场景（回调或事件），以便下一帧或下一轮收集时正常绘制。

**小结**：设备层**允许暂无 DResource**；通过 IsDeviceReady/HasDResource 查询与“跳过/等待/占位”策略，保证整条渲染路径在 DResource 未就绪时仍可安全运行。

### 创建顺序（Load 仅 RResource；按需时再创建 DResource）

Load 时：**描述/数据 → RResource（内持描述/源数据，DResource 槽位为空）**。
按需时：**场景收集到 Model → 触发 EnsureDeviceResourcesAsync → 描述/源数据 → 异步创建 DResource → 填入 RResource/MeshHandle/MaterialHandle**。

1. **Load 阶段**：013 得到描述或解析结果 → 创建 RResource（及可选的 MeshHandle/MaterialHandle），**不**调用 008-RHI；RResource 内仅保存创建 DResource 所需的数据，DResource 为空。
2. **按需阶段**：当场景收集到该 Model 时，触发异步创建 → 013/012/011 调用 008-RHI（或 009）创建纹理/缓冲等 DResource → 将 DResource 填入对应 RResource/MeshHandle/MaterialHandle。
3. **对外**：调用方始终通过 IResource* 访问；是否已有 DResource 通过 IsDeviceReady() 等查询，绘制路径须兼容未就绪情况。

### 按需阶段：DResource 的创建调用流程（谁调用谁）

以下为**按需创建 DResource 时**（EnsureDeviceResourcesAsync 触发后）的调用链；此时 RResource/MeshHandle/MaterialHandle 已存在，仅**异步填入 DResource**。若实现“Load 时即创建 DResource”，也可在 Load 阶段同步执行本流程。箭头表示调用方向，同一层级内按执行顺序排列。

**按需阶段入口**
- 020-Pipeline 或 013 在**场景收集到该 Model** 时 → **013 EnsureDeviceResourcesAsync(IModelResource*)**（或单资源）；013 对该 Model 及其下所有 Mesh/Material/Texture 递归触发 DResource 创建（**异步**）。

**Load 阶段入口（仅建 RResource，不建 DResource）**
- 调用方 → **013 IResourceManager::RequestLoadAsync(ResourceId, type)** 或 **LoadSync(ResourceId, type)** → 013 解析并创建 RResource（内持描述/源数据，DResource 槽位为空），返回 IResource*。

**Texture（按需阶段）**
1. 013 从已有 RResource 取得 TextureDescriptor 与 .texdata 源数据（Load 时已保存）。
2. 013 → **008-RHI::CreateTexture**(texdata, descriptor 采样设置) → 008 返回 **DResource**（GPU 纹理句柄/对象）。
3. 013 将 DResource **填入** 已有 TextureRResource；若在 Load 时即创建则为本步骤中 new TextureRResource(DResource) 并登记。

**Mesh（按需阶段）**
1. 013 从已有 RResource 或 .mesh 解析取得 vertexData、indexData、layout、submeshes（Load 时已保存）。
2. 013 → **012 IMeshModule::CreateMesh**(vertexData, indexData, layout, submeshes) → **012 内部调用 008-RHI**（CreateBuffer 等）得到 **DResource**，012 组装或更新 **MeshHandle**（内持 DResource），return 给 013。
3. 013 将 MeshHandle（含 DResource）**填入** 已有 MeshRResource；若在 Load 时即创建则为本步骤中 new MeshRResource(MeshHandle) 并登记。

**Material（按需阶段）**
1. 013 从已有 RResource 取得 MaterialAssetDesc（Load 时已保存）。
2. 013 → **011 IMaterialModule::CreateMaterial**(MaterialAssetDesc) 或 **EnsureDeviceResources**(MaterialHandle)。
   - 011 内部按需 → **013 RequestLoadAsync(textureGuid, Texture)** / **RequestLoadAsync(shaderGuid, Shader)** 取得贴图与 Shader；
   - 011 调用 **008** 创建 uniform/常量缓冲等（若需要），组装或更新 **MaterialHandle**，return 给 013。
3. 013 将 MaterialHandle（含 DResource）**填入** 已有 MaterialRResource；若在 Load 时即创建则为本步骤中 new MaterialRResource(MaterialHandle) 并登记。

**Model（按需阶段）**
1. 013 对 IModelResource* 下所有 IMeshResource*、IMaterialResource* 递归调用 **EnsureDeviceResourcesAsync**（单资源）。
2. 各 Mesh/Material/Texture 按上列步骤异步创建 DResource 并填入对应 RResource。
3. Model 自身无 DResource；当其下所有子资源 IsDeviceReady() 为 true 时，可视为该 Model 设备就绪。

**小结**：Load 阶段只建 RResource（DResource 槽位为空）；**当场景收集到该 Model 时**，013 触发 EnsureDeviceResourcesAsync，按上列流程**异步**创建并填入 DResource。013 与 008、012、011 契约须支持“先有 RResource/句柄、后填 DResource”及 IsDeviceReady() 查询。

### 按资源类型的对应关系

| 资源类型 | DResource 的创建者 | RResource 内如何持有 |
|----------|--------------------|------------------------|
| **Texture** | 013 调用 **008-RHI**，用 .texdata 与 TextureDescriptor 的采样设置创建 GPU 纹理 | RResource 内部直接持有该 **DResource**（纹理对象） |
| **Mesh** | 013 将 .mesh 解析结果交给 **012**，**012 调用 008-RHI**（CreateBuffer 等）创建顶点/索引缓冲（DResource），组装 **MeshHandle** | RResource 持有或引用 012 的 **MeshHandle**（MeshHandle 内包含 DResource） |
| **Material** | 011 根据 MaterialAssetDesc 创建 **MaterialHandle**（内部可含 DResource，如 uniform 缓冲、贴图绑定） | RResource 持有或引用 011 的 **MaterialHandle** |
| **Model** | 无自身 DResource；聚合若干 IMeshResource*、IMaterialResource*（其内部各有 DResource） | RResource 持有上述子资源的 **IResource*** 引用 |

### 生命周期与释放

- **RResource 被 Unload 或 Release 时**：013 负责释放其内部持有的 DResource，或通知 012/011 释放对应句柄（MeshHandle、MaterialHandle），由 013 与 012/011 契约约定。
- **不单独释放 DResource**：调用方仅对 RResource（IResource*）做 Release/Unload；013 在内部完成对 DResource（或 012/011 句柄）的释放，保证 **RResource 与 DResource 生命周期一致**。

### Material 的 UniformBuffer（DResource）的创建与设置

材质的标量/向量等参数（MaterialAssetDesc.**scalarParams**）在 GPU 侧通常通过 **Uniform Buffer**（常量缓冲）传给 Shader；该缓冲是 **DResource**（GPU 缓冲），占用 GPU 显存。

**创建**（谁创建、何时创建）：

- **011-Material** 在 **CreateMaterial(MaterialAssetDesc)** 时，根据 Shader 的 Uniform 布局（来自 **010-Shader** 反射或 **009-RenderCore** 的布局约定）申请一块 GPU 缓冲：
  - 若使用 **009-RenderCore**：011 调用 **009 UniformBuffer.Create(Layout)**（或等价接口），009 内部再调 **008-RHI** 创建 Buffer（Usage = Uniform/Constant），得到 DResource；009 可返回句柄给 011，011 将该句柄存入 **MaterialHandle**。
  - 若 011 直接对接 008：011 调用 **008-RHI::CreateBuffer**(size, usage=Uniform)，得到 DResource，011 将该 DResource 或 008 的缓冲句柄存入 MaterialHandle。
- 布局须与 Shader 的 uniform block 一致（010 反射或 009 的 ShaderParams 布局），保证 scalarParams 与 Uniform 成员一一对应。

**设置**（如何把参数写入 GPU）：

- **数据来源**：MaterialAssetDesc 的 **scalarParams**（name + value）及运行时覆盖（如每帧由 Pipeline 填写的 MVP、时间等）。
- **写入时机**：在**提交绘制前**（每帧或材质参数变更时），011 或 020-Pipeline 将当前材质参数（CPU 侧）写入 GPU 缓冲：
  - 若使用 009：调用 **009 UniformBuffer.Update**(cpuData, size) 或 **SetParam**(name, value)；009 内部 Map/Update/Unmap 或 008 UpdateBuffer 上传到 GPU。
  - 若 011 直接对接 008：011 调用 **008-RHI::UpdateBuffer**(buffer, offset, data, size) 或 Map/Write/Unmap，将按布局拼好的 cpuData 上传。
- **布局对齐**：写入的 cpuData 须与 Shader Uniform 布局、009 ShaderParams 布局一致（对齐、偏移由 009 或 010 约定）。

**绑定**（绘制时如何挂到 Shader）：

- 绘制时 **020-Pipeline** 或 **009-RenderCore** 在提交 DrawCall 前，将材质的 UniformBuffer（DResource 或 009 句柄）**绑定到 Shader 的对应 slot**（如 008 的 SetConstantBuffer(slot, buffer)）；Shader 通过该 slot 访问 uniform 数据。

**小结**：UniformBuffer 是 DResource；**创建**由 011 经 009 或 008 完成并存入 MaterialHandle；**设置**由 011 或 Pipeline 在绘制前经 009/008 上传 scalarParams；**绑定**由 Pipeline/009 在 Draw 前绑定到 Shader。具体接口由 011、009、008 的契约约定。

### 可选约定（由实现与契约补充）

- **默认策略**：DResource **异步创建**、**当场景收集到该 Model 时才创建**；设备层可能暂无 DResource，须通过 IsDeviceReady() 兼容（见上）。
- **多设备/多上下文**：若存在多 GPU 或多上下文，RResource 可持有多个 DResource（按设备/上下文索引）；具体由 013 与 008-RHI 契约约定。

---

## Entity / Component 如何引用资源

**序列化与磁盘**：场景、预制体、组件中对资源的引用**仅存 GUID（ResourceId）**，不存指针；与 002-Object 的 ObjectRef、013 的 ResourceId 一致。

**运行时（内存）**：Entity、Component 引用资源可采用以下方式之一或组合，由 004-Scene、005-Entity 与 013-Resource 约定：

| 方式 | 说明 |
|------|------|
| **ResourceId / GUID** | Component 或节点数据中保存 **ResourceId**（如 modelGuid、materialGuid、textureGuid）；需要使用时（如渲染前）由 Pipeline 或系统通过 **IResourceManager::LoadSync(ResourceId)** 或 **GetCached(ResourceId)** 取得 **IResource***（如 IModelResource*、IMaterialResource*），再提交绘制。不长期持有 IResource*，避免跨模块指针与生命周期耦合。 |
| **句柄 / 弱引用** | 若 013 提供“从 ResourceId 解析为稳定句柄”的接口，Component 可存**不透明句柄**（如 `ModelHandle`、`TextureHandle`），由 013 内部映射到 RResource；卸载时 013 使句柄失效，Component 不再使用。 |
| **运行时解析** | Level 加载时，004 根据 LevelAssetDesc 的节点 modelGuid 调用 013 加载 Model，将得到的 **IModelResource*** 或句柄挂到**场景节点**（如通过 005-Entity 的 Component）；渲染时 Pipeline 从节点取 Model/Material 等引用再提交。Component 类型（如 RenderableComponent）的字段可为 ResourceId 或句柄，由引擎在 Tick/渲染前解析。 |

**推荐**：Component 内**存 ResourceId（GUID）**；需要时由 013 解析为 IResource* 使用，用毕可不持有或仅缓存到下一帧，便于与 013 的缓存与 Unload 策略一致。若实现“句柄表”，则 Component 存句柄、013 负责 ResourceId ↔ 句柄 ↔ RResource 的映射。

契约依据：005-Entity（EntityId、ComponentHandle、与 Scene 节点关联）、004-Scene（SceneRef、WorldRef、Level 资源句柄）、013-Resource（ResourceId、IResource、RequestLoadAsync/LoadSync/Unload）。

---

## 版本与迁移

- ModelAssetDesc、TextureDescriptor 当前格式版本均为 **1**。
- 不兼容变更时递增对应描述类型的 formatVersion，并在 002-Object 的 IVersionMigration 中实现迁移。

## 引用

- 与 [resource-serialization.md](./resource-serialization.md) 一致。
- 契约：[013-resource-public-api.md](../../specs/_contracts/013-resource-public-api.md)、[013-resource-ABI.md](../../specs/_contracts/013-resource-ABI.md)。
