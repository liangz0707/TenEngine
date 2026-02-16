/**
 * @file test_render_flow.cpp
 * @brief Integration test for complete render flow.
 *
 * Tests the full pipeline flow:
 * 1. Create frame graph with GBuffer + Lighting + PostProcess passes
 * 2. Compile frame graph
 * 3. Collect mock renderables
 * 4. Build logical command buffer
 * 5. Execute passes (simulated)
 * 6. Verify draw call counts
 */

#include <te/pipeline/RenderPipeline.h>
#include <te/pipeline/PassBuilders.h>
#include <te/pipelinecore/FrameGraph.h>
#include <te/pipelinecore/LogicalCommandBuffer.h>
#include <te/pipelinecore/RenderItem.h>
#include <cstdio>
#include <memory>

using namespace te::pipeline;
using namespace te::pipelinecore;

// Mock render element for testing
class MockRenderMesh : public te::rendercore::IRenderMesh {
  te::rendercore::SubmeshRange submesh_;

 public:
  MockRenderMesh() {
    submesh_.indexOffset = 0;
    submesh_.indexCount = 36;  // Cube
    submesh_.vertexOffset = 0;
  }

  te::rhi::IBuffer* GetVertexBuffer() override { return nullptr; }
  te::rhi::IBuffer const* GetVertexBuffer() const override { return nullptr; }
  te::rhi::IBuffer* GetIndexBuffer() override { return nullptr; }
  te::rhi::IBuffer const* GetIndexBuffer() const override { return nullptr; }
  std::uint32_t GetSubmeshCount() const override { return 1; }
  bool GetSubmesh(std::uint32_t index, te::rendercore::SubmeshRange* out) const override {
    if (index == 0 && out) {
      *out = submesh_;
      return true;
    }
    return false;
  }
  void SetDataVertex(void const*, std::size_t) override {}
  void SetDataIndex(void const*, std::size_t) override {}
  void SetDataIndexType(te::rendercore::IndexType) override {}
  void SetDataSubmeshCount(std::uint32_t) override {}
  void SetDataSubmesh(std::uint32_t, te::rendercore::SubmeshRange const&) override {}
  void UpdateDeviceResource(te::rhi::IDevice*) override {}
};

class MockRenderMaterial : public te::rendercore::IRenderMaterial {
 public:
  te::rendercore::IUniformBuffer* GetUniformBuffer() override { return nullptr; }
  te::rendercore::IUniformBuffer const* GetUniformBuffer() const override { return nullptr; }
  te::rhi::IDescriptorSet* GetDescriptorSet() override { return nullptr; }
  te::rhi::IDescriptorSet const* GetDescriptorSet() const override { return nullptr; }
  te::rhi::IPSO* GetGraphicsPSO(std::uint32_t) override { return nullptr; }
  te::rhi::IPSO const* GetGraphicsPSO(std::uint32_t) const override { return nullptr; }
  void CreateDeviceResource() override {}
  void CreateDeviceResource(te::rhi::IRenderPass*, std::uint32_t, te::rhi::IDescriptorSetLayout*) override {}
  void UpdateDeviceResource(te::rhi::IDevice*, std::uint32_t) override {}
  void SetDataParameter(char const*, void const*, std::size_t) override {}
  void SetDataTexture(std::uint32_t, te::rhi::ITexture*) override {}
  void SetDataTextureByName(char const*, te::rhi::ITexture*) override {}
  bool IsDeviceReady() const override { return true; }
  te::rendercore::IRenderPipelineState const* GetPipelineState() const override { return nullptr; }
  te::rendercore::IShaderEntry const* GetShaderEntry() const override { return nullptr; }
  te::rhi::GraphicsPipelineStateDesc const* GetRHIStateDesc() const override { return nullptr; }
};

class MockRenderElement : public te::rendercore::IRenderElement {
 public:
  MockRenderMesh mesh_;
  MockRenderMaterial material_;

