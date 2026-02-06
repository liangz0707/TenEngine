#pragma once

#include <te/pipelinecore/Config.h>

namespace te::pipelinecore {

struct ISceneWorld;  // 定义在 FrameGraph.h

/// 视口描述
struct ViewportDesc {
  uint32_t width{0};
  uint32_t height{0};
};

/// 帧上下文；020 构造并传入 BuildLogicalPipeline、CollectRenderItemsParallel
struct FrameContext {
  ISceneWorld const* scene{nullptr};
  void const* camera{nullptr};
  ViewportDesc viewport{};
  FrameSlotId frameSlotId{0u};
};

}  // namespace te::pipelinecore
