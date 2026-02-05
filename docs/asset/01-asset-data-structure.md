# 1. 资源数据结构与数据表述

本文档**仅描述资源的磁盘结构、内存结构与数据表述**，不涉及加载/创建流程。依据 `specs/_contracts/000-module-dependency-map.md` 中的资源引用及资源相关模块设计。

---

## 1.1 设计原则（必须满足）

| # | 原则 | 说明 |
|---|------|------|
| 1 | **GUID 管理** | 所有资源依 **GUID**（ResourceId）进行管理；磁盘描述与引用仅使用 GUID，不存指针。 |
| 2 | **IResource 统一基类** | 所有可加载资源继承自 **IResource**，需统一的加载、卸载、导入接口。 |
| 3 | **磁盘资源结构** | **一个目录存储一个资源**：资源描述（.material / .mesh / .texture / .model / .level 等）、实际数据（.texdata / .meshdata 等，如需要）、导入前源数据（.png / .obj 等）。 |
| 4 | **内存结构** | 资源在内存中通过 **IResource** 表示；**使用方**仅持 ResourceId 或句柄、经 Resource 层解析；**资源内部**RResource 间通过指针做层级引用（见 §1.6）。 |
| 5 | **GPU 资源** | **IResource 子类**提供 **EnsureDeviceResources**；在需要渲染时由子类实现内部调用 008-RHI（或经 028/011/012 封装）创建 DResource。 |

---

## 1.2 GUID 与资源标识

- **ResourceId** 与 **GUID** 同义：资源的**全局唯一标识**。
- **用途**：
  - 磁盘上资源描述之间**仅通过 GUID 引用**；
  - 运行时缓存按 **ResourceId** 查找，存 **IResource***；
  - **可寻址**：ResourceId（GUID）可解析为资源路径或包内路径；**GUID 与文件路径的对应方式**（约定派生或 Manifest 等）见 **02-asset-loading-flow** §2.2。
- **打包**：Bundle 内通过「GUID → 包内路径/偏移」映射定位资源。
- **引用解析**：描述文件中的 GUID 引用由 **002-Object** 在反序列化时按类型注册与引用解析约定处理。

**引用方式小结**：

| 形态 | 引用方式 | 说明 |
|------|----------|------|
| **FResource（磁盘）** | 仅 **GUID** | 描述文件中不存指针，只存 GUID。 |
| **RResource（内存）** | **指针**引用其他 RResource | 由各 IResource 实现按依赖关系填充。 |
| **DResource（GPU）** | 由 RResource 内部持有 | IResource 子类在 EnsureDeviceResources 时调 008 创建。 |

---

## 1.3 资源类型与描述类型（统一对齐）

所有可加载资源均**继承 IResource**；每种资源对应**一种描述类型（*AssetDesc）**，归属一个模块，磁盘上对应一种描述文件扩展名。

### 1.3.1 资源与描述归属总表

| 资源类型 | 接口 | 描述类型（AssetDesc） | 归属模块 | 磁盘描述扩展名 |
|----------|------|----------------------|----------|----------------|
| Shader | IShaderResource | **ShaderAssetDesc** | 010-Shader | .shader |
| Material | IMaterialResource | **MaterialAssetDesc** | 011-Material | .material |
| Mesh | IMeshResource | **MeshAssetDesc** | 012-Mesh | .mesh |
| Texture | ITextureResource | **TextureAssetDesc** | 013-Resource | .texture |
| Model | IModelResource | **ModelAssetDesc** | 013-Resource | .model |
| Level | ILevelResource | **LevelAssetDesc**、**SceneNodeDesc** | 029-World | .level |
| Audio（若需） | IAudioResource | **AudioAssetDesc**（若需） | 016-Audio | 按需 |

- **029-World**：拥有 LevelAssetDesc、SceneNodeDesc。
- **004-Scene**：由 029 等依赖，提供场景结构、场景管理、数据管理。

### 1.3.2 IResource 统一接口（概念层）

- **加载 / 卸载 / 导入**：统一接口，由 Resource 层与各 IResource 实现配合完成（具体流程见 02-asset-loading-flow）。
- **设备资源**：IResource 子类提供 **EnsureDeviceResources**；在需要渲染时，子类实现内部调用 008-RHI（或经 028/011/012 封装）创建 DResource。

---

## 1.4 资源三态：FResource / RResource / DResource（内存与磁盘结构）

| 形态 | 语义 | 引用方式 | 持有方 |
|------|------|----------|--------|
| **FResource** | 硬盘（或资源包）上的表示 | 仅 GUID | 描述文件与实际数据在磁盘；反序列化后得到内存中的 *AssetDesc 与数据。 |
| **RResource** | 内存中的表示，IResource 实现体 | 指针引用其他 RResource；内部预留 DResource 槽位 | 运行时缓存存 IResource*，按 ResourceId 查找。 |
| **DResource** | GPU 资源（Buffer、Texture、PSO 等） | 由 RResource 内部持有 | IResource 子类在 EnsureDeviceResources 时调 008 创建。 |

