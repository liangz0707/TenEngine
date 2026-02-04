#include <te/pipelinecore/LogicalCommandBuffer.h>
#include <te/rendercore/types.hpp>

namespace te::pipelinecore {

namespace {

class LogicalCommandBufferImpl : public ILogicalCommandBuffer {};

}  // namespace

te::rendercore::ResultCode ConvertToLogicalCommandBuffer(IRenderItemList const* /*items*/,
                                                         ILogicalPipeline const* /*pipeline*/,
                                                         ILogicalCommandBuffer** out) {
  if (!out) return te::rendercore::ResultCode::InvalidHandle;
  *out = new LogicalCommandBufferImpl();
  return te::rendercore::ResultCode::Success;
}

void DestroyLogicalCommandBuffer(ILogicalCommandBuffer* cb) { delete cb; }

}  // namespace te::pipelinecore
