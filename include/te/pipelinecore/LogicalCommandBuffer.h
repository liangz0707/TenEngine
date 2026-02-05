#pragma once

#include <te/pipelinecore/LogicalPipeline.h>
#include <te/pipelinecore/RenderItem.h>

namespace te::rendercore {
enum class ResultCode : uint32_t;
}

namespace te::pipelinecore {

/// CPU 侧逻辑命令序列；格式符合 pipeline-to-rci.md
struct ILogicalCommandBuffer {
  virtual ~ILogicalCommandBuffer() = default;
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
