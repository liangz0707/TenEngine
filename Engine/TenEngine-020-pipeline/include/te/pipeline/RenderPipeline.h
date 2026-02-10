/**
 * @file RenderPipeline.h
 * @brief 020-Pipeline: IRenderPipeline, CreateRenderPipeline, TriggerRender, SubmitLogicalCommandBuffer.
 * Contract: specs/_contracts/020-pipeline-ABI.md, pipeline-to-rci.md
 */

#ifndef TE_PIPELINE_RENDER_PIPELINE_H
#define TE_PIPELINE_RENDER_PIPELINE_H

#include <te/pipeline/FrameContext.h>
#include <te/pipeline/RenderingConfig.h>
#include <cstdint>

namespace te {
namespace pipeline {

struct RenderPipelineDesc;
class IRenderPipeline;

/// 渲染模式（校验程度）
enum class RenderMode : uint32_t {
  Debug = 0,    ///< 全量校验
  Hybrid,       ///< 部分校验
  Resource,     ///< 发布/最小校验
};

/// 创建渲染管线；调用方或引擎管理返回指针的生命周期
IRenderPipeline* CreateRenderPipeline(RenderPipelineDesc const& desc);
IRenderPipeline* CreateRenderPipeline(void* device);  ///< 简化：仅 IDevice*

/// 创建管线时的配置
struct RenderPipelineDesc {
  uint32_t frameInFlightCount{2u};
  void* device{nullptr};   ///< te::rhi::IDevice*
  void* swapChain{nullptr}; ///< te::rhi::ISwapChain* 可选
  void* resourceManager{nullptr}; ///< te::resource::IResourceManager* 可选，用于 029 CollectRenderables 解析 model
};

/// 一帧渲染入口；主循环或 Editor 每帧调用
class IRenderPipeline {
 public:
  virtual ~IRenderPipeline() = default;

  /// 一帧渲染（等价于 TriggerRender，单入口）
  virtual void RenderFrame(FrameContext const& ctx) = 0;

  /// 多阶段推进一帧：等待 slot → 收集 → 录制并 submit → 若轮到则 present
  virtual void TickPipeline(FrameContext const& ctx) = 0;

  /// 触发一次渲染：BuildLogicalPipeline → CollectRenderItemsParallel + Merge → (Device 线程) PrepareRenderResources、ConvertToLogicalCommandBuffer、SubmitLogicalCommandBuffer
  virtual void TriggerRender(FrameContext const& ctx) = 0;

  /// 本帧使用的 slot
  virtual uint32_t GetCurrentSlot() const = 0;

  /// 渲染配置；下一帧生效
  virtual void SetRenderingConfig(RenderingConfig const& config) = 0;
  virtual RenderingConfig GetRenderingConfig() const = 0;

  /// FrameGraph；每帧执行时编译并执行 graph 中的 Pass
  virtual void* GetFrameGraph() = 0;   ///< 返回 te::pipelinecore::IFrameGraph*
  virtual void SetFrameGraph(void* graph) = 0;  ///< graph 为 te::pipelinecore::IFrameGraph*

  /// 提交逻辑命令缓冲到实际 GPU；**必须在线程 D（Device 线程）调用**
  virtual void SubmitLogicalCommandBuffer(void* logical_cb) = 0;  ///< logical_cb 为 te::pipelinecore::ILogicalCommandBuffer*
};

}  // namespace pipeline
}  // namespace te

#endif  // TE_PIPELINE_RENDER_PIPELINE_H