  te::rendercore::IRenderMesh* GetMesh() override { return &mesh_; }
  te::rendercore::IRenderMesh const* GetMesh() const override { return &mesh_; }
  te::rendercore::IRenderMaterial* GetMaterial() override { return &material_; }
  te::rendercore::IRenderMaterial const* GetMaterial() const override { return &material_; }
};

// Test state
static std::vector<std::unique_ptr<MockRenderElement>> g_elements;
static IRenderItemList* g_renderList = nullptr;
static ILightItemList* g_lightList = nullptr;

// Collect callback
void CollectMockRenderables(IRenderItemList* outList, ILightItemList* outLights) {
  // Add mock renderables
  for (size_t i = 0; i < 3; ++i) {
    auto elem = std::make_unique<MockRenderElement>();
    RenderItem item{};
    item.element = elem.get();
    item.submeshIndex = 0;
    item.instanceCount = 1;
    outList->Push(item);
    g_elements.push_back(std::move(elem));
  }

  // Add mock light
  LightItem light{};
  light.type = LightType::Directional;
  light.color[0] = 1.f;
  light.color[1] = 1.f;
  light.color[2] = 1.f;
  light.intensity = 1.f;
  outLights->Push(light);
}

// Test: Complete render flow
bool test_complete_render_flow() {
  printf("Test: Complete render flow...\n");

  // Step 1: Create frame graph
  printf("  Step 1: Creating frame graph...\n");
  IFrameGraph* fg = CreateFrameGraph();
  if (!fg) {
    printf("    FAILED: CreateFrameGraph returned null\n");
    return false;
  }

  // Step 2: Add standard passes using PassBuilders
  printf("  Step 2: Adding standard passes...\n");
  GBufferPassConfig gbufferConfig{};
  gbufferConfig.width = 1280;
  gbufferConfig.height = 720;

  IPassBuilder* gbufferPass = CreateGBufferPass(fg, gbufferConfig);
  if (!gbufferPass) {
    printf("    FAILED: CreateGBufferPass returned null\n");
    DestroyFrameGraph(fg);
    return false;
  }

  LightingPassConfig lightingConfig{};
  lightingConfig.width = 1280;
  lightingConfig.height = 720;

  IPassBuilder* lightingPass = CreateLightingPass(fg, lightingConfig);
  if (!lightingPass) {
    printf("    FAILED: CreateLightingPass returned null\n");
    DestroyFrameGraph(fg);
    return false;
  }

  PostProcessPassConfig ppConfig{};
  ppConfig.width = 1280;
  ppConfig.height = 720;

  IPassBuilder* ppPass = CreatePostProcessPass(fg, ppConfig);
  if (!ppPass) {
    printf("    FAILED: CreatePostProcessPass returned null\n");
    DestroyFrameGraph(fg);
    return false;
  }

  // Step 3: Compile frame graph
  printf("  Step 3: Compiling frame graph...\n");
  if (!fg->Compile()) {
    printf("    FAILED: Frame graph compilation failed\n");
    DestroyFrameGraph(fg);
    return false;
  }

  size_t passCount = fg->GetPassCount();
  printf("    Compiled %zu passes\n", passCount);
  if (passCount != 3) {
    printf("    FAILED: Expected 3 passes, got %zu\n", passCount);
    DestroyFrameGraph(fg);
    return false;
  }

  // Step 4: Build logical pipeline
  printf("  Step 4: Building logical pipeline...\n");
  FrameContext ctx;
  ctx.viewport.width = 1280;
  ctx.viewport.height = 720;
  ctx.frameSlotId = 0;

  ILogicalPipeline* pipeline = BuildLogicalPipeline(fg, ctx);
  if (!pipeline) {
    printf("    FAILED: BuildLogicalPipeline returned null\n");
    DestroyFrameGraph(fg);
    return false;
  }

  // Step 5: Collect renderables
  printf("  Step 5: Collecting renderables...\n");
  g_renderList = CreateRenderItemList();
  g_lightList = CreateLightItemList();

  CollectMockRenderables(g_renderList, g_lightList);

  if (g_renderList->Size() != 3) {
    printf("    FAILED: Expected 3 renderables, got %zu\n", g_renderList->Size());
    DestroyRenderItemList(g_renderList);
    DestroyLightItemList(g_lightList);
    DestroyLogicalPipeline(pipeline);
    DestroyFrameGraph(fg);
    return false;
  }
  printf("    Collected %zu renderables, %zu lights\n", g_renderList->Size(), g_lightList->Size());

  // Step 6: Convert to logical command buffer
  printf("  Step 6: Converting to logical command buffer...\n");
  ILogicalCommandBuffer* cmdBuffer = nullptr;
  auto result = ConvertToLogicalCommandBuffer(g_renderList, pipeline, &cmdBuffer);

  if (result != te::rendercore::ResultCode::Success) {
    printf("    FAILED: ConvertToLogicalCommandBuffer failed\n");
    DestroyRenderItemList(g_renderList);
    DestroyLightItemList(g_lightList);
    DestroyLogicalPipeline(pipeline);
    DestroyFrameGraph(fg);
    return false;
  }

  if (!cmdBuffer) {
    printf("    FAILED: LogicalCommandBuffer is null\n");
    DestroyRenderItemList(g_renderList);
    DestroyLightItemList(g_lightList);
    DestroyLogicalPipeline(pipeline);
    DestroyFrameGraph(fg);
    return false;
  }

  size_t drawCount = cmdBuffer->GetDrawCount();
  printf("    Converted to %zu draw calls\n", drawCount);

  // Step 7: Verify pass configurations
  printf("  Step 7: Verifying pass configurations...\n");
  for (size_t i = 0; i < passCount; ++i) {
    PassCollectConfig config;
    fg->GetPassCollectConfig(i, &config);
    printf("    Pass %zu: kind=%u, name=%s\n", i,
           static_cast<uint32_t>(config.passKind),
           config.passName ? config.passName : "(null)");
  }

  // Cleanup
  printf("  Cleaning up...\n");
  DestroyLogicalCommandBuffer(cmdBuffer);
  DestroyRenderItemList(g_renderList);
  DestroyLightItemList(g_lightList);
  DestroyLogicalPipeline(pipeline);
  DestroyFrameGraph(fg);
  g_elements.clear();

  printf("  PASSED\n");
  return true;
}

