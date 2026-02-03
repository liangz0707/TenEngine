# US-rendering-006：模型渲染 = Mesh + Material 组织，Model 引用 Mesh/Material，Material 保存 Shader/贴图/参数

- **标题**：模型渲染由 **Mesh** 与 **Material** 组织；Mesh 来源于 OBJ、FBX 等常用格式；Material 为引擎自有格式，**保存 Shader**，并引用贴图、材质参数；**硬盘上的 Model 资源**引用 Material 和 Mesh；Material 引用贴图、材质参数与 Shader。
- **编号**：US-rendering-006

---

## 1. 角色/触发

- **角色**：程序员（引擎或游戏侧）、美术/TA（资源与材质）
- **触发**：需要明确**模型渲染**的数据组织：模型 = Mesh（几何）+ Material（着色）；Mesh 来自 OBJ、FBX 等格式；Material 为引擎格式并持有 Shader、贴图、参数；**硬盘上的 Model 资源**引用若干 Mesh 与若干 Material；Material 资源引用贴图、材质参数与 Shader。

---

## 2. 端到端流程与数据层级

1. **模型渲染** = **Mesh** + **Material** 的组织（几何与材质的组合）。
2. **Mesh**：来源于**各种格式**的网格，如 **OBJ**、**FBX** 等常用格式；经 Resource 按类型 Mesh 加载或经 Model 资源间接引用，由 012-Mesh 提供顶点/索引与子网格。
3. **Material**：**引擎自有格式**；**Material 当中保存了 Shader**（Shader 引用由材质持有）；材质引用**贴图**与**材质参数**（即渲染 Shader 的参数值）；经 Resource 按类型 Material 加载或经 Model 资源间接引用，由 011-Material 提供材质定义与实例。
4. **硬盘上的 Model 资源**：**引用了 Material 和 Mesh**；Model 资源文件内声明对若干 Mesh 与若干 Material 的引用；经 **requestLoadAsync(path, ResourceType::Model, ...)** 加载后得到 **IModelResource**，可取得其引用的 Mesh 与 Material（或子资源句柄）。
5. **Material 资源**（磁盘/资源）：**引用贴图、材质参数、Shader**；材质中保存 Shader，并引用渲染 Shader 所需的贴图与参数值。

---

## 3. 涉及模块

| 模块 | 职责摘要 |
|------|----------|
| 013-Resource | Model/Mesh/Material 统一加载；IModelResource（引用 Mesh、Material）、IMeshResource（OBJ/FBX 等）、IMaterialResource（引擎格式，保存 Shader，引用贴图、参数） |
| 012-Mesh | Mesh 来源于 OBJ、FBX 等格式；顶点/索引、子网格、LOD；可单独加载或经 Model 引用 |
| 011-Material | 材质为引擎格式；**保存 Shader**；引用贴图、材质参数；可单独加载或经 Model 引用 |
| 010-Shader | 被 Material 引用；材质持有 Shader 句柄/变体 |
| 020-Pipeline | 使用 Model/Mesh/Material 组织进行 DrawCall 与批次 |

---

## 4. 每模块职责与 I/O

### 013-Resource

- **职责**：**硬盘上的 Model 资源引用了 Material 和 Mesh**；IModelResource 提供对若干 Mesh 与若干 Material 的引用（或子资源 ID/句柄）；IMeshResource 表示来自 OBJ/FBX 等格式的网格；IMaterialResource 表示引擎格式材质，**保存 Shader**，引用贴图、材质参数；统一 requestLoadAsync 加载 Model、Mesh、Material。
- **输入**：路径、ResourceType（Model/Mesh/Material）；Model 加载时解析其引用的 Mesh、Material 并可按需加载子资源。
- **输出**：IModelResource（getMeshRefs、getMaterialRefs 或等价接口）、IMeshResource、IMaterialResource；引用关系与依赖解析。

### 012-Mesh

- **职责**：Mesh 来源于 **OBJ、FBX** 等常用格式；提供顶点/索引、子网格、LOD；可单独以 ResourceType::Mesh 加载，或经 IModelResource 引用的 Mesh 加载。
- **输入**：来自 OBJ/FBX 等文件或 Model 资源内的 Mesh 引用。
- **输出**：MeshHandle、顶点/索引、SubmeshDesc、MaterialSlot；与 Pipeline 对接。

### 011-Material

- **职责**：材质为**引擎自有格式**；**Material 中保存 Shader**；材质引用贴图、材质参数；提供 MaterialHandle、GetShaderRef、GetParameters、GetTextureRefs；可单独以 ResourceType::Material 加载，或经 IModelResource 引用的 Material 加载。
- **输入**：材质资源（引擎格式）、Shader 引用、贴图引用、参数值。
- **输出**：MaterialHandle、材质实例、与 Shader/贴图/参数的绑定；与 Pipeline 对接。

---

## 5. 派生 ABI（与现有契约对齐）

以下与 **specs/_contracts/013-resource-ABI.md**、011-Material、012-Mesh 契约一致，无需重复新增表项；确保：

- **IModelResource**：硬盘上的 Model 资源**引用了 Material 和 Mesh**；提供对若干 Mesh、若干 Material 的引用访问。
- **IMaterialResource**：引擎格式；材质**保存 Shader**，引用**贴图**、**材质参数**。
- **IMeshResource**：Mesh 来源于 OBJ、FBX 等格式；可经 Model 引用或单独加载。

---

## 6. 验收要点

- 模型渲染可描述为「Mesh + Material 的组织」；Mesh 来自 OBJ、FBX 等格式；Material 为引擎格式并保存 Shader、引用贴图与参数。
- 硬盘上的 Model 资源可加载，且**引用**若干 Mesh 与若干 Material；Material 资源**引用**贴图、材质参数与 Shader；材质当中**保存了 Shader**。
