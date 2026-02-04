# 契约：013-Resource 模块对外 API

## 适用模块

- **实现方**：**013-Resource**（资源导入、加载与生命周期）
- **对应规格**：`docs/module-specs/013-resource.md`
- **依赖**：001-Core（001-core-public-api）、002-Object（002-object-public-api）

## 消费者（T0 下游）

- 016-Audio（资源加载、句柄）
- 020-Pipeline（纹理/网格等资源加载、流式）
- 022-2D、023-Terrain（资源加载、流式、可寻址）
- 024-Editor（资源浏览、导入、引用解析）

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。
- 当前契约版本：（由实现或计划阶段填写）

## 资源三态与引用方式

为便于管理，所有资源可有三种形态：

| 形态 | 语义 | 引用方式 | 使用场景 |
|------|------|----------|----------|
| **FResource** | 硬盘形态 | 在硬盘上；通过**全局唯一 GUID** 引用其他资源 | **硬盘加载使用 FResource**；FResource 内只存 GUID，不存指针 |
| **RResource** | 运行时/内存形态 | 根据 FResource 的引用，通过**指针**引用其他 RResource；**DResource 直接保存在 RResource 内部** | **内存引用使用 RResource**；RResource 持有对其它 RResource 的指针，并内部持有 DResource |
| **DResource** | GPU 形态 | 不单独作为跨对象引用；**保存在 RResource 内部**，由 RResource 管理 | **GPU 类型资源**；生命周期与绑定由 RResource 负责 |

部分资源可能**只存在某一形态**（例如仅 FResource、仅 RResource、或仅作为 RResource 内的 DResource）。

## 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| ResourceId / GUID | 资源全局唯一标识；FResource 间引用使用 GUID；可寻址路径、与 Object 引用解析对接 | 与资源绑定 |
| FResource | 硬盘上的资源表示；引用其它资源仅通过 GUID | 与磁盘/包绑定 |
| RResource | 内存中的资源表示；通过指针引用其它 RResource；内部持有 DResource | 加载后直至卸载 |
| DResource | GPU 资源；保存在 RResource 内部，不单独暴露为引用对象 | 由 RResource 管理 |
| LoadHandle | 加载请求句柄；同步/异步加载、完成回调、依赖解析；从 FResource 加载得到 RResource | 请求发出至完成或取消 |
| AsyncResult | 异步加载结果；完成状态、依赖解析、加载队列与优先级 | 由调用方或回调管理 |
| StreamingHandle | 流式请求句柄；按需加载、优先级、与 LOD/地形对接 | 请求有效期内 |
| Metadata | 资源元数据；格式、依赖记录、与导入管线对接 | 与资源或导入产物绑定 |

下游通过上述类型与句柄访问；硬盘加载使用 FResource（GUID 引用）；内存引用使用 RResource（指针引用）；DResource 由 RResource 内部持有。

## 能力列表（提供方保证）

1. **Import**：RegisterImporter、DetectFormat、Convert、Metadata、Dependencies；导入器注册与依赖记录。
2. **Load**：LoadSync、LoadAsync、ResolveDependencies、Queue、Priority；同步/异步加载与依赖解析。
3. **Unload**：Release、GC、UnloadPolicy；与各模块资源句柄协调、卸载策略。
4. **Streaming**：RequestStreaming、SetPriority；与 LOD/Terrain 等按需加载对接。
5. **Addressing**：ResourceId、GUID、Address、BundleMapping；可寻址路径与打包/Bundle 对应。

## 调用顺序与约束

- 须在 Core、Object 初始化之后使用；资源引用（GUID/ObjectRef）格式须与 Object 序列化约定一致。
- 各模块资源句柄的释放顺序须与 Resource 卸载策略协调，避免悬空引用。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 按 013-Resource 模块规格与依赖表新增契约；类型与能力与 docs/module-specs/013-resource.md 一致 |
| 2026-01-29 | 契约更新由 plan 013-resource-fullversion-001 同步 |
