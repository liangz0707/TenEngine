#pragma once

#include <te/pipelinecore/Config.h>
#include <te/pipelinecore/FrameContext.h>
#include <te/rendercore/types.hpp>
#include <cstddef>
#include <cstdint>

namespace te::rhi {
struct ICommandList;
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

/// Pass 类型（用于派生 PassBuilder 与收集分发）
enum class PassKind : uint32_t {
  Scene = 0,       // 场景物体（ModelComponent）
  Light,           // 灯光
  PostProcess,     // 后处理
  Effect,          // 特效
  Custom,
};

/// Pass 渲染内容来源
enum class PassContentSource : uint32_t {
  FromModelComponent = 0,  // 场景模型
  FromLightComponent,      // 灯光
  FromPassDefined,         // Pass 内定义（后处理、全屏 quad 等）
  Custom,
};

constexpr uint32_t kMaxPassColorAttachments = 8u;

/// Pass 输出描述（渲染目标、深度、多 RT、分辨率、格式等）. 020 据此构建 RHI RenderPassDesc；具体 ITexture* 由 020 从 FrameGraph 资源或 SwapChain 解析填入。
struct PassOutputDesc {
  uint32_t width{0};
  uint32_t height{0};
  uint32_t colorAttachmentCount{1};
  bool useDepthStencil{false};
  uint32_t colorFormats[kMaxPassColorAttachments]{0};  // 0 = infer; 用于校验或创建 RT
};

/// LoadOp/StoreOp 与 te::rhi::LoadOp/StoreOp 值一致，020 转换时使用（避免 Windows Load/Store 宏）
enum class AttachmentLoadOp : uint32_t { LoadOp_Load = 0, Clear = 1, DontCare = 2 };
enum class AttachmentStoreOp : uint32_t { StoreOp_Store = 0, DontCare = 1 };

/// Attachment 生命周期：Transient 每帧创建/销毁，Persistent 创建一次多帧复用
enum class AttachmentLifetime : uint32_t { Transient = 0, Persistent = 1 };

/// 单 Attachment 描述；用于 Pass 级 Attachment 定义与连接
struct PassAttachmentDesc {
  te::rendercore::ResourceHandle handle{};
  uint32_t width{0};
  uint32_t height{0};
  uint32_t format{0};  // 0 = infer
  bool isDepthStencil{false};
  AttachmentLoadOp loadOp{AttachmentLoadOp::Clear};
  AttachmentStoreOp storeOp{AttachmentStoreOp::Store};
  AttachmentLifetime lifetime{AttachmentLifetime::Transient};
  size_t sourcePassIndex{static_cast<size_t>(-1)};  // 上一 Pass 索引，-1 表示无
  uint32_t sourceAttachmentIndex{0};
};

/// 收集到的物体列表（只读）；由 Pipeline 在收集阶段填充
struct IRenderObjectList {
  virtual ~IRenderObjectList() = default;
  virtual size_t Size() const = 0;
};

struct IRenderItemList;
struct ILightItemList;

constexpr size_t kMaxPassContextRenderItemSlots = 4u;

/// Pass 执行上下文
struct PassContext {
  IRenderObjectList const* GetCollectedObjects() const { return collectedObjects_; }
  void SetCollectedObjects(IRenderObjectList const* o) { collectedObjects_ = o; }

  IRenderItemList const* GetRenderItemList(size_t slot) const {
    return (slot < kMaxPassContextRenderItemSlots) ? renderItemLists_[slot] : nullptr;
  }
  ILightItemList const* GetLightItemList() const { return lightItemList_; }
  void SetRenderItemList(size_t slot, IRenderItemList const* list) {
    if (slot < kMaxPassContextRenderItemSlots) renderItemLists_[slot] = list;
  }
  void SetLightItemList(ILightItemList const* list) { lightItemList_ = list; }

