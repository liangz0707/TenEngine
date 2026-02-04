# 资源序列化规范

本文档描述**需序列化的资源类型、归属、Import/Load/Save 流程、类型注册、版本管理、GUID 与资源引用关系**，仅描述资源与序列化本身，不包含任务或流程安排。

---

## 1. 需序列化的资源类型与归属

引擎自有格式的 FResource 描述均为 **002-Object 已注册类型**，磁盘上以 Object 序列化形式存储；引用一律为 **GUID**（16 字节），不存指针或文件路径。

| 资源类型 | 描述类型（Object 可序列化） | 格式归属 | 扩展名 |
|----------|-----------------------------|----------|--------|
| **贴图 Texture** | TextureDescriptor（sourceImageGuid + 采样设置，Object 可序列化） | 013-Resource | .texture |
| **材质 Material** | MaterialAssetDesc | 011-Material | .material |
| **网格 Mesh** | 方案二：引擎缓存 .mesh（二进制，013/012 约定格式；含 formatVersion、debugDescription、顶点/索引、子网格等） | 013-Resource + 012-Mesh | .mesh |
| **模型 Model** | ModelAssetDesc | 013-Resource | .model |
| **场景/关卡 Level** | LevelAssetDesc、SceneNodeDesc | 004-Scene | .level / .scene |

**格式定义**：Material 见 [011-material-data-model.md](./011-material-data-model.md)；Model 与 Texture 见 [013-resource-data-model.md](./013-resource-data-model.md)；Level/Scene 见 [004-scene-data-model.md](./004-scene-data-model.md)。

### 1.1 通用约定：明文描述（Debug）

所有资源描述类型均包含 **debugDescription**（string，明文）：用于调试时在日志、dump 或编辑器中可读地标识该资源。序列化时按字符串写入，反序列化后可直接用于打印或展示；实现须保证该字段为 UTF-8 明文，不用于运行时逻辑，仅用于 Debug。

---

## 2. 资源引用关系

- **MaterialAssetDesc**：引用 Shader（**shaderGuid**）、贴图（textureSlots[].textureGuid），无跨资源指针。**Shader 与贴图一致，通过 GUID 加载**：加载时材质模块用 shaderGuid 请求 Shader；若 Shader 由 013 管理则 013 按 §3.1 将 shaderGuid 解析为路径并加载，若由 010-Shader 管理则由 010 提供按 shaderGuid 的解析与加载接口（010 与 013 契约约定）。
- **ModelAssetDesc**：引用 Mesh 列表（meshGuids）、Material 列表（materialGuids）、子网格材质索引（submeshMaterialIndices）；无跨资源指针。
- **LevelAssetDesc / SceneNodeDesc**：根节点与子节点树；节点可引用 Model（modelGuid）、预制体（entityPrefabGuid）；无跨资源指针。
- **TextureDescriptor**：引用图片（sourceImageGuid）；材质中 textureSlots 引用的是 .texture 资源（即 TextureDescriptor）的 GUID。

依赖解析由 013-Resource（及 004-Scene 对 Level）在 Load 时按 GUID 完成，序列化层只读写 16 字节 GUID。

---

## 3. GUID 管理

- **含义**：全局唯一标识，与 002-Object 的 ObjectRef、013-Resource 的 ResourceId 一致；16 字节。
- **在描述中的用法**：所有跨资源引用字段均为 GUID；序列化/反序列化只读写该 16 字节，不解析为指针。
- **解析**：由 013-Resource（及消费描述的 011-Material、004-Scene）在 Load 后根据 GUID 请求加载或查找已加载资源，生成 RResource 或挂接引用。

### 3.1 引擎如何通过 GUID 找到资源路径（Addressing）

加载资源时，调用方只提供 **ResourceId（GUID）** 与（可选）**ResourceType**；013-Resource 负责将 GUID 解析为**磁盘上的资源子目录路径**，再在该目录内读取引擎格式文件（.texture、.mesh、.material 等）。解析方式须在项目与 013 之间统一约定，可采用以下两种之一或组合。

#### 方案 A：按约定直接派生路径（无注册表）

