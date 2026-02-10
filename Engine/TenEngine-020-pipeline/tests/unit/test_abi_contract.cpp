/**
 * @file test_abi_contract.cpp
 * @brief Minimal ABI contract test: CreateRenderPipeline, IRenderPipeline API, CheckWarning/CheckError, TriggerRender one frame.
 */

#include <te/pipeline/RenderPipeline.h>
#include <te/pipeline/FrameContext.h>
#include <te/pipeline/RenderingConfig.h>

int main() {
  te::pipeline::RenderingConfig config;
  config.validationLevel = te::pipeline::ValidationLevel::Debug;
  te::pipeline::CheckWarning(config, "test warning");
  te::pipeline::CheckError(config, "test error");
  config.validationLevel = te::pipeline::ValidationLevel::Resource;
  te::pipeline::CheckWarning(config, "no-op");
  te::pipeline::CheckError(config, "no-op");

  te::pipeline::RenderPipelineDesc desc;
  desc.frameInFlightCount = 2u;
  desc.device = nullptr;

  te::pipeline::IRenderPipeline* p = te::pipeline::CreateRenderPipeline(desc);
  if (!p) return 1;

  te::pipeline::FrameContext ctx;
  p->RenderFrame(ctx);
  p->TriggerRender(ctx);
  p->TickPipeline(ctx);
  (void)p->GetCurrentSlot();
  p->SetRenderingConfig(te::pipeline::RenderingConfig{});
  (void)p->GetRenderingConfig();
  (void)p->GetFrameGraph();
  p->SetFrameGraph(nullptr);
  p->SubmitLogicalCommandBuffer(nullptr);

  delete p;
  return 0;
}
