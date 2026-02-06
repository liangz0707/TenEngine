#pragma once

#include <cstdint>

namespace te::pipelinecore {

/// 建议最大在途帧数；实现可配置
constexpr uint32_t kMaxFramesInFlight = 4u;

/// 管线配置
struct PipelineConfig {
  uint32_t frameInFlightCount{2u};  ///< 2～4
};

/// 帧 slot 索引；有效范围 [0, frameInFlightCount)；与 RHI waitForSlot / getCommandListForSlot 协同
using FrameSlotId = uint32_t;

}  // namespace te::pipelinecore