部分资源可能只存在某一形态，按需定义。

---

## 1.5 磁盘资源结构（一目录一资源）

约定：**一个目录存储一个资源**，目录内包含以下三类文件（按需存在）。

### 1.5.1 文件分类

| 类别 | 扩展名示例 | 说明 |
|------|------------|------|
| **资源描述** | .material、.mesh、.texture、.model、.level、.shader 等 | 对应上表各 *AssetDesc 的序列化；元数据与引用（GUID）。 |
| **实际数据** | .texdata、.meshdata 等 | 导入后的二进制数据（如纹理像素、顶点/索引流）。 |
| **导入前源数据** | .png、.jpg、.obj、.fbx、.wav、.ogg、.hlsl、.glsl 等 | 美术/设计原始资产；导入管线产出描述 + 实际数据。 |

### 1.5.2 按资源类型的目录示例（与 1.3.1 表一致）

| 资源类型 | 目录内容示例 |
|----------|----------------|
| 材质 | `<guid>/` 下 `.material`（MaterialAssetDesc）；贴图在各自目录。 |
| 网格 | `<guid>/` 下 `.mesh`（MeshAssetDesc）+ `.meshdata`（顶点/索引等）+ 可选 `.obj`/`.fbx`。 |
| 贴图 | `<guid>/` 下 `.texture`（TextureAssetDesc）+ `.texdata`（像素数据）+ 可选 `.png`/`.jpg`。 |
| 模型 | `<guid>/` 下 `.model`（ModelAssetDesc：若干 Material/Mesh GUID）+ 可选 .fbx。 |
| 关卡 | `<guid>/` 下 `.level`（LevelAssetDesc、SceneNodeDesc，节点引用 Model GUID）。 |
| 着色器 | `<guid>/` 下 `.shader`（ShaderAssetDesc 或源码）+ 可选 .hlsl/.glsl。 |
| 音频 | `<guid>/` 下描述 + 实际数据或内联 + 可选 .wav/.ogg。 |

同一资源的 LOD 或流式块可在同目录下约定命名（如 `mesh_lod0.meshdata`）或在描述中声明。

---

## 1.6 内存中的资源结构（RResource）

内存中资源的引用方式分**两层**，二者同时成立、不冲突：

### 1.6.1 使用方对资源的引用

- **使用方**（029-World、005-Entity、020-Pipeline、016-Audio 等）**不长期持有 IResource***。
- 使用方仅持 **ResourceId** 或**句柄**；需要访问资源时，**通过 Resource 层（013）解析**到 IResource* 使用，用毕不保留指针。004 由 029 等依赖，提供场景结构、场景管理、数据管理。

### 1.6.2 资源内部（RResource）的引用

- 所有资源在内存中由 **IResource**（RResource）表示；Resource 层缓存按 **ResourceId** 查找，存 **IResource***。
- **RResource 内部**可持有对其它 **RResource** 的**指针**，形成层级/依赖引用（如 Material 持有所引用的 Texture、Shader 的 RResource 指针）；由各 IResource 实现按依赖关系填充。
- RResource 还持有反序列化得到的 *AssetDesc 与内存数据或句柄，以及预留的 **DResource** 槽位（由 IResource 子类在 EnsureDeviceResources 时调 008 创建并填入）。

---

## 1.7 数据格式与版本

- 各 *AssetDesc 的序列化格式与版本由归属模块定义或经 **002-Object** 注册，与 002 的 VersionMigration、ObjectRef/GUID 解析一致。
- 描述文件中的 GUID 引用由 002 在反序列化时按引用解析约定处理。
- 资源包/Bundle 与可寻址路径为 013 Addressing 子模块范畴，与上述磁盘结构可并存。

---

## 1.8 与 000-module-dependency-map 的对应关系（结构层面）

| 模块 | 在资源/数据结构中的角色 |
|------|--------------------------|
| **013-Resource** | 统一加载入口与缓存（IResource* 按 ResourceId）；不创建 DResource，不直接调用 008。 |
| **002-Object** | *AssetDesc 类型注册；统一反序列化；描述中 GUID 引用的解析。 |
| **028-Texture / 010-Shader / 011-Material / 012-Mesh / 029-World 等** | 提供各资源类型的 IResource 实现；EnsureDeviceResources 时由子类调用 008-RHI 创建 DResource。 |
| **008-RHI** | 提供创建 DResource 的 API；由各 IResource 子类在需要渲染时调用（或经 028/011/012 封装）。 |
| **029-World** | 拥有 LevelAssetDesc、SceneNodeDesc。020/024 可选依赖 029。 |
| **004-Scene** | 由 029 等依赖，提供场景结构、场景管理、数据管理。 |

以上与 `docs/module-specs/013-resource.md`、`specs/_contracts/013-resource-public-api.md` 及 `000-module-dependency-map.md` 中的资源引用、描述归属保持一致。加载与创建流程见 **02-asset-loading-flow**。
