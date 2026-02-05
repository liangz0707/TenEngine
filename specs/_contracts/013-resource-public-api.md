# 契约：013-Resource 模块对外 API

## 适用模块

- **实现方**：013-Resource（L2；资源导入、同步/异步加载、卸载、流式、可寻址；唯一 Load 入口；统一接口与缓存针对 IResource*；不创建 DResource）
- **对应规格**：`docs/module-specs/013-resource.md`
- **依赖**：001-Core、002-Object、028-Texture

## 消费者

- 010-Shader、011-Material、012-Mesh、028-Texture、029-World（向 013 注册 Create*/Loader；029 在 Level 加载时调 004 CreateSceneFromDesc）、016-Audio、020-Pipeline、022-2D、023-Terrain、024-Editor（调用 Load 等获取资源）

## 能力列表

### 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| ResourceId / GUID | 资源全局唯一标识；FResource 间引用用 GUID；可寻址路径、与 Object 引用解析对接 | 与资源绑定 |
| FResource | 硬盘形态；引用其它资源仅通过 GUID | 与磁盘/包绑定 |
| RResource | 运行时/内存形态；通过指针引用其它 RResource；013 仅创建 RResource，不创建 DResource；DResource 槽位由 011/012/028/008 在 EnsureDeviceResources 时填充 | 加载后直至卸载 |
| DResource | GPU 形态；由 008/011/012/028 在 EnsureDeviceResources 时创建，对 013 不可见 | 由 RResource 管理 |
| LoadHandle | 加载请求句柄；同步/异步、完成回调、依赖解析 | 请求发出至完成或取消 |
| AsyncResult | 异步加载结果；完成状态、依赖解析、队列与优先级 | 由调用方或回调管理 |
| StreamingHandle | 流式请求句柄；按需加载、优先级、与 LOD/地形对接 | 请求有效期内 |
| Metadata | 资源元数据；格式、依赖记录、与导入管线对接 | 与资源或导入产物绑定 |
| IResource / IModelResource | 可加载资产统一接口；IModelResource 聚合 IMeshResource*、IMaterialResource*、submeshMaterialIndices | 013 内部或缓存持有；下游经 ResourceId/句柄解析使用 |

### 能力（提供方保证）

| 序号 | 能力 | 说明 |
|------|------|------|
| 1 | Import | RegisterImporter、DetectFormat、Convert、Metadata、Dependencies；导入器注册与依赖记录 |
| 2 | Load | LoadSync、LoadAsync、ResolveDependencies、Queue、Priority；013 为唯一加载入口；依赖主动递归加载；通过注册调用 010/011/012/028/029 的 Create*/Loader；Load 阶段不创建 DResource |
| 3 | EnsureDeviceResources | EnsureDeviceResourcesAsync、EnsureDeviceResources；由下游触发，011/012/028/008 在此时创建 DResource，013 不参与 |
| 4 | Unload | Release、GC、UnloadPolicy；与各模块资源句柄协调 |
| 5 | Streaming | RequestStreaming、SetPriority；与 LOD/Terrain 按需加载对接 |
| 6 | Addressing | ResourceId、GUID、Address、BundleMapping；可寻址路径与打包对应 |

*Desc 归属：013 拥有 ModelAssetDesc、TextureAssetDesc；ShaderAssetDesc→010，MaterialAssetDesc→011，LevelAssetDesc/SceneNodeDesc→029，MeshAssetDesc→012。

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。

## 约束

- 须在 Core、Object、028-Texture 初始化之后使用；010/011/012/028/029 须已向 013 注册 Create*/Loader。013 不创建、不持有、不调用 008。资源引用格式须与 Object 序列化约定一致。句柄释放顺序须与卸载策略协调。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 013-Resource 契约 |
| 2026-02-05 | 统一目录；能力列表用表格；去除冗余流程说明与 ABI 引用 |
