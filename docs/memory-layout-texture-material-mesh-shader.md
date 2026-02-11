# Texture / Material / Mesh / Shader 内存保存内容与位置

## 1. 028-Texture（贴图）

### 内存里存什么

| 内容 | 类型 | 说明 |
|------|------|------|
| 像素数据 | `uint8_t[]` | 原始像素，按 `TextureFormat` 解释 |
| 尺寸与格式 | `width`, `height`, `depth`, `mipLevels`, `format` | 元数据 |
| 是否 HDR | `isHDR` | 布尔 |
| 引用计数 | `refCount` | 用于释放 |

### 保存在哪里

- **TextureResource**（`TextureResource.h/cpp`）  
  - 成员：`m_textureHandle`（`TextureHandle`）、`m_resourceId`、`m_resourceManager`、`m_refCount`。  
  - 实际像素与元数据不直接放在 TextureResource 里，而是通过 **TextureHandle** 间接持有。

- **TextureHandle** = `detail::TextureData*`（`Texture.h`）。

- **detail::TextureData**（`detail/texture_data.hpp`）  
  - **真正保存像素和元数据的地方**：  
    - `pixelData`：`std::unique_ptr<uint8_t[], CoreAllocDeleter>`，用 `te::core::Alloc` 分配。  
    - `pixelDataSize`、`format`、`width`、`height`、`depth`、`mipLevels`、`isHDR`、`refCount`。  
  - 创建/释放：`TextureFactory::CreateTexture` / `ReleaseTexture`；Load 时由 `TextureResource` 调用 `CreateTexture` 得到 handle，析构时 `ReleaseTexture(m_textureHandle)`。

**小结**：贴图内存 = **TextureResource 里的 m_textureHandle 指向的 TextureData**，像素在 `TextureData::pixelData`，用 001-core 的 Alloc/Free 管理。

---

## 2. 011-Material（材质）

### 内存里存什么

| 内容 | 类型 | 说明 |
|------|------|------|
| 资源 ID | `resource::ResourceId` | 材质资源唯一标识 |
| Shader 引用 | `resource::ResourceId shaderGuid_` | 仅 GUID，不存 shader 代码/字节码 |
| 参数表 | `std::map<std::string, MaterialParam>` | 按名称的 uniform 参数（类型 + 原始字节，std140） |
| 贴图槽 | `std::vector<std::pair<std::string, std::string>>` | (binding 名称, 贴图 GUID 字符串) |
| 管线状态 | `PipelineStateDesc` | blend/depth/cull 等（仅 CPU 侧描述） |
| 保存用 JSON 缓存 | `MaterialJSONData jsonData_` | 供 Save 时序列化用 |

单参数 **MaterialParam**（`MaterialParam.hpp`）：`type`（UniformMemberType）、`count`、`data`（`std::vector<uint8_t>`）。

### 保存在哪里

- **MaterialResource**（`MaterialResource.h/cpp`）  
  - 所有上述内容都在 **MaterialResource 对象自身** 的成员里：  
    - `resourceId_`, `refCount_`, `shaderGuid_`  
    - `params_`（map 名称 → MaterialParam）  
    - `textureSlots_`（名称 → 贴图 GUID）  
    - `pipelineStateDesc_`  
    - `jsonData_`, `loaded_`  
  - 由 013-Resource 的工厂/加载器创建；通常由 ResourceManager 或业务层持有指针/句柄。

**小结**：材质内存 = **MaterialResource 实例**；不存 shader 代码或贴图像素，只存 shader GUID、参数值、贴图 GUID 和管线状态描述。

---

## 3. 012-Mesh（网格）

### 内存里存什么

| 内容 | 类型 | 说明 |
|------|------|------|
| 顶点数据 | `uint8_t[]` | 原始顶点缓冲，按 `VertexFormat` 解释 |
| 索引数据 | `uint8_t[]` | 原始索引缓冲，按 `IndexFormat`（16/32bit） |
| 顶点/索引格式 | `rendercore::VertexFormat`, `IndexFormat` | 元数据 |
| 子网格 | `std::vector<SubmeshDesc>` | offset/count/materialSlotIndex |
| LOD | `std::vector<LODLevel>` | 每级 submesh 范围与阈值 |
| 蒙皮数据 | `SkinningData*`（可选） | 骨骼/权重等 |
| 本地 AABB | `te::core::AABB` | 用于视锥剔除 |
| 引用计数 | `refCount` | 用于释放 |

### 保存在哪里

- **MeshResource**（`MeshResource.h/cpp`）  
  - 成员：`m_meshHandle`（`MeshHandle`）、`m_resourceId`、`m_refCount`。  
  - 顶点/索引等**不**直接放在 MeshResource 里，而是通过 **MeshHandle** 间接持有。

- **MeshHandle** = `detail::MeshData*`（`Mesh.h`）。

- **detail::MeshData**（`detail/mesh_data.hpp`）  
  - **真正保存顶点/索引和元数据的地方**：  
    - `vertexData` / `indexData`：`std::unique_ptr<uint8_t[], CoreAllocDeleter>`（`te::core::Alloc`）。  
    - `vertexDataSize` / `indexDataSize`  
    - `vertexLayout` / `indexFormat`  
    - `submeshes`、`lodLevels`、`skinningData`、`localAABB`、`refCount`  
  - 创建/释放：`MeshFactory::CreateMesh` / `ReleaseMesh`；Load 时由 MeshLoader 等创建 MeshData 并 `SetMeshHandle` 给 MeshResource，析构时在 MeshResource 或工厂里释放 handle。