 private:
  IRenderObjectList const* collectedObjects_{nullptr};
  IRenderItemList const* renderItemLists_[kMaxPassContextRenderItemSlots]{};
  ILightItemList const* lightItemList_{nullptr};
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
  /// Pass 类型与内容来源（派生 Builder 可覆盖默认）
  virtual void SetPassKind(PassKind kind) = 0;
  virtual void SetContentSource(PassContentSource source) = 0;
  virtual PassKind GetPassKind() const = 0;
  virtual PassContentSource GetContentSource() const = 0;
  virtual void AddColorAttachment(PassAttachmentDesc const& desc) = 0;
  virtual void SetDepthStencilAttachment(PassAttachmentDesc const& desc) = 0;
};

/// 场景 Pass Builder（ModelComponent 收集）
struct IScenePassBuilder : public IPassBuilder {
  ~IScenePassBuilder() override = default;
};

/// 灯光 Pass Builder
struct ILightPassBuilder : public IPassBuilder {
  ~ILightPassBuilder() override = default;
};

/// 后处理 Pass Builder（Pass 内定义 quad + material）
struct IPostProcessPassBuilder : public IPassBuilder {
  ~IPostProcessPassBuilder() override = default;
  /// 指定本 Pass 使用的材质名（如 "color_grading"）
  virtual void SetMaterial(char const* name) = 0;
  /// 指定本 Pass 使用的 mesh 名（如 "fullscreen_quad"）
  virtual void SetMesh(char const* name) = 0;
  /// 等价于 SetMesh("fullscreen_quad")
  virtual void SetFullscreenQuad() = 0;
};

/// 特效 Pass Builder
struct IEffectPassBuilder : public IPassBuilder {
  ~IEffectPassBuilder() override = default;
};

/// 保留 ResourceHandle.id 约定：0 表示 SwapChain/BackBuffer
constexpr uint64_t kResourceHandleIdBackBuffer = 0u;

constexpr uint32_t kMaxPassReadResources = 8u;

/// Pass 收集配置；供 BuildLogicalPipeline 使用
struct PassCollectConfig {
  ISceneWorld const* scene{nullptr};
  CullMode cullMode{CullMode::None};
  RenderType renderType{RenderType::Opaque};
  PassOutputDesc output{};
  PassKind passKind{PassKind::Scene};
  PassContentSource contentSource{PassContentSource::FromModelComponent};
  uint32_t colorAttachmentCount{0};
  PassAttachmentDesc colorAttachments[kMaxPassColorAttachments];
  bool hasDepthStencil{false};
  PassAttachmentDesc depthStencilAttachment{};
  /// Pass 名称（指针在 FrameGraph 存活期内有效）
  char const* passName{nullptr};
  /// 本 Pass 使用的材质名（如 "color_grading"），由 020 解析为句柄；空表示用场景材质
  char const* materialName{nullptr};
  /// 本 Pass 使用的 mesh 名（如 "fullscreen_quad"），空表示用场景 mesh
  char const* meshName{nullptr};
  uint32_t readResourceCount{0};
  uint64_t readResourceIds[kMaxPassReadResources]{};
};

/// FrameGraph 入口
struct IFrameGraph {
  virtual ~IFrameGraph() = default;
  /// 添加 Pass，返回通用 Builder；kind 决定收集与执行时的分发
  virtual IPassBuilder* AddPass(char const* name) = 0;
  /// 添加 Pass 并指定类型，返回对应派生 Builder（调用方可转型为 IScenePassBuilder* 等）
  virtual IPassBuilder* AddPass(char const* name, PassKind kind) = 0;
  virtual bool Compile() = 0;
  /// 编译后可用；executionOrder 0 为第一个执行的 Pass
  virtual size_t GetPassCount() const = 0;
  virtual void GetPassCollectConfig(size_t executionOrder, PassCollectConfig* out) const = 0;
  /// 按执行顺序调用指定 Pass 的 ExecuteCallback；020 在 Device 任务内按 GetPassCount 循环调用
  virtual void ExecutePass(size_t executionOrder, PassContext& ctx, te::rhi::ICommandList* cmd) = 0;
};

IFrameGraph* CreateFrameGraph();
void DestroyFrameGraph(IFrameGraph* g);

}  // namespace te::pipelinecore