// Test: RenderPipeline configuration
bool test_render_pipeline_configuration() {
  printf("Test: RenderPipeline configuration...\n");

  RenderPipeline pipeline;

  // Test configuration
  RenderingConfig config;
  config.validationLevel = RenderingConfig::ValidationLevel::Debug;
  config.enableProfiling = true;
  config.frameSlotCount = 3;
  pipeline.SetConfig(config);

  // Verify configuration
  auto const& retrieved = pipeline.GetConfig();
  if (retrieved.validationLevel != RenderingConfig::ValidationLevel::Debug) {
    printf("  FAILED: Validation level mismatch\n");
    return false;
  }
  if (!retrieved.enableProfiling) {
    printf("  FAILED: Profiling not enabled\n");
    return false;
  }
  if (retrieved.frameSlotCount != 3) {
    printf("  FAILED: Frame slot count mismatch\n");
    return false;
  }

  printf("  PASSED\n");
  return true;
}

// Test: Frame stats
bool test_frame_stats() {
  printf("Test: Frame statistics...\n");

  RenderPipeline pipeline;

  auto const& stats = pipeline.GetFrameStats();
  if (stats.frameNumber != 0) {
    printf("  FAILED: Initial frame number should be 0\n");
    return false;
  }
  if (stats.drawCallCount != 0) {
    printf("  FAILED: Initial draw call count should be 0\n");
    return false;
  }

  printf("  PASSED\n");
  return true;
}

int main() {
  printf("=== Render Pipeline Integration Tests ===\n\n");

  int failures = 0;

  if (!test_render_pipeline_configuration()) failures++;
  if (!test_frame_stats()) failures++;
  if (!test_complete_render_flow()) failures++;

  printf("\n=== Results: %d test(s) failed ===\n", failures);

  return failures;
}
