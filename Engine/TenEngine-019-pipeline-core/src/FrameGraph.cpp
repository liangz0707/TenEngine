#include <te/pipelinecore/FrameGraph.h>
#include <te/pipelinecore/Profiling.h>
#include <te/rendercore/types.hpp>
#include <algorithm>
#include <chrono>
#include <memory>
#include <utility>
#include <string>
#include <unordered_map>
#include <vector>

namespace te::pipelinecore {

namespace {

struct PassData {
  std::string name;
  std::string materialName;
  std::string meshName;
  ISceneWorld const* scene{nullptr};
  CullMode cullMode{CullMode::None};
  RenderType renderType{RenderType::Opaque};
  PassOutputDesc output{};
  PassExecuteCallback executeCallback{nullptr};
  PassKind passKind{PassKind::Scene};
  PassContentSource contentSource{PassContentSource::FromModelComponent};
  uint32_t colorAttachmentCount{0};
  PassAttachmentDesc colorAttachments[kMaxPassColorAttachments];
  bool hasDepthStencil{false};
  PassAttachmentDesc depthStencilAttachment{};
  std::vector<size_t> depIndices;
  std::vector<uint64_t> readResources;
  std::vector<uint64_t> writeResources;
};

class PassBuilderImpl : public IPassBuilder {
 public:
  explicit PassBuilderImpl(PassData& data) : data_(data) {}
  void SetScene(ISceneWorld const* scene) override { data_.scene = scene; }
  void SetCullMode(CullMode mode) override { data_.cullMode = mode; }
  void SetObjectTypeFilter(void const* /*filter*/) override { /* 占位 */ }
  void SetRenderType(RenderType type) override { data_.renderType = type; }
  void SetOutput(PassOutputDesc const& desc) override { data_.output = desc; }
  void SetExecuteCallback(PassExecuteCallback cb) override { data_.executeCallback = cb; }
  void DeclareRead(te::rendercore::ResourceHandle const& resource) override {
    if (resource.IsValid()) data_.readResources.push_back(resource.id);
  }
  void DeclareWrite(te::rendercore::ResourceHandle const& resource) override {
    if (resource.IsValid()) data_.writeResources.push_back(resource.id);
  }
  void SetPassKind(PassKind kind) override { data_.passKind = kind; }
  void SetContentSource(PassContentSource source) override { data_.contentSource = source; }
  PassKind GetPassKind() const override { return data_.passKind; }
  PassContentSource GetContentSource() const override { return data_.contentSource; }
  void AddColorAttachment(PassAttachmentDesc const& desc) override {
    if (data_.colorAttachmentCount < kMaxPassColorAttachments)
      data_.colorAttachments[data_.colorAttachmentCount++] = desc;
  }
  void SetDepthStencilAttachment(PassAttachmentDesc const& desc) override {
    data_.hasDepthStencil = true;
    data_.depthStencilAttachment = desc;
  }

 protected:
  void SetPassMaterial(char const* name) { data_.materialName = name ? name : ""; }
  void SetPassMesh(char const* name) { data_.meshName = name ? name : ""; }