- **约定**：资源根路径（如 `assets/`）与类型子路径（如 `textures/`、`meshes/`、`materials/`、`models/`、`levels/`）固定；**每个资源的子目录名 = GUID 的字符串表示**（如 32 位小写十六进制 `a1b2c3d4e5f6...`，或 013 约定的 base64 等）。
- **公式**：  
  `资源目录 = <资源根路径> / <类型子路径> / <GUID 字符串>`  
  例如：GUID `0a1b2c3d...`、类型 Texture → `assets/textures/0a1b2c3d4e5f6789.../`
- **主文件命名**：该目录内引擎格式主文件命名由 013 与 §7 约定（如 `desc.texture`、`mesh.mesh`、`desc.material`、`desc.model`、`level.level`），或固定扩展名 + 默认文件名。
- **特点**：无需额外索引文件；给定 GUID 与类型即可直接构造路径并访问；目录名不可读，重命名/移动需改代码或工具约定。

#### 方案 B：通过资源注册表（Manifest）解析路径

- **约定**：维护一份**资源注册表**（Manifest），记录 **GUID → 相对路径（或绝对路径）** 的映射。路径可为“可读目录名 + 后缀”或任意项目约定结构。
- **注册表形态**：可由 013 约定，例如：
  - 单文件：如 `assets/resource_manifest.bin` 或 `resource_registry.json`，表项为 `(guid, type, path)`；
  - 或按类型分表：`assets/textures/manifest.json` 等。
- **解析流程**：013 收到 `RequestLoad(ResourceId, ResourceType)` 时，先查注册表得到该 GUID 对应的**资源目录路径**（或主文件路径），再在该路径下读取引擎格式文件；若未找到表项则按约定 fallback 到方案 A 或报错。
- **维护时机**：Import/保存资源时，由导入器或 013 的 Save 逻辑**更新注册表**（新增或更新 GUID → path）；删除资源时从表项中移除。
- **特点**：支持可读目录名、资源移动而不改 GUID；依赖注册表与导入/保存流程的一致性。

#### 推荐与契约

- **实现必须二选一或兼有**：要么仅用方案 A（纯约定），要么引入方案 B（注册表）；若兼有，须约定**先查注册表，未命中再按方案 A 派生**（或反之，由 013 契约明确）。
- **013 对外契约**：013-Resource 的 **IResourceManager::RequestLoadAsync / LoadSync** 等接口的入参为 **ResourceId（GUID）** 及 **ResourceType**；内部实现负责完成 **GUID → 资源路径** 的解析，调用方不传入文件路径。路径解析的具体策略（A/B 或组合）由 013 与项目配置约定，并在 `specs/_contracts/013-resource-ABI.md` 或等价契约中写明。

---

## 4. 类型注册方式

- **机制**：002-Object 的 **TypeRegistry**。每种描述类型对应一个 **TypeId** 与 **TypeDescriptor**（名称、大小，完整版含 PropertyDescriptor 列表）。
- **谁注册**：使用该描述类型的模块在启动/模块加载时注册——011-Material 注册 MaterialAssetDesc；013-Resource 注册 ModelAssetDesc、TextureDescriptor；004-Scene 注册 LevelAssetDesc、SceneNodeDesc。
- **时机**：须在 Core 初始化之后、任何对该类型的序列化/反序列化之前完成。
- **用法**：013 在 Load 时根据资源类型或扩展名得到描述类型名，通过 `GetTypeByName(...)` 取 TypeId，再用于 `Deserialize(buf, obj, typeId)`；Save/Import 时用同一 TypeId 做 `Serialize(out, obj, typeId)`。

---

## 5. 版本管理方式

- **格式版本**：每种描述类型带有 **formatVersion** 字段（如 uint32）；用于识别磁盘格式版本。
- **迁移**：002-Object 提供 **IVersionMigration::Migrate(buf, fromVersion, toVersion)**；反序列化前若读出版本低于当前支持版本，可先调用 Migrate 再反序列化。序列化始终按当前版本写出。
- **兼容性**：不兼容的格式变更时递增 formatVersion，并在 IVersionMigration 中实现旧版到新版的转换逻辑。

