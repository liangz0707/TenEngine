# Data Model: 019-PipelineCore

**Feature**: 019-pipelinecore-implement-full | **Date**: 2026-02-03

## 1. 核心实体

### 1.1 FrameContext

| 字段 | 类型 | 说明 |
|------|------|------|
| scene | ISceneWorld const* | 场景根；可为 nullptr |
| camera | void const* | 相机（或相机 ID）；020 定义具体类型 |
| viewport | ViewportDesc | 视口宽高、比例等 |
| frameSlotId | FrameSlotId | 当前帧 slot |
| (扩展) | — | 可由 020 在构造时填充其他字段 |

- **生命周期**: 由 020 每帧构造，传入 BuildLogicalPipeline、CollectRenderItemsParallel。
- **校验**: scene 为 nullptr 时收集阶段可产出空列表；其他字段由 020 保证有效。

### 1.2 ISceneWorld（接口）

| 方法 | 签名 | 说明 |
|------|------|------|
| QueryVisible | 见下 | 查询可见实体/渲染对象 |
| GetRoot | 见下 | 获取场景根（可选） |

- **最小接口**: `struct ISceneWorld { virtual ~ISceneWorld() = default; /* 查询可见实体等 */ };`；020/004 实现。
- **关系**: 019 定义接口；020 在 AddPass 时通过 SetScene 绑定。

### 1.3 PassGraph / IFrameGraph

| 概念 | 说明 |
|------|------|
| Pass 节点 | 每个 AddPass 产生一个节点；含 name、CullMode、RenderType、PassOutputDesc、ExecuteCallback |
| 依赖边 | 通过 DeclareRead/DeclareWrite 与 009 PassResourceDecl 对接，构建有向图 |
| 拓扑序 | Compile 时计算；环检测失败则 Compile 返回 false |

### 1.4 PassContext

| 字段/方法 | 说明 |
|-----------|------|
| GetCollectedObjects() | 返回 IRenderObjectList const*，本 Pass 收集到的物体 |
| GetCommandList() | 与 RHI ICommandList 的关联（由 Pass 执行时注入） |
| (扩展) | 可含 PassOutputDesc、当前 slot 等 |

### 1.5 RenderItem

| 字段 | 说明 |
|------|------|
| mesh | IMeshHandle const*（或 opaque） | 网格句柄 |
| material | IMaterialHandle const*（或 opaque） | 材质句柄 |
| sortKey | uint64_t | 排序 key |
| transform / bounds | 可选 | 变换、包围盒等 |
| (扩展) | 由 Collect 阶段填充 |

### 1.6 ILogicalCommandBuffer

- **含义**: CPU 侧逻辑命令序列（Draw、Bind、Barrier 等），非 GPU 原生命令。
- **格式**: 符合 pipeline-to-rci.md；020 提交给 RHI 时由 RHI 转换为 ICommandList 录制。
- **生命周期**: ConvertToLogicalCommandBuffer 产出；由 020 持有并在帧内提交后释放。

### 1.7 ResourceHandle / PassHandle

- **来源**: 009-RenderCore 定义；019 使用于 DeclareRead/DeclareWrite 与资源生命周期管理。
- **关系**: PassHandle 标识 Pass；ResourceHandle 标识管线内资源；019 与 009 Pass 协议对接。

## 2. 状态转换

### 2.1 Pass 图

```
[构建] AddPass × N, SetXxx, SetExecuteCallback
    → [编译] Compile() → 成功则产出可执行图 / 失败（环等）
    → [执行] Execute(graph, ctx) → 按拓扑序调用各 Pass 回调
```

### 2.2 多线程阶段

| 阶段 | 线程 | 输入 | 输出 |
|------|------|------|------|
| Build | B | IFrameGraph, FrameContext | ILogicalPipeline |
| Collect | C | ILogicalPipeline, FrameContext | IRenderItemList（合并后） |
| Prepare + Convert | D | IRenderItemList, ILogicalPipeline, IDevice | PrepareRenderResources 结果；ILogicalCommandBuffer |

## 3. 校验规则

- Compile 前：Pass 须至少设置 ExecuteCallback。
- Compile：若存在环或非法依赖，返回 false。
- PrepareRenderResources / ConvertToLogicalCommandBuffer：仅在线程 D 调用；遇 RHI 失败返回 ResultCode。
- FrameSlotId：有效范围 [0, frameInFlightCount)。
