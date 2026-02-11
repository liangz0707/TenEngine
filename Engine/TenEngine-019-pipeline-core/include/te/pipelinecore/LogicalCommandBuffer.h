#pragma once

#include <te/pipelinecore/LogicalPipeline.h>
#include <te/pipelinecore/RenderItem.h>
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

}  // namespace te::pipelinecore
