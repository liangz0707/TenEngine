# 011-Material 资源数据模型

本文档定义磁盘上**材质资源**的引擎自有格式（FResource 描述）。该描述为 **002-Object 可序列化类型**，引用均为 **GUID**；013-Resource 反序列化后交 011-Material 转为 RResource（MaterialHandle）。

- **文件扩展名**：`.material`
- **格式归属**：011-Material
- **序列化**：002-Object（ISerializer + 已注册类型）

---

## MaterialAssetDesc

| 字段 | 类型 | 说明 |
|------|------|------|
| formatVersion | uint32 | 格式版本，用于 IVersionMigration |
| debugDescription | string | 明文描述，用于 Debug（日志/dump/编辑器展示）；UTF-8 |
| shaderGuid | GUID (16 bytes) | Shader 资源引用；**加载时通过 shaderGuid 请求**（与 textureGuid 加载贴图一致）：向 013 或 010 传入 shaderGuid，由 013/010 按 GUID 解析路径并返回 Shader 句柄，与 010-Shader 对应 |
| textureSlots | array of TextureSlot | 贴图槽：槽位名（与 Shader 声明一致）+ 贴图 GUID |
| scalarParams | array of ScalarParam | 标量/向量等默认参数（与 Shader 参数名、类型一致） |
| variantKeywords | array of string 或 key-value | 可选；Shader 变体关键字，与 010-Shader 变体约定一致 |

### TextureSlot

| 字段 | 类型 | 说明 |
|------|------|------|
| slotName | string | 与 Shader 中纹理绑定名一致（如 `_MainTex`、`_NormalMap`） |
| textureGuid | GUID | 贴图资源引用 |

### ScalarParam

| 字段 | 类型 | 说明 |
|------|------|------|
| name | string | 与 Shader Uniform 名一致 |
| value | 标量/向量/矩阵等（按类型序列化） | 默认值；类型与 010-Shader 反射一致 |

**运行时与 UniformBuffer**：scalarParams 在 GPU 侧通过 **Uniform Buffer**（常量缓冲，即 DResource）传给 Shader。011 在 CreateMaterial 时根据 Shader/009 的布局**创建**该缓冲；在提交绘制前将 scalarParams（及运行时覆盖）**设置**进缓冲并上传到 GPU；绘制时由 Pipeline/009 **绑定**到 Shader 的对应 slot。创建、设置、绑定的详细约定见 [013-resource-data-model.md](./013-resource-data-model.md) 中「Material 的 UniformBuffer（DResource）的创建与设置」小节。

---

## 版本与迁移

- 当前格式版本：**1**
- 后续不兼容变更时递增 formatVersion，并在 002-Object 的 IVersionMigration 中实现从旧版本到新版本的迁移逻辑。

## 引用

- 与 [resource-serialization.md](./resource-serialization.md) 一致。
- 契约：[011-material-public-api.md](../../specs/_contracts/011-material-public-api.md)。