 private:
  PassData& data_;
};

/// 派生 Builder 实现：复用 PassBuilderImpl，仅类型不同以便 AddPass(name, kind) 返回可转型指针
class ScenePassBuilderImpl : public IScenePassBuilder, private PassBuilderImpl {
 public:
  explicit ScenePassBuilderImpl(PassData& data) : PassBuilderImpl(data) {}
  void SetScene(ISceneWorld const* scene) override { PassBuilderImpl::SetScene(scene); }
  void SetCullMode(CullMode mode) override { PassBuilderImpl::SetCullMode(mode); }
  void SetObjectTypeFilter(void const* f) override { PassBuilderImpl::SetObjectTypeFilter(f); }
  void SetRenderType(RenderType type) override { PassBuilderImpl::SetRenderType(type); }
  void SetOutput(PassOutputDesc const& desc) override { PassBuilderImpl::SetOutput(desc); }
  void SetExecuteCallback(PassExecuteCallback cb) override { PassBuilderImpl::SetExecuteCallback(cb); }
  void DeclareRead(te::rendercore::ResourceHandle const& r) override { PassBuilderImpl::DeclareRead(r); }
  void DeclareWrite(te::rendercore::ResourceHandle const& r) override { PassBuilderImpl::DeclareWrite(r); }
  void SetPassKind(PassKind kind) override { PassBuilderImpl::SetPassKind(kind); }
  void SetContentSource(PassContentSource src) override { PassBuilderImpl::SetContentSource(src); }
  PassKind GetPassKind() const override { return PassBuilderImpl::GetPassKind(); }
  PassContentSource GetContentSource() const override { return PassBuilderImpl::GetContentSource(); }
  void AddColorAttachment(PassAttachmentDesc const& d) override { PassBuilderImpl::AddColorAttachment(d); }
  void SetDepthStencilAttachment(PassAttachmentDesc const& d) override { PassBuilderImpl::SetDepthStencilAttachment(d); }
};

class LightPassBuilderImpl : public ILightPassBuilder, private PassBuilderImpl {
 public:
  explicit LightPassBuilderImpl(PassData& data) : PassBuilderImpl(data) {}
  void SetScene(ISceneWorld const* scene) override { PassBuilderImpl::SetScene(scene); }
  void SetCullMode(CullMode mode) override { PassBuilderImpl::SetCullMode(mode); }
  void SetObjectTypeFilter(void const* f) override { PassBuilderImpl::SetObjectTypeFilter(f); }
  void SetRenderType(RenderType type) override { PassBuilderImpl::SetRenderType(type); }
  void SetOutput(PassOutputDesc const& desc) override { PassBuilderImpl::SetOutput(desc); }
  void SetExecuteCallback(PassExecuteCallback cb) override { PassBuilderImpl::SetExecuteCallback(cb); }
  void DeclareRead(te::rendercore::ResourceHandle const& r) override { PassBuilderImpl::DeclareRead(r); }
  void DeclareWrite(te::rendercore::ResourceHandle const& r) override { PassBuilderImpl::DeclareWrite(r); }
  void SetPassKind(PassKind kind) override { PassBuilderImpl::SetPassKind(kind); }
  void SetContentSource(PassContentSource src) override { PassBuilderImpl::SetContentSource(src); }
  PassKind GetPassKind() const override { return PassBuilderImpl::GetPassKind(); }
  PassContentSource GetContentSource() const override { return PassBuilderImpl::GetContentSource(); }
  void AddColorAttachment(PassAttachmentDesc const& d) override { PassBuilderImpl::AddColorAttachment(d); }
  void SetDepthStencilAttachment(PassAttachmentDesc const& d) override { PassBuilderImpl::SetDepthStencilAttachment(d); }
};

class PostProcessPassBuilderImpl : public IPostProcessPassBuilder, private PassBuilderImpl {
 public:
  explicit PostProcessPassBuilderImpl(PassData& data) : PassBuilderImpl(data) {}
  void SetScene(ISceneWorld const* scene) override { PassBuilderImpl::SetScene(scene); }
  void SetCullMode(CullMode mode) override { PassBuilderImpl::SetCullMode(mode); }
  void SetObjectTypeFilter(void const* f) override { PassBuilderImpl::SetObjectTypeFilter(f); }
  void SetRenderType(RenderType type) override { PassBuilderImpl::SetRenderType(type); }
  void SetOutput(PassOutputDesc const& desc) override { PassBuilderImpl::SetOutput(desc); }
  void SetExecuteCallback(PassExecuteCallback cb) override { PassBuilderImpl::SetExecuteCallback(cb); }
  void DeclareRead(te::rendercore::ResourceHandle const& r) override { PassBuilderImpl::DeclareRead(r); }
  void DeclareWrite(te::rendercore::ResourceHandle const& r) override { PassBuilderImpl::DeclareWrite(r); }
  void SetPassKind(PassKind kind) override { PassBuilderImpl::SetPassKind(kind); }
  void SetContentSource(PassContentSource src) override { PassBuilderImpl::SetContentSource(src); }
  PassKind GetPassKind() const override { return PassBuilderImpl::GetPassKind(); }
  PassContentSource GetContentSource() const override { return PassBuilderImpl::GetContentSource(); }
  void AddColorAttachment(PassAttachmentDesc const& d) override { PassBuilderImpl::AddColorAttachment(d); }
  void SetDepthStencilAttachment(PassAttachmentDesc const& d) override { PassBuilderImpl::SetDepthStencilAttachment(d); }
  void SetMaterial(char const* name) override { SetPassMaterial(name); }
  void SetMesh(char const* name) override { SetPassMesh(name); }
  void SetFullscreenQuad() override { SetPassMesh("fullscreen_quad"); }
};

class EffectPassBuilderImpl : public IEffectPassBuilder, private PassBuilderImpl {
 public:
  explicit EffectPassBuilderImpl(PassData& data) : PassBuilderImpl(data) {}
  void SetScene(ISceneWorld const* scene) override { PassBuilderImpl::SetScene(scene); }
  void SetCullMode(CullMode mode) override { PassBuilderImpl::SetCullMode(mode); }
  void SetObjectTypeFilter(void const* f) override { PassBuilderImpl::SetObjectTypeFilter(f); }
  void SetRenderType(RenderType type) override { PassBuilderImpl::SetRenderType(type); }
  void SetOutput(PassOutputDesc const& desc) override { PassBuilderImpl::SetOutput(desc); }
  void SetExecuteCallback(PassExecuteCallback cb) override { PassBuilderImpl::SetExecuteCallback(cb); }
  void DeclareRead(te::rendercore::ResourceHandle const& r) override { PassBuilderImpl::DeclareRead(r); }
  void DeclareWrite(te::rendercore::ResourceHandle const& r) override { PassBuilderImpl::DeclareWrite(r); }
  void SetPassKind(PassKind kind) override { PassBuilderImpl::SetPassKind(kind); }
  void SetContentSource(PassContentSource src) override { PassBuilderImpl::SetContentSource(src); }
  PassKind GetPassKind() const override { return PassBuilderImpl::GetPassKind(); }
  PassContentSource GetContentSource() const override { return PassBuilderImpl::GetContentSource(); }
  void AddColorAttachment(PassAttachmentDesc const& d) override { PassBuilderImpl::AddColorAttachment(d); }
  void SetDepthStencilAttachment(PassAttachmentDesc const& d) override { PassBuilderImpl::SetDepthStencilAttachment(d); }
};

static PassContentSource ContentSourceFromKind(PassKind kind) {
  switch (kind) {
    case PassKind::Scene: return PassContentSource::FromModelComponent;
    case PassKind::Light: return PassContentSource::FromLightComponent;
    case PassKind::PostProcess:
    case PassKind::Effect: return PassContentSource::FromPassDefined;
    default: return PassContentSource::Custom;
  }
}

static std::unique_ptr<IPassBuilder> CreateBuilderForKind(PassData& data, PassKind kind) {
  switch (kind) {
    case PassKind::Scene: return std::make_unique<ScenePassBuilderImpl>(data);
    case PassKind::Light: return std::make_unique<LightPassBuilderImpl>(data);
    case PassKind::PostProcess: return std::make_unique<PostProcessPassBuilderImpl>(data);
    case PassKind::Effect: return std::make_unique<EffectPassBuilderImpl>(data);
    default: return std::make_unique<PassBuilderImpl>(data);
  }
}

class FrameGraphImpl : public IFrameGraph {
 public:
  IPassBuilder* AddPass(char const* name) override {
    return AddPass(name, PassKind::Scene);
  }
  IPassBuilder* AddPass(char const* name, PassKind kind) override {
    passes_.emplace_back();
    PassData& data = passes_.back();
    data.name = name ? name : "";
    data.passKind = kind;
    data.contentSource = ContentSourceFromKind(kind);
    builders_.push_back(CreateBuilderForKind(data, kind));
    return builders_.back().get();
  }

