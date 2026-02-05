# 004-Scene 资源数据模型

本文档定义磁盘上**场景/关卡**资源的引擎自有格式（FResource 描述）。该描述为 **002-Object 可序列化类型**，引用均为 **GUID**；013-Resource 或 004-Scene 反序列化后由 004 创建场景图与层级。

- **文件扩展名**：`.level` 或 `.scene`
- **格式归属**：004-Scene
- **序列化**：002-Object（ISerializer + 已注册类型）

---

## LevelAssetDesc

| 字段 | 类型 | 说明 |
|------|------|------|
| formatVersion | uint32 | 格式版本，用于迁移 |
| debugDescription | string | 明文描述，用于 Debug（日志/dump/编辑器展示）；UTF-8 |
| rootNodes | array of SceneNodeDesc | 根节点列表；顺序与层级由 children 递归定义 |
| defaultWorldSettings | 可选 | 光照、环境等关卡级设置，按需扩展 |

---

## SceneNodeDesc

描述场景图中一个节点：变换、层级、以及可选的实体/模型引用。

| 字段 | 类型 | 说明 |
|------|------|------|
| name | string | 节点名 |
| debugDescription | string | 可选；明文描述，用于 Debug（日志/dump/编辑器展示）；UTF-8 |
| localTransform | TransformDesc | 局部变换（位置、旋转、缩放） |
| children | array of SceneNodeDesc | 子节点，递归构成完整树 |
| modelGuid | GUID | 可选；挂载的 Model 资源，由 013-Resource 加载 |
| entityPrefabGuid | GUID | 可选；挂载的预制体，与 005-Entity、013 预制体资源对接 |
| components | array of ComponentData | 可选；005-Entity 组件数据（类型 + 可序列化属性） |
| active | bool | 是否激活，默认 true |

**说明**：节点可只挂 Model（modelGuid）、只挂预制体（entityPrefabGuid）、或仅作为空节点（仅 name + transform + children）。components 与 Entity 组件系统对接时，类型与属性名与 002-Object 反射一致。

### TransformDesc

| 字段 | 类型 | 说明 |
|------|------|------|
| position | float3 | 局部位置 |
| rotation | float4 (quat) 或 float3 (euler) | 局部旋转 |
| scale | float3 | 局部缩放 |

（与 Core.Math 或 004-Scene 使用的变换表示一致；序列化时由实现约定。）

### ComponentData

| 字段 | 类型 | 说明 |
|------|------|------|
| typeId 或 typeName | TypeId / string | 组件类型，与 002-Object 注册一致 |
| properties | 键值或 PropertyBag | 可序列化属性，与反射一致 |

---

## 版本与迁移

- 当前格式版本：**1**
- 不兼容变更时递增 formatVersion，并在 002-Object 的 IVersionMigration 中实现迁移。

## 引用

- 与 [resource-serialization.md](./resource-serialization.md) 一致。
- 契约：[004-scene-public-api.md](../../specs/_contracts/004-scene-public-api.md)。
