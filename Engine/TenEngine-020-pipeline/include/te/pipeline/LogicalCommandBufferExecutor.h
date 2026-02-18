/**
 * @file LogicalCommandBufferExecutor.h
 * @brief Executes logical command buffer on the device thread.
 *
 * Converts ILogicalCommandBuffer to actual RHI draw calls.
 */

#pragma once

#include <te/pipelinecore/LogicalCommandBuffer.h>
#include <cstdint>

namespace te {
namespace rhi {
struct ICommandList;
}
namespace pipelinecore {
class ILogicalCommandBuffer;
}
}

namespace te::pipeline {

/// Statistics from command buffer execution
struct ExecutionStats {
  uint32_t drawCalls{0};
  uint32_t instanceCount{0};
  uint32_t triangleCount{0};
  uint32_t vertexCount{0};
};

/**
 * @brief Execute logical command buffer on the device thread.
 *
 * This function must be called from the device thread (Thread D) and
 * must be within a BeginRenderPass/EndRenderPass block.
 *
 * @param cmd The RHI command list to record draw calls into.
 * @param logicalCB The logical command buffer containing draw commands.
 * @param frameSlot The current frame slot for resource updates.
 *
 * For each LogicalDraw in the buffer:
 * 1. Gets mesh and material from IRenderElement
 * 2. Sets PSO from material
 * 3. Binds descriptor set from material
 * 4. Binds vertex and index buffers from mesh
 * 5. Calls DrawIndexed with submesh range
 */
void ExecuteLogicalCommandBufferOnDeviceThread(
    rhi::ICommandList* cmd,
    pipelinecore::ILogicalCommandBuffer const* logicalCB,
    uint32_t frameSlot = 0);

/**
 * @brief Execute logical command buffer with statistics output.
 *
 * Same as ExecuteLogicalCommandBufferOnDeviceThread but also
 * outputs execution statistics.
 *
 * @param cmd The RHI command list to record draw calls into.
 * @param logicalCB The logical command buffer containing draw commands.
 * @param frameSlot The current frame slot for resource updates.
 * @param outStats Output statistics structure.
 */
void ExecuteLogicalCommandBufferOnDeviceThreadWithStats(
    rhi::ICommandList* cmd,
    pipelinecore::ILogicalCommandBuffer const* logicalCB,
    uint32_t frameSlot,
    ExecutionStats* outStats);

}  // namespace te::pipeline