  bool Compile() override {
#if TE_PIPELINECORE_PROFILING
    auto t0 = std::chrono::steady_clock::now();
#endif
    sortedIndices_.clear();
    if (passes_.empty()) {
#if TE_PIPELINECORE_PROFILING
      if (g_onCompileProfiling) {
        auto dt = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now() - t0).count();
        g_onCompileProfiling(static_cast<uint64_t>(dt));
      }
#endif
      return true;
    }

    size_t n = passes_.size();
    std::vector<std::vector<size_t>> adj(n);
    std::vector<int> inDegree(n, 0);

    auto addEdge = [&](size_t from, size_t to) {
      if (from != to && from < n && to < n) {
        adj[from].push_back(to);
        ++inDegree[to];
      }
    };

    for (size_t i = 0; i < n; ++i) {
      for (size_t j : passes_[i].depIndices) addEdge(j, i);
    }
    // 从 DeclareRead/DeclareWrite 推导资源依赖边：writer -> reader
    std::unordered_map<uint64_t, std::vector<size_t>> readers, writers;
    for (size_t i = 0; i < n; ++i) {
      for (uint64_t r : passes_[i].readResources) readers[r].push_back(i);
      for (uint64_t r : passes_[i].writeResources) writers[r].push_back(i);
    }
    for (auto const& kv : writers) {
      auto it = readers.find(kv.first);
      if (it == readers.end()) continue;
      for (size_t w : kv.second)
        for (size_t r : it->second) addEdge(w, r);
    }
    // 资源生命周期（Transient, ReleaseAfterPass）与 RHI ResourceBarrier 协同由 020 实现；首版此处不扩展

