/**
 * @file test_render_pipeline.cpp
 * @brief Test RenderPipeline basic flow without actual GPU device.
 *
 * This test verifies the logical flow of the render pipeline:
 * - Frame graph creation and compilation
 * - Render item collection
 * - Logical command buffer conversion
 */

#include <te/pipeline/RenderPipeline.h>
#include <te/pipelinecore/FrameGraph.h>
#include <te/pipelinecore/LogicalCommandBuffer.h>
#include <te/pipelinecore/RenderItem.h>
#include <cstdio>

using namespace te::pipeline;
using namespace te::pipelinecore;

// Mock render element for testing
class MockRenderMesh : public te::rendercore::IRenderMesh {
 public:
  te::rhi::IBuffer* GetVertexBuffer() override { return nullptr; }
  te::rhi::IBuffer const* GetVertexBuffer() const override { return nullptr; }
  te::rhi::IBuffer* GetIndexBuffer() override { return nullptr; }
  te::rhi::IBuffer const* GetIndexBuffer() const override { return nullptr; }
  std::uint32_t GetSubmeshCount() const override { return 1; }
  bool GetSubmesh(std::uint32_t index, te::rendercore::SubmeshRange* out) const override {
    if (index == 0 && out) {
      out->indexOffset = 0;
      out->indexCount = 36;  // Cube indices
      out->vertexOffset = 0;
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

  // IShadingState interface
  te::rendercore::IRenderPipelineState const* GetPipelineState() const override { return nullptr; }
  te::rendercore::IShaderEntry const* GetShaderEntry() const override { return nullptr; }

  // IRenderPipelineState interface
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

// Test 1: Frame graph creation and compilation
bool test_frame_graph() {
  printf("Test 1: Frame graph creation and compilation...\n");

  IFrameGraph* fg = CreateFrameGraph();
  if (!fg) {
    printf("  FAILED: CreateFrameGraph returned null\n");
    return false;
  }

  // Add a simple scene pass
  IPassBuilder* pass = fg->AddPass("TestPass", PassKind::Scene);
  if (!pass) {
    printf("  FAILED: AddPass returned null\n");
    DestroyFrameGraph(fg);
    return false;
  }

  PassOutputDesc output{};
  output.width = 800;
  output.height = 600;
  output.colorAttachmentCount = 1;
  pass->SetOutput(output);

  // Compile
  if (!fg->Compile()) {
    printf("  FAILED: Frame graph compilation failed\n");
    DestroyFrameGraph(fg);
    return false;
  }

  if (fg->GetPassCount() != 1) {
    printf("  FAILED: Expected 1 pass, got %zu\n", fg->GetPassCount());
    DestroyFrameGraph(fg);
    return false;
  }

  DestroyFrameGraph(fg);
  printf("  PASSED\n");
  return true;
}

// Test 2: Render item list operations
bool test_render_item_list() {
  printf("Test 2: Render item list operations...\n");

  IRenderItemList* list = CreateRenderItemList();
  if (!list) {
    printf("  FAILED: CreateRenderItemList returned null\n");
    return false;
  }

  if (list->Size() != 0) {
    printf("  FAILED: Expected empty list, got size %zu\n", list->Size());
    DestroyRenderItemList(list);
    return false;
  }

  // Add an item
  RenderItem item{};
  MockRenderElement element;
  item.element = &element;
  item.submeshIndex = 0;
  list->Push(item);

  if (list->Size() != 1) {
    printf("  FAILED: Expected size 1, got %zu\n", list->Size());
    DestroyRenderItemList(list);
    return false;
  }

  RenderItem const* retrieved = list->At(0);
  if (!retrieved || retrieved->element != &element) {
    printf("  FAILED: Retrieved item mismatch\n");
    DestroyRenderItemList(list);
    return false;
  }

  list->Clear();
  if (list->Size() != 0) {
    printf("  FAILED: Clear failed\n");
    DestroyRenderItemList(list);
    return false;
  }

  DestroyRenderItemList(list);
  printf("  PASSED\n");
  return true;
}

// Test 3: Logical command buffer conversion
bool test_logical_command_buffer() {
  printf("Test 3: Logical command buffer conversion...\n");

  IRenderItemList* list = CreateRenderItemList();
  IFrameGraph* fg = CreateFrameGraph();

  // Add a pass
  fg->AddPass("TestPass", PassKind::Scene);
  fg->Compile();

  // Create frame context
  FrameContext ctx;
  ctx.viewport.width = 800;
  ctx.viewport.height = 600;

  // Build logical pipeline
  ILogicalPipeline* pipeline = BuildLogicalPipeline(fg, ctx);
  if (!pipeline) {
    printf("  FAILED: BuildLogicalPipeline returned null\n");
    DestroyFrameGraph(fg);
    DestroyRenderItemList(list);
    return false;
  }

  // Add render items
  MockRenderElement element1, element2;
  RenderItem item1{}, item2{};
  item1.element = &element1;
  item1.submeshIndex = 0;
  item2.element = &element2;
  item2.submeshIndex = 0;
  list->Push(item1);
  list->Push(item2);

  // Convert to logical command buffer
  ILogicalCommandBuffer* cmdBuffer = nullptr;
  te::rendercore::ResultCode result = ConvertToLogicalCommandBuffer(list, pipeline, &cmdBuffer);

  if (result != te::rendercore::ResultCode::Success) {
    printf("  FAILED: ConvertToLogicalCommandBuffer failed with code %u\n",
           static_cast<uint32_t>(result));
    DestroyLogicalPipeline(pipeline);
    DestroyFrameGraph(fg);
    DestroyRenderItemList(list);
    return false;
  }

  if (!cmdBuffer) {
    printf("  FAILED: LogicalCommandBuffer is null\n");
    DestroyLogicalPipeline(pipeline);
    DestroyFrameGraph(fg);
    DestroyRenderItemList(list);
    return false;
  }

  if (cmdBuffer->GetDrawCount() != 2) {
    printf("  FAILED: Expected 2 draws, got %zu\n", cmdBuffer->GetDrawCount());
    DestroyLogicalCommandBuffer(cmdBuffer);
    DestroyLogicalPipeline(pipeline);
    DestroyFrameGraph(fg);
    DestroyRenderItemList(list);
    return false;
  }

  // Verify draw data
  LogicalDraw draw;
  cmdBuffer->GetDraw(0, &draw);
  if (draw.element != &element1) {
    printf("  FAILED: Draw 0 element mismatch\n");
    DestroyLogicalCommandBuffer(cmdBuffer);
    DestroyLogicalPipeline(pipeline);
    DestroyFrameGraph(fg);
    DestroyRenderItemList(list);
    return false;
  }

  printf("  PASSED (2 draws converted successfully)\n");
  DestroyLogicalCommandBuffer(cmdBuffer);
  DestroyLogicalPipeline(pipeline);
  DestroyFrameGraph(fg);
  DestroyRenderItemList(list);
  return true;
}

// Test 4: Light item list
bool test_light_item_list() {
  printf("Test 4: Light item list operations...\n");

  ILightItemList* list = CreateLightItemList();
  if (!list) {
    printf("  FAILED: CreateLightItemList returned null\n");
    return false;
  }

  LightItem light{};
  light.type = LightType::Directional;
  light.color[0] = 1.0f;
  light.color[1] = 1.0f;
  light.color[2] = 1.0f;
  light.intensity = 1.0f;
  list->Push(light);

  if (list->Size() != 1) {
    printf("  FAILED: Expected size 1, got %zu\n", list->Size());
    DestroyLightItemList(list);
    return false;
  }

  LightItem const* retrieved = list->At(0);
  if (!retrieved || retrieved->type != LightType::Directional) {
    printf("  FAILED: Retrieved light mismatch\n");
    DestroyLightItemList(list);
    return false;
  }

  DestroyLightItemList(list);
  printf("  PASSED\n");
  return true;
}

// Test 5: RenderPipeline configuration
bool test_render_pipeline_config() {
  printf("Test 5: RenderPipeline configuration...\n");

  RenderPipeline pipeline;

  RenderingConfig config;
  config.validationLevel = RenderingConfig::ValidationLevel::Debug;
  config.enableProfiling = true;
  config.frameSlotCount = 3;  // Triple buffering

  pipeline.SetConfig(config);

  RenderingConfig const& retrieved = pipeline.GetConfig();
  if (retrieved.validationLevel != RenderingConfig::ValidationLevel::Debug ||
      !retrieved.enableProfiling ||
      retrieved.frameSlotCount != 3) {
    printf("  FAILED: Config mismatch\n");
    return false;
  }

  printf("  PASSED\n");
  return true;
}

int main() {
  printf("=== RenderPipeline Unit Tests ===\n\n");

  int failures = 0;

  if (!test_frame_graph()) failures++;
  if (!test_render_item_list()) failures++;
  if (!test_logical_command_buffer()) failures++;
  if (!test_light_item_list()) failures++;
  if (!test_render_pipeline_config()) failures++;

  printf("\n=== Results: %d test(s) failed ===\n", failures);

  return failures;
}