**小结**：网格内存 = **MeshResource 里的 m_meshHandle 指向的 MeshData**；顶点在 `MeshData::vertexData`，索引在 `MeshData::indexData`，用 001-core 的 Alloc/Free 管理。

---

## 4. 010-Shader（着色器）

### 内存里存什么

**（1）ShaderResource（013 资源层）**

| 内容 | 类型 | 说明 |
|------|------|------|
| 资源 ID | `resource::ResourceId` | 与 ShaderCollection 的 key 一致 |
| 编译器 | `IShaderCompiler*` | 外部注入，编译用 |
| 句柄 | `IShaderHandle* handle_` | 指向 ShaderHandleImpl，存源码/字节码/反射等 |
| 资源描述 | `ShaderAssetDesc desc_` | guid、路径等 |
| 源码 blob | `std::vector<char> sourceBlob_` | Load 时读入的源文件内容 |

**（2）ShaderHandleImpl（编译/运行时句柄，`detail/handle_impl.hpp`）**

| 内容 | 类型 | 说明 |
|------|------|------|
| SPIR-V | `std::vector<uint32_t> bytecode_` | 当前/默认变体字节码 |
| DXIL | `std::vector<uint8_t> bytecodeBlob_` | 后端用 |
| 交叉编译源码 | `std::string crossCompiledSource_` | MSL/HLSL 等 |
| 源码路径/内容 | `sourcePath_`, `sourceCode_` | 原始 HLSL/GLSL 等 |
| 变体宏/字节码 | `variantMacros_`, `variantBytecode_` | 按 VariantKey 缓存的 SPIR-V/DXIL |
| 反射 | `reflectionMembers_`, `reflectionTotalSize_`, `reflectionResourceBindings_` 等 | Uniform/纹理/采样器布局（若启用 TE_SHADER_USE_CORE） |

**（3）ShaderCollection（全局单例，`ShaderCollection.h`）**

| 内容 | 类型 | 说明 |
|------|------|------|
| 按 ResourceId 的条目 | `std::unordered_map<ResourceId, ShaderCollectionEntry>` | LoadAllShaders 等填充 |
| 每条目 ShaderCollectionEntry | 见下 | 按 shader 一份 |

**ShaderCollectionEntry**：  
- `vertexBytecode` / `fragmentBytecode`：`std::vector<uint8_t>`（SPIR-V 等）。  
- `vertexInput` + `vertexInputAttributes`：顶点输入布局。  
- `vertexReflection` + `vertexUniformMembers` / `vertexResourceBindings`：顶点阶段 UBO/资源绑定。  
- `fragmentReflection` + `fragmentUniformMembers` / `fragmentResourceBindings`：片段阶段反射。

### 保存在哪里

- **ShaderResource**：013 的 Load 创建，由 ResourceManager 或业务层持有；**保存**的是资源 ID、编译器指针、**IShaderHandle\***（实际是 ShaderHandleImpl）、描述和源码 blob。
- **ShaderHandleImpl**：在 010-shader 内部 new/管理，**保存**源码、SPIR-V/DXIL、变体缓存和反射；生命周期与 ShaderResource 或编译/缓存逻辑绑定。
- **ShaderCollection**：单例 `ShaderCollection::GetInstance()`，**保存**全局按 ResourceId 的 SPIR-V 与反射（LoadAllShaders 时写入）；与 ShaderResource/ShaderHandleImpl 可并存（例如 Resource 用 handle 编译，Collection 用 id 查已加载的 bytecode/reflection）。

**小结**：  
- 着色器“资源”层：**ShaderResource 对象**（ID、handle、desc、sourceBlob）。  
- 着色器“可执行/反射”数据：**ShaderHandleImpl**（bytecode、dxil、反射、变体）和 **ShaderCollection 的 ShaderCollectionEntry**（按 ResourceId 的 bytecode + reflection）。  
- 三者都在进程堆内存；不涉及 GPU 显存（CPU-only 设计下）。

---

## 5. 汇总表

| 模块 | 内存里“内容” | 主要保存在哪个对象/结构 |
|------|----------------|---------------------------|
| **Texture** | 像素 + 宽高/格式/mip 等 | `detail::TextureData`（TextureHandle 指向），由 TextureResource 持有 handle |
| **Material** | shader GUID + 参数 map + 贴图 GUID 列表 + 管线状态 | `MaterialResource` 自身成员 |
| **Mesh** | 顶点/索引缓冲 + 子网格/LOD/蒙皮/AABB | `detail::MeshData`（MeshHandle 指向），由 MeshResource 持有 handle |
| **Shader** | 源码、SPIR-V/DXIL、反射、变体缓存 | `ShaderHandleImpl`（ShaderResource 的 handle_）+ `ShaderCollection::entries_`（按 ResourceId） |

所有上述数据均在 **CPU 进程堆**；Texture/Mesh 的“大块”用 **te::core::Alloc/Free**（001-core），其余为 C++ 标准容器与对象。
