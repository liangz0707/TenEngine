/**
 * @file FrameContext.h
 * @brief 020-Pipeline frame context: scene_root, camera, render_target, viewport, delta_time.
 * Contract: specs/_contracts/020-pipeline-ABI.md
 */

#ifndef TE_PIPELINE_FRAME_CONTEXT_H
#define TE_PIPELINE_FRAME_CONTEXT_H

#include <cstdint>

namespace te {
namespace pipeline {

/// 视口描述（只读）
struct ViewportDesc {
  uint32_t width{0};
  uint32_t height{0};
};

/// 帧上下文；只读，由调用方每帧填充；供 TriggerRender / RenderFrame / BuildLogicalPipeline 使用
struct FrameContext {
  void* sceneRoot{nullptr};    ///< 场景根或 LevelHandle/SceneRef；020 用于 029 CollectRenderables
  void* camera{nullptr};       ///< 相机句柄
  void* frustum{nullptr};      ///< 可选；视锥剔除用，te::scene::Frustum const*；为 nullptr 时不剔除
  void* renderTarget{nullptr}; ///< 渲染目标句柄（与 SwapChain/XR 对接）
  ViewportDesc viewport{};
  float deltaTime{0.f};
  uint32_t frameSlotId{0u};    ///< 本帧 slot；与 pipelinecore::FrameSlotId 一致
};

}  // namespace pipeline
}  // namespace te

#endif  // TE_PIPELINE_FRAME_CONTEXT_H
