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

## 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| ResourceId | 资源唯一标识；可寻址路径、GUID、与 Object 引用解析对接 | 与资源绑定 |
| LoadHandle | 加载请求句柄；同步/异步加载、完成回调、依赖解析 | 请求发出至完成或取消 |
| AsyncResult | 异步加载结果；完成状态、依赖解析、加载队列与优先级 | 由调用方或回调管理 |
| StreamingHandle | 流式请求句柄；按需加载、优先级、与 LOD/地形对接 | 请求有效期内 |
| Metadata | 资源元数据；格式、依赖记录、与导入管线对接 | 与资源或导入产物绑定 |

下游通过上述类型与句柄访问；纹理/网格等 GPU 资源由各模块通过句柄向 RHI/Pipeline 申请，Resource 管理加载状态与生命周期。

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
