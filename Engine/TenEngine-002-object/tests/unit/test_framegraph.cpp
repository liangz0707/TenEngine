#include <te/pipelinecore/FrameGraph.h>
#include <cassert>
#include <cstdio>

int main() {
  using namespace te::pipelinecore;
  IFrameGraph* graph = CreateFrameGraph();
  assert(graph);

  IPassBuilder* pb = graph->AddPass("Opaque");
  assert(pb);
  pb->SetCullMode(CullMode::FrustumCull);
  pb->SetRenderType(RenderType::Opaque);

  bool compileOk = graph->Compile();
  assert(compileOk);

  DestroyFrameGraph(graph);
  std::printf("test_framegraph: pass\n");
  return 0;
}
