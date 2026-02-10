#pragma once

#include <te/pipelinecore/LogicalPipeline.h>
#include <te/pipelinecore/RenderItem.h>
#include <cstddef>
#include <cstdint>

namespace te::rendercore {
enum class ResultCode : uint32_t;
}

namespace te::pipelinecore {

/// 单条逻辑绘制；映射到 RHI DrawIndexed 时使用
struct LogicalDraw {
  IMeshHandle const* mesh{nullptr};
  IMaterialHandle const* material{nullptr};
  uint32_t submeshIndex{0};
  uint32_t indexCount{0};
  uint32_t firstIndex{0};
  int32_t vertexOffset{0};
  uint32_t instanceCount{1};
  uint32_t firstInstance{0};
};

/// CPU 侧逻辑命令序列；格式符合 pipeline-to-rci.md
struct ILogicalCommandBuffer {
  virtual ~ILogicalCommandBuffer() = default;
  virtual size_t GetDrawCount() const = 0;
  virtual void GetDraw(size_t index, LogicalDraw* out) const = 0;
};

/// 必须在线程 D 调用；将 RenderItem 列表转换为逻辑命令缓冲
te::rendercore::ResultCode ConvertToLogicalCommandBuffer(IRenderItemList const* items,
                                                         ILogicalPipeline const* pipeline,
                                                         ILogicalCommandBuffer** out);

/// 别名：同 ConvertToLogicalCommandBuffer
inline te::rendercore::ResultCode CollectCommandBuffer(IRenderItemList const* items,
                                                       ILogicalPipeline const* pipeline,
                                                       ILogicalCommandBuffer** out) {
  return ConvertToLogicalCommandBuffer(items, pipeline, out);
}

void DestroyLogicalCommandBuffer(ILogicalCommandBuffer* cb);

}  // namespace te::pipelinecore
