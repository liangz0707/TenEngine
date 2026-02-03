# Asset 相关文档

本目录集中存放**与 Asset（资源格式、管线、序列化）相关**的文档，其余目录保持干净。

## 文档索引

| 文档 | 说明 |
|------|------|
| [resource-serialization.md](./resource-serialization.md) | 资源序列化规范：可序列化资源类型与归属、Import/Load/Save、磁盘目录、注册方式、版本管理、GUID 与引用关系 |
| [resource-loading-flow.md](./resource-loading-flow.md) | 各模块与 Resource 的资源加载流程：013 按类型加载步骤，011/012/004/005/020 与 013 的衔接 |
| [011-material-data-model.md](./011-material-data-model.md) | 材质资源格式（MaterialAssetDesc，.material） |
| [013-resource-data-model.md](./013-resource-data-model.md) | 模型/贴图/网格数据模型（Model、Texture、Mesh）；Texture 方案 B、Mesh 方案二；RResource 与 DResource 流程、DResource 异步按需创建与设备层兼容 |
| [004-scene-data-model.md](./004-scene-data-model.md) | 场景/关卡资源格式（LevelAssetDesc、SceneNodeDesc，.level/.scene） |
