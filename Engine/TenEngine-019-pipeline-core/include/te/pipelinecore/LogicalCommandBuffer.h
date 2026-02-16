#pragma once

#include <te/pipelinecore/LogicalPipeline.h>
#include <te/pipelinecore/RenderItem.h>
#include <te/rhi/command_list.hpp>
#include <te/rhi/device.hpp>
#include <te/rhi/swapchain.hpp>
#include <te/rhi/sync.hpp>
#include <cstddef>
#include <cstdint>

namespace te::rendercore {
enum class ResultCode : uint32_t;
struct IRenderElement;
}

namespace te::pipelinecore {

/// 单条逻辑绘制；仅由 element 提供 mesh/material
struct LogicalDraw {
  te::rendercore::IRenderElement* element{nullptr};
  uint32_t submeshIndex{0};
  uint32_t indexCount{0};
  uint32_t firstIndex{0};
  int32_t vertexOffset{0};
  uint32_t instanceCount{1};
  uint32_t firstInstance{0};
  void* skinMatrixBuffer{nullptr};
  uint32_t skinMatrixOffset{0};
};

struct ILogicalCommandBuffer {
  virtual ~ILogicalCommandBuffer() = default;
  virtual size_t GetDrawCount() const = 0;
  virtual void GetDraw(size_t index, LogicalDraw* out) const = 0;
};

te::rendercore::ResultCode ConvertToLogicalCommandBuffer(IRenderItemList const* items,
                                                         ILogicalPipeline const* pipeline,
                                                         ILogicalCommandBuffer** out);

inline te::rendercore::ResultCode CollectCommandBuffer(IRenderItemList const* items,
                                                       ILogicalPipeline const* pipeline,
                                                       ILogicalCommandBuffer** out) {
  return ConvertToLogicalCommandBuffer(items, pipeline, out);
}

void DestroyLogicalCommandBuffer(ILogicalCommandBuffer* cb);

/// 执行逻辑命令缓冲到 RHI 命令列表（在 Device 线程调用）
/// @param cmd 已调用的 RHI 命令列表（须已 Begin）
/// @param logicalCb 逻辑命令缓冲
/// @param device RHI 设备（用于获取格式信息等）
/// @param frameSlot 当前帧槽位（用于更新 Uniform Buffer）
te::rendercore::ResultCode ExecuteLogicalCommandBufferOnDeviceThread(
    te::rhi::ICommandList* cmd,
    ILogicalCommandBuffer const* logicalCb,
    te::rhi::IDevice* device,
    uint32_t frameSlot);

/// 提交逻辑命令缓冲到 RHI 队列
/// @param cmd RHI 命令列表
/// @param queue RHI 队列
/// @param signalFence 可选的信号 Fence
/// @param waitSem 可选的等待信号量
/// @param signalSem 可选的完成信号量
te::rendercore::ResultCode SubmitLogicalCommandBuffer(
    te::rhi::ICommandList* cmd,
    te::rhi::IQueue* queue,
    te::rhi::IFence* signalFence = nullptr,
    te::rhi::ISemaphore* waitSem = nullptr,
    te::rhi::ISemaphore* signalSem = nullptr);

/// Present 交换链
/// @param swapChain 交换链
te::rendercore::ResultCode PresentSwapChain(te::rhi::ISwapChain* swapChain);

}  // namespace te::pipelinecore
