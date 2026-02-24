#pragma once

#include <te/pipelinecore/FrameContext.h>
#include <te/pipelinecore/FrameGraph.h>

namespace te::pipelinecore {

struct ILogicalPipeline {
  virtual ~ILogicalPipeline() = default;
  virtual size_t GetPassCount() const = 0;
  virtual void GetPassConfig(size_t index, PassCollectConfig* out) const = 0;
  virtual IRenderItemList const* GetSourceItemList() const = 0;
};

/// 从已编译的 FrameGraph 构建逻辑管线；产出 Pass 列表与每 Pass 收集配置
ILogicalPipeline* BuildLogicalPipeline(IFrameGraph const* graph, FrameContext const& ctx);
void DestroyLogicalPipeline(ILogicalPipeline* p);

}  // namespace te::pipelinecore