    // Kahn 拓扑排序 + 环检测
    std::vector<size_t> stack;
    for (size_t i = 0; i < n; ++i)
      if (inDegree[i] == 0) stack.push_back(i);
    sortedIndices_.reserve(n);
    while (!stack.empty()) {
      size_t u = stack.back();
      stack.pop_back();
      sortedIndices_.push_back(u);
      for (size_t v : adj[u]) {
        if (--inDegree[v] == 0) stack.push_back(v);
      }
    }
    if (sortedIndices_.size() != n) return false;  // 环
#if TE_PIPELINECORE_PROFILING
    if (g_onCompileProfiling) {
      auto dt = std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::steady_clock::now() - t0).count();
      g_onCompileProfiling(static_cast<uint64_t>(dt));
    }
#endif
    return true;
  }

  size_t GetPassCount() const override { return sortedIndices_.size(); }
  void GetPassCollectConfig(size_t executionOrder, PassCollectConfig* out) const override {
    if (!out || executionOrder >= sortedIndices_.size()) return;
    size_t idx = sortedIndices_[executionOrder];
    PassData const& p = passes_[idx];
    out->scene = p.scene;
    out->cullMode = p.cullMode;
    out->renderType = p.renderType;
    out->output = p.output;
    out->passKind = p.passKind;
    out->contentSource = p.contentSource;
    out->colorAttachmentCount = p.colorAttachmentCount;
    for (uint32_t i = 0; i < p.colorAttachmentCount && i < kMaxPassColorAttachments; ++i)
      out->colorAttachments[i] = p.colorAttachments[i];
    out->hasDepthStencil = p.hasDepthStencil;
    out->depthStencilAttachment = p.depthStencilAttachment;
    out->passName = p.name.empty() ? nullptr : p.name.c_str();
    out->materialName = p.materialName.empty() ? nullptr : p.materialName.c_str();
    out->meshName = p.meshName.empty() ? nullptr : p.meshName.c_str();
    out->readResourceCount = static_cast<uint32_t>(std::min(p.readResources.size(), size_t{kMaxPassReadResources}));
    for (uint32_t i = 0; i < out->readResourceCount; ++i)
      out->readResourceIds[i] = p.readResources[i];
  }

  void ExecutePass(size_t executionOrder, PassContext& ctx, te::rhi::ICommandList* cmd) override {
    if (executionOrder >= sortedIndices_.size() || !cmd) return;
    size_t idx = sortedIndices_[executionOrder];
    PassExecuteCallback cb = passes_[idx].executeCallback;
    if (cb) cb(ctx, cmd);
  }

 private:
  std::vector<PassData> passes_;
  std::vector<std::unique_ptr<IPassBuilder>> builders_;
  std::vector<size_t> sortedIndices_;
};

}  // namespace

IFrameGraph* CreateFrameGraph() { return new FrameGraphImpl(); }
void DestroyFrameGraph(IFrameGraph* g) { delete g; }

}  // namespace te::pipelinecore
