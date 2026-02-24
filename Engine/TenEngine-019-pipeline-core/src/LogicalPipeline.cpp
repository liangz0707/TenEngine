#include <te/pipelinecore/LogicalPipeline.h>
#include <te/pipelinecore/FrameGraph.h>
#include <memory>
#include <vector>

namespace te::pipelinecore {

namespace {

struct PassEntry {
  PassCollectConfig config;
};

class LogicalPipelineImpl : public ILogicalPipeline {
 public:
  explicit LogicalPipelineImpl(std::vector<PassEntry> passes) : passes_(std::move(passes)) {}
  size_t GetPassCount() const override { return passes_.size(); }
  void GetPassConfig(size_t index, PassCollectConfig* out) const override {
    if (!out || index >= passes_.size()) return;
    *out = passes_[index].config;
  }
  IRenderItemList const* GetSourceItemList() const override {
    // Returns nullptr by default; can be extended to hold source items
    return nullptr;
  }

 private:
  std::vector<PassEntry> passes_;
};

}  // namespace

ILogicalPipeline* BuildLogicalPipeline(IFrameGraph const* graph, FrameContext const& /*ctx*/) {
  if (!graph) return nullptr;
  size_t n = graph->GetPassCount();
  std::vector<PassEntry> passes;
  passes.reserve(n);
  for (size_t i = 0; i < n; ++i) {
    PassEntry e;
    graph->GetPassCollectConfig(i, &e.config);
    passes.push_back(std::move(e));
  }
  return new LogicalPipelineImpl(std::move(passes));
}

void DestroyLogicalPipeline(ILogicalPipeline* p) { delete p; }

}  // namespace te::pipelinecore
