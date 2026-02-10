/**
 * @file PipelineImpl.h
 * @brief 020-Pipeline internal: ISceneWorld adapter, FrameContext conversion, Deferred graph setup.
 */

#ifndef TE_PIPELINE_DETAIL_PIPELINE_IMPL_H
#define TE_PIPELINE_DETAIL_PIPELINE_IMPL_H

#include <te/pipeline/FrameContext.h>
#include <te/pipelinecore/FrameContext.h>
#include <te/pipelinecore/FrameGraph.h>
#include <cstdint>

namespace te {
namespace pipeline {
namespace detail {

/// 适配 020 的 sceneRoot（void*）为 019 的 ISceneWorld；当前仅占位，019 不查询方法
struct SceneWorldAdapter : pipelinecore::ISceneWorld {
  void* sceneRefOrLevelHandle{nullptr};  ///< SceneRef* 或 LevelHandle*
  bool useLevelHandle{false};
};

/// 从 020 FrameContext 构建 019 pipelinecore::FrameContext
void ToPipelinecoreFrameContext(pipeline::FrameContext const& ctx,
                                pipelinecore::ISceneWorld const* sceneAdapter,
                                pipelinecore::FrameContext& out);

/// 创建并配置 Deferred 用 FrameGraph（GBuffer + Lighting），已 Compile
pipelinecore::IFrameGraph* CreateDeferredFrameGraph(uint32_t viewportWidth, uint32_t viewportHeight);

}  // namespace detail
}  // namespace pipeline
}  // namespace te

#endif  // TE_PIPELINE_DETAIL_PIPELINE_IMPL_H
