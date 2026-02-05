#include <te/pipelinecore/FrameGraph.h>
#include <te/pipelinecore/LogicalPipeline.h>
#include <te/pipelinecore/CollectPass.h>
#include <te/pipelinecore/RenderItem.h>
#include <te/pipelinecore/LogicalCommandBuffer.h>
#include <te/rendercore/types.hpp>
#include <cassert>
#include <cstdio>

int main() {
  using namespace te::pipelinecore;
  using namespace te::rendercore;

  // 1. FrameGraph + Compile
  IFrameGraph* graph = CreateFrameGraph();
  assert(graph);
  graph->AddPass("Opaque")->SetRenderType(RenderType::Opaque);
  assert(graph->Compile());

  // 2. BuildLogicalPipeline
  FrameContext ctx{};
  ILogicalPipeline* pipeline = BuildLogicalPipeline(graph, ctx);
  assert(pipeline);
  assert(pipeline->GetPassCount() == 1u);

  // 3. CollectRenderItemsParallel (首版仅清空)
  IRenderItemList* items = CreateRenderItemList();
  assert(items);
  CollectRenderItemsParallel(pipeline, ctx, items);
  assert(items->Size() == 0u);

  // 4. MergeRenderItems
  IRenderItemList* merged = CreateRenderItemList();
  IRenderItemList const* partials[] = {items};
  MergeRenderItems(partials, 1, merged);
  assert(merged->Size() == 0u);

  // 5. PrepareRenderResources (nullptr device -> InvalidHandle)
  ResultCode rc = PrepareRenderResources(merged, nullptr);
  assert(rc == ResultCode::InvalidHandle);

  // 6. ConvertToLogicalCommandBuffer
  ILogicalCommandBuffer* cmdbuf = nullptr;
  rc = ConvertToLogicalCommandBuffer(merged, pipeline, &cmdbuf);
  assert(rc == ResultCode::Success);
  assert(cmdbuf);

  DestroyLogicalCommandBuffer(cmdbuf);
  DestroyRenderItemList(merged);
  DestroyRenderItemList(items);
  DestroyLogicalPipeline(pipeline);
  DestroyFrameGraph(graph);
  std::printf("test_logical_pipeline: pass\n");
  return 0;
}