---

## 6. Import / Load / Save 流程

**原则**：Mesh 与 Texture 的图片数据在 **Import 阶段**转为**引擎内部统一格式**（.mesh、.texdata）；**Load/Save 仅操作这些内部格式**，运行时不再通过外部库加载，流程统一、简化。

### Import（外部 → 引擎内部格式）

- **描述类资源**（.material、.model、.level、.texture）：导入器产出描述结构实例，由 002-Object 的 **ISerializer::Serialize** 写入磁盘或包内；元数据与依赖从描述中的 GUID 集合得出。
- **Mesh**：外部几何（.obj/.fbx/.gltf）→ 用外部库解析 → **一次性写出 .mesh**；此后该 Mesh 仅以 .mesh 存在。
- **Texture 图片数据**：外部图片（.png/.jpg 等）→ 解码 → **一次性写出 .texdata**；.texture 的 sourceImageGuid 指向该 .texdata。此后 Load 不再读取或解码 .png/.jpg。

### Load（仅引擎内部格式 → 内存 RResource）

- 013 从磁盘/包**只读引擎内部格式**：.texture + .texdata、.mesh、.material、.model、.level 等；**不在此阶段解析 .png/.obj/.fbx**。
- **贴图 / Mesh 无引擎格式时的回退**：加载 **Texture** 或 **Mesh** 时，若在资源子目录内**不存在 .texture（及 .texdata）或 .mesh**，013 先在该目录内**查找原始文件**（贴图：.png/.jpg/.webp 等；Mesh：.obj/.fbx/.gltf 等），**执行一遍 Import（转为引擎格式）→ Save（写入 .texture/.texdata 或 .mesh）**，**再按正常 Load** 读入引擎格式并构建 RResource。即：仅放原始资源进目录时，首次加载会自动完成“导入 → 保存 → 加载”。
- 描述类：读入字节 → **SerializedBuffer** → 按类型 GetTypeByName 得 TypeId → 分配目标内存（或 CreateInstance）→ **Deserialize**；若需版本迁移则先 **Migrate**。
- 得到描述或缓存数据后，013 交对应模块（011/012/004）按 GUID 解析依赖并构建 **RResource**；Texture 按 sourceImageGuid 加载 .texdata，Mesh 读 .mesh，无外部库调用（若已通过回退完成 Import+Save，则本步读刚生成的引擎格式）。**默认不在此阶段创建 DResource**；DResource 在场景/管线收集到该 Model 时**异步按需创建**（见 [013-resource-data-model.md §RResource 到 DResource 的流程](./013-resource-data-model.md#rresource-到-dresource-的流程)）。

### Save（内存 → 引擎内部格式）

- 描述类：构造描述实例 → **ISerializer::Serialize** → 写盘或包。
- Mesh/Texture 图片：写出**引擎内部格式**（.mesh、.texdata），与 Load 所用格式一致，不写回 .obj/.png。

---

## 7. 磁盘目录保存方式

**原则**：**每个资源单独占用一个子目录**；该目录内存放该资源的引擎格式文件，以及（若有）导入前的原始资源。导入时须**把“导入前的原始资源”一并放入该资源的子目录**，便于溯源与重新导入。

### 7.1 目录命名与根路径

- **资源根路径**：由项目或 013 配置约定（如 `assets/`、`content/`）；以下均相对于该根路径。
- **子目录命名**：每个资源对应一个子目录，命名由 013 与项目约定，且须与 **§3.1 引擎如何通过 GUID 找到资源路径** 的解析方式一致：
  - 若采用**方案 A（按约定派生）**：子目录名 = **ResourceId/GUID** 的字符串形式（如 32 位小写 hex 或 013 约定的 base64）。
  - 若采用**方案 B（注册表）**：子目录可为 **可读名 + ResourceId 后缀**（如 `diffuse_albedo_<id>`）或任意路径，由注册表记录 GUID → 该目录路径。
- 同一资源的所有相关文件（描述、引擎缓存、原始文件）均放在**同一子目录**内。

### 7.2 贴图（Texture）

每个贴图资源一个子目录，目录内包含：

| 文件 | 说明 |
|------|------|
| **.texture** | TextureDescriptor 的 Object 序列化结果（描述层）；Load 时必读。 |
| **.texdata** | 引擎内部图片格式；由 Import 从原始图片生成，Load 时必读。 |
| **导入前的原始资源** | 如 **.png / .jpg / .webp** 等；**每次导入时须一并放入本目录**（复制或移动），便于溯源与重新导入。若有多张源图（如 albedo + normal），可并存于同目录，.texture 的 sourceImageGuid 指向本次导入所用的一张（或约定主图文件名）。 |

示例：`assets/textures/<ResourceId>/` 下存在 `desc.texture`、`image.texdata`、`source.png`。

### 7.3 网格（Mesh）

每个 Mesh 资源一个子目录，目录内包含：

| 文件 | 说明 |
|------|------|
| **.mesh** | 引擎缓存格式；Import 从原始几何生成，Load 时必读。 |
| **导入前的原始资源** | 如 **.obj / .fbx / .gltf**；**每次导入时须一并放入本目录**，便于溯源与重新导入。 |

示例：`assets/meshes/<ResourceId>/` 下存在 `mesh.mesh`、`source.obj`。

### 7.4 材质（Material）、模型（Model）、场景/关卡（Level）

- **Material**：每个材质资源一个子目录，目录内至少包含 **.material**（引擎描述）；若有“原始”来源（如外部工具导出的 json/资产），导入时也可一并放入该子目录。
- **Model**：每个模型资源一个子目录，目录内至少包含 **.model**；若有原始来源可一并放入。
- **Level/Scene**：每个关卡/场景一个子目录，目录内至少包含 **.level** 或 **.scene**；若有原始来源可一并放入。

以上类型的 **Load 仅读取引擎格式文件**（.material、.model、.level/.scene）；原始文件仅用于溯源与重新导入。

### 7.5 导入流程与目录的对应关系

- **Import 时**：先确定该资源的 **ResourceId** 与**目标子目录**（如 `assets/<类型>/<ResourceId>/`）；若目录不存在则创建。将**导入前的原始资源**复制或移动到该子目录内；再在该子目录内生成引擎格式（.texture、.texdata、.mesh、.material 等）。禁止把同一资源拆到多个目录。
- **Load 时**：根据 ResourceId 或路径解析到该子目录。若存在引擎格式文件则直接读取；**贴图/Mesh 若目录内无 .texture/.texdata 或 .mesh**，则先在该目录内用原始文件执行 **Import → Save**，再读取生成的引擎格式（见 §6 Load 回退）。
- **Save 时**：将引擎格式写回该资源的子目录；原始资源已存在则不改动，仅更新引擎格式。

---

## 8. 小结表

| 项 | 说明 |
|----|------|
| **可序列化资源类型** | MaterialAssetDesc、ModelAssetDesc、LevelAssetDesc/SceneNodeDesc、TextureDescriptor。Mesh 采用方案二：.mesh 引擎缓存（二进制），非 Object 描述类型。 |
| **归属** | Material → 011-Material；Model、Texture → 013-Resource；Level/Scene → 004-Scene。 |
| **引用** | 仅 GUID（16 字节）；解析在 Load 阶段由 013/004 完成。 |
| **注册** | 各模块用 TypeRegistry::RegisterType 登记描述类型，Load/Save 时按 TypeId 序列化/反序列化。 |
| **版本** | 描述内 formatVersion + IVersionMigration 在反序列化前迁移。 |
| **磁盘目录** | 每资源一子目录；贴图目录含 .texture、.texdata 及导入前原始资源（如 .png）；Mesh 目录含 .mesh 及导入前原始资源（如 .obj）；导入时须把原始资源一并放入该目录。 |
| **Import/Load/Save** | Import：外部数据 → 转为引擎内部格式并写入该资源子目录，原始资源一并放入；Load/Save 仅操作内部格式，不依赖运行时外部库。 |
