#pragma once

#include <te/pipelinecore/Config.h>
#include <te/pipelinecore/FrameContext.h>

namespace te::rhi {
struct ICommandList;
}
namespace te::rendercore {
struct PassHandle;
struct ResourceHandle;
}

namespace te::pipelinecore {

/// 场景世界最小接口；020/004 实现
struct ISceneWorld {
  virtual ~ISceneWorld() = default;
};

/// 收集/剔除方式
enum class CullMode : uint32_t {
  None = 0,
  FrustumCull,
  OcclusionCull,
  FrustumAndOcclusion,
};

/// 渲染类型
enum class RenderType : uint32_t {
  Opaque = 0,
  Transparent,
  Overlay,
  Custom,
};

/// Pass 输出描述（渲染目标、深度、多 RT、分辨率、格式等）
struct PassOutputDesc {
  uint32_t width{0};
  uint32_t height{0};
  // 可扩展：colorAttachments, depthStencil, format 等
};

/// 收集到的物体列表（只读）；由 Pipeline 在收集阶段填充
struct IRenderObjectList {
  virtual ~IRenderObjectList() = default;
  virtual size_t Size() const = 0;
};

/// Pass 执行上下文
struct PassContext {
  IRenderObjectList const* GetCollectedObjects() const { return collectedObjects_; }
  void SetCollectedObjects(IRenderObjectList const* o) { collectedObjects_ = o; }

 private:
  IRenderObjectList const* collectedObjects_{nullptr};
};

/// Pass 执行回调：void (*)(PassContext& ctx, ICommandList* cmd)
using PassExecuteCallback = void (*)(PassContext& ctx, te::rhi::ICommandList* cmd);

/// Pass 配置 Builder
struct IPassBuilder {
  virtual ~IPassBuilder() = default;
  virtual void SetScene(ISceneWorld const* scene) = 0;
  virtual void SetCullMode(CullMode mode) = 0;
  virtual void SetObjectTypeFilter(void const* filter) = 0;  // 占位，具体类型由 020 定义
  virtual void SetRenderType(RenderType type) = 0;
  virtual void SetOutput(PassOutputDesc const& desc) = 0;
  virtual void SetExecuteCallback(PassExecuteCallback cb) = 0;
  /// RDG 资源声明；与 009 PassProtocol 对接；Compile 时据此推导执行顺序
  virtual void DeclareRead(te::rendercore::ResourceHandle const& resource) = 0;
  virtual void DeclareWrite(te::rendercore::ResourceHandle const& resource) = 0;
};

/// Pass 收集配置；供 BuildLogicalPipeline 使用
struct PassCollectConfig {
  ISceneWorld const* scene{nullptr};
  CullMode cullMode{CullMode::None};
  RenderType renderType{RenderType::Opaque};
  PassOutputDesc output{};
};

/// FrameGraph 入口
struct IFrameGraph {
  virtual ~IFrameGraph() = default;
  virtual IPassBuilder* AddPass(char const* name) = 0;
  virtual bool Compile() = 0;
  /// 编译后可用；executionOrder 0 为第一个执行的 Pass
  virtual size_t GetPassCount() const = 0;
  virtual void GetPassCollectConfig(size_t executionOrder, PassCollectConfig* out) const = 0;
};

IFrameGraph* CreateFrameGraph();
void DestroyFrameGraph(IFrameGraph* g);

}  // namespace te::pipelinecore
