# Quickstart: 019-PipelineCore

**Feature**: 019-pipelinecore-implement-full | **Date**: 2026-02-03

## 1. 模块简介

019-PipelineCore 提供 RDG 风格 Pass 图协议、命令缓冲约定、资源生命周期管理与 RHI 提交约定。对应 Unreal RDG、Unity SRP Core。

## 2. 依赖

- **008-RHI**：IDevice、ICommandList、CreateBuffer/CreateTexture、ResourceBarrier、Submit
- **009-RenderCore**：PassHandle、ResourceHandle、DeclareRead/DeclareWrite、ResultCode

## 3. 典型流程

**命名空间**：`te::pipelinecore`；头文件 `#include <te/pipelinecore/FrameGraph.h>` 等。

### 3.1 FrameGraph 构建与执行

```cpp
#include <te/pipelinecore/FrameGraph.h>
// using namespace te::pipelinecore;  // 或 te::pipelinecore::IFrameGraph 等

// 1. 创建 FrameGraph
te::pipelinecore::IFrameGraph* graph = CreateFrameGraph();

// 2. 添加 Pass
auto* opaque = graph->AddPass("Opaque");
opaque->SetScene(scene);
opaque->SetCullMode(te::pipelinecore::CullMode::FrustumCull);
opaque->SetRenderType(te::pipelinecore::RenderType::Opaque);
opaque->SetExecuteCallback([](te::pipelinecore::PassContext& ctx, te::rhi::ICommandList* cmd) {
    auto* objects = ctx.GetCollectedObjects();
    // 录制 DrawCall...
});

// 3. 编译
if (!graph->Compile()) { /* 处理失败 */ }

// 4. 执行（由 020-Pipeline 驱动）
ExecuteFrameGraph(graph, frameContext);
```

### 3.2 多线程流水线

```cpp
// 线程 B：构建逻辑管线
te::pipelinecore::FrameContext ctx = BuildFrameContext(...);
te::pipelinecore::ILogicalPipeline* pipeline = BuildLogicalPipeline(graph, ctx);

// 线程 C：收集 RenderItem
te::pipelinecore::IRenderItemList* items = CreateRenderItemList();
CollectRenderItemsParallel(pipeline, ctx, items);
// 可选：MergeRenderItems(partial_lists, count, merged);

// 线程 D：准备资源并产出逻辑命令缓冲
te::rendercore::ResultCode r = PrepareRenderResources(items, device);
if (r != te::rendercore::ResultCode::Success) { /* 处理错误 */ }
te::pipelinecore::ILogicalCommandBuffer* logicalCB = nullptr;
r = ConvertToLogicalCommandBuffer(items, pipeline, &logicalCB);
// 020 将 logicalCB 交给 RHI 录制并 Submit
```

## 4. 线程约束

- **线程 D**：PrepareRenderResources、PrepareRenderMaterial、PrepareRenderMesh、ConvertToLogicalCommandBuffer 必须在线程 D（GPU/Device 线程）调用。
- 线程 B/C 可并行；线程 D 与 RHI 调用须同线程。

## 4.1 Slot 语义

- `FrameSlotId` 范围 `[0, frameInFlightCount)`，由 `PipelineConfig.frameInFlightCount` 决定。
- `FrameContext.frameSlotId` 表示当前帧所属 slot；020 每帧构造时设置。
- 与 RHI 协同：`waitForSlot(slotId)` 等待该 slot 上帧完成；`getCommandListForSlot(slotId)` 获取本帧命令列表。RHI 接口见 008-rhi-public-api。

## 5. 错误处理

- PrepareRenderResources、ConvertToLogicalCommandBuffer 返回 `ResultCode`；调用方决定跳过/重试/中止。
- Compile 遇环返回 false。

## 6. Profiling

启用 `TE_PIPELINECORE_PROFILING` 时可获取 Pass 执行耗时、Compile 耗时等指标。

```cpp
#define TE_PIPELINECORE_PROFILING 1
#include <te/pipelinecore/Profiling.h>
// 注册 Compile 耗时回调
te::pipelinecore::g_onCompileProfiling = [](uint64_t micros) { /* ... */ };
```

## 7. 线程 D 约束（详细）

- **PrepareRenderResources**、**PrepareRenderMaterial**、**PrepareRenderMesh**、**ConvertToLogicalCommandBuffer** 必须在线程 D（GPU/Device 专用线程）调用。
- 线程 D 与 RHI `IDevice`、`ICommandList` 的创建与提交须在同一线程，避免多线程竞态。
