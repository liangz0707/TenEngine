#include <te/pipelinecore/Profiling.h>

#if TE_PIPELINECORE_PROFILING
#include <chrono>
namespace te::pipelinecore {

OnCompileProfiling g_onCompileProfiling = nullptr;

PassProfilingScope::PassProfilingScope(char const* passName)
    : passName_(passName),
      startTick_(std::chrono::steady_clock::now().time_since_epoch().count()) {}

PassProfilingScope::~PassProfilingScope() {
  (void)passName_;
  (void)startTick_;
}

}  // namespace te::pipelinecore
#endif
