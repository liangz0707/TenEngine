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
  ISceneWorld const* scene{nullptr};
  CullMode cullMode{CullMode::None};
  RenderType renderType{RenderType::Opaque};
  PassOutputDesc output{};
  PassExecuteCallback executeCallback{nullptr};
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

 private:
  PassData& data_;
};

class FrameGraphImpl : public IFrameGraph {
 public:
  IPassBuilder* AddPass(char const* name) override {
    passes_.emplace_back();
    passes_.back().name = name ? name : "";
    builders_.push_back(std::make_unique<PassBuilderImpl>(passes_.back()));
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
    out->scene = passes_[idx].scene;
    out->cullMode = passes_[idx].cullMode;
    out->renderType = passes_[idx].renderType;
    out->output = passes_[idx].output;
  }

  void ExecutePass(size_t executionOrder, PassContext& ctx, te::rhi::ICommandList* cmd) override {
    if (executionOrder >= sortedIndices_.size() || !cmd) return;
    size_t idx = sortedIndices_[executionOrder];
    PassExecuteCallback cb = passes_[idx].executeCallback;
    if (cb) cb(ctx, cmd);
  }

 private:
  std::vector<PassData> passes_;
  std::vector<std::unique_ptr<PassBuilderImpl>> builders_;
  std::vector<size_t> sortedIndices_;
};

}  // namespace

IFrameGraph* CreateFrameGraph() { return new FrameGraphImpl(); }
void DestroyFrameGraph(IFrameGraph* g) { delete g; }

}  // namespace te::pipelinecore
