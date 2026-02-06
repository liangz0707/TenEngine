#pragma once

/// 定义此宏以启用 Pass 执行耗时、Compile 耗时等 profiling 钩子
#ifndef TE_PIPELINECORE_PROFILING
#define TE_PIPELINECORE_PROFILING 0
#endif

#if TE_PIPELINECORE_PROFILING
#include <cstdint>
#endif

namespace te::pipelinecore {

#if TE_PIPELINECORE_PROFILING
/// RAII：Pass 开始/结束计时
struct PassProfilingScope {
  PassProfilingScope(char const* passName);
  ~PassProfilingScope();
  char const* passName_;
  uint64_t startTick_;
};

/// Compile 耗时回调；可由调用方注册
using OnCompileProfiling = void (*)(uint64_t compileTimeMicros);
extern OnCompileProfiling g_onCompileProfiling;
#else
struct PassProfilingScope {
  explicit PassProfilingScope(char const* /*passName*/) {}
};
#endif

}  // namespace te::pipelinecore
