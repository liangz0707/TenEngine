/**
 * @file LogicalCommandBufferExecutor.cpp
 * @brief Implementation of ExecuteLogicalCommandBufferOnDeviceThread.
 *
 * Converts logical draw commands to actual RHI draw calls.
 */

#include <te/pipeline/LogicalCommandBufferExecutor.h>
#include <te/pipelinecore/LogicalCommandBuffer.h>
#include <te/pipelinecore/RenderItem.h>
#include <te/rendercore/IRenderElement.hpp>
#include <te/rendercore/IRenderMesh.hpp>
#include <te/rendercore/IRenderMaterial.hpp>
#include <te/rendercore/IRenderTexture.hpp>
#include <te/rhi/command_list.hpp>
#include <te/rhi/device.hpp>

#include <algorithm>

namespace te::pipeline {

void ExecuteLogicalCommandBufferOnDeviceThread(
    rhi::ICommandList* cmd,
    pipelinecore::ILogicalCommandBuffer const* logicalCB,
    uint32_t frameSlot) {

  if (!cmd || !logicalCB) return;

  size_t drawCount = logicalCB->GetDrawCount();
  if (drawCount == 0) return;

  for (size_t i = 0; i < drawCount; ++i) {
    pipelinecore::LogicalDraw draw;
    logicalCB->GetDraw(i, &draw);

    if (!draw.element) continue;

    // Get mesh and material from element
    rendercore::IRenderMesh* mesh = draw.element->GetMesh();
    rendercore::IRenderMaterial* material = draw.element->GetMaterial();

    if (!mesh) continue;

    // === Material setup ===
    if (material) {
      // Update device resources for this frame slot
      // This ensures uniform buffers are up-to-date
      // Note: UpdateDeviceResource should be called with a valid device,
      // but we only have ICommandList here. The device is set during PrepareResources.

      // Get and bind PSO
      rhi::IPSO* pso = material->GetGraphicsPSO(0);
      if (pso) {
        cmd->SetGraphicsPSO(pso);
      }

      // Bind descriptor set (contains textures, uniform buffers, samplers)
      rhi::IDescriptorSet* descSet = material->GetDescriptorSet();
      if (descSet) {
        cmd->BindDescriptorSet(descSet);
      }
    }

    // === Mesh setup ===
    rhi::IBuffer* vertexBuffer = mesh->GetVertexBuffer();
    rhi::IBuffer* indexBuffer = mesh->GetIndexBuffer();

    if (!vertexBuffer || !indexBuffer) continue;

    // Get submesh range
    rendercore::SubmeshRange submeshRange;
    if (!mesh->GetSubmesh(draw.submeshIndex, &submeshRange)) {
      // If submesh not found, use the draw's values
      submeshRange.indexOffset = draw.firstIndex;
      submeshRange.indexCount = draw.indexCount;
      submeshRange.vertexOffset = draw.vertexOffset;
    }

    // Calculate vertex stride (assuming standard vertex format)
    // In a real implementation, this would come from the mesh or material
    constexpr uint32_t vertexStride = 32; // Position(12) + Normal(12) + UV(8)
    constexpr uint32_t indexFormat = 1;   // 0 = 16-bit, 1 = 32-bit

    // Bind vertex buffer
    cmd->SetVertexBuffer(0, vertexBuffer, submeshRange.vertexOffset, vertexStride);

    // Bind index buffer
    cmd->SetIndexBuffer(indexBuffer, submeshRange.indexOffset, indexFormat);

    // === Draw ===
    uint32_t indexCount = submeshRange.indexCount > 0 ? submeshRange.indexCount : draw.indexCount;
    if (indexCount == 0) continue;

    cmd->DrawIndexed(
      indexCount,
      draw.instanceCount,
      0,  // first index (relative to bound index buffer offset)
      static_cast<int32_t>(submeshRange.vertexOffset),  // vertex offset
      draw.firstInstance);
  }
}

void ExecuteLogicalCommandBufferOnDeviceThreadWithStats(
    rhi::ICommandList* cmd,
    pipelinecore::ILogicalCommandBuffer const* logicalCB,
    uint32_t frameSlot,
    ExecutionStats* outStats) {

  if (outStats) {
    *outStats = ExecutionStats{};
  }

  if (!cmd || !logicalCB) return;

  size_t drawCount = logicalCB->GetDrawCount();
  if (drawCount == 0) return;

  uint32_t totalDrawCalls = 0;
  uint32_t totalInstances = 0;
  uint32_t totalTriangles = 0;
  uint32_t totalVertices = 0;

  for (size_t i = 0; i < drawCount; ++i) {
    pipelinecore::LogicalDraw draw;
    logicalCB->GetDraw(i, &draw);

    if (!draw.element) continue;

    rendercore::IRenderMesh* mesh = draw.element->GetMesh();
    rendercore::IRenderMaterial* material = draw.element->GetMaterial();

    if (!mesh) continue;

    // Material setup
    if (material) {
      rhi::IPSO* pso = material->GetGraphicsPSO(0);
      if (pso) {
        cmd->SetGraphicsPSO(pso);
      }

      rhi::IDescriptorSet* descSet = material->GetDescriptorSet();
      if (descSet) {
        cmd->BindDescriptorSet(descSet);
      }
    }

    // Mesh setup
    rhi::IBuffer* vertexBuffer = mesh->GetVertexBuffer();
    rhi::IBuffer* indexBuffer = mesh->GetIndexBuffer();

    if (!vertexBuffer || !indexBuffer) continue;

    rendercore::SubmeshRange submeshRange;
    if (!mesh->GetSubmesh(draw.submeshIndex, &submeshRange)) {
      submeshRange.indexOffset = draw.firstIndex;
      submeshRange.indexCount = draw.indexCount;
      submeshRange.vertexOffset = draw.vertexOffset;
    }

    constexpr uint32_t vertexStride = 32;
    constexpr uint32_t indexFormat = 1;

    cmd->SetVertexBuffer(0, vertexBuffer, submeshRange.vertexOffset, vertexStride);
    cmd->SetIndexBuffer(indexBuffer, submeshRange.indexOffset, indexFormat);

    uint32_t indexCount = submeshRange.indexCount > 0 ? submeshRange.indexCount : draw.indexCount;
    if (indexCount == 0) continue;

    cmd->DrawIndexed(
      indexCount,
      draw.instanceCount,
      0,
      static_cast<int32_t>(submeshRange.vertexOffset),
      draw.firstInstance);

    // Update stats
    totalDrawCalls++;
    totalInstances += draw.instanceCount;
    totalTriangles += (indexCount / 3) * draw.instanceCount;
    // Estimate vertex count (would need actual data for accurate count)
    totalVertices += indexCount * draw.instanceCount;
  }

  if (outStats) {
    outStats->drawCalls = totalDrawCalls;
    outStats->instanceCount = totalInstances;
    outStats->triangleCount = totalTriangles;
    outStats->vertexCount = totalVertices;
  }
}

}  // namespace te::pipeline
