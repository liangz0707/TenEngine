/**
 * @file log.cpp
 * @brief Implementation of Log, LogSink, Assert, CrashHandler per contract (001-core-public-api.md).
 * Thread-safe; single message atomic. Comments in English.
 */

#include "te/core/log.h"
#include "te/core/thread.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>

namespace te {
namespace core {

namespace {

Mutex g_log_mutex;
LogLevel g_level_filter = LogLevel::Debug;
LogLevel g_stderr_threshold = LogLevel::Warn;
LogSink* g_sink = nullptr;
CrashHandlerFn g_crash_handler = nullptr;

void DefaultWrite(LogLevel level, char const* message) {
  bool to_stderr = static_cast<unsigned>(level) >= static_cast<unsigned>(g_stderr_threshold);
  FILE* f = to_stderr ? stderr : stdout;
  std::fprintf(f, "%s\n", message ? message : "");
  std::fflush(f);
}

}  // namespace

void Log(LogLevel level, char const* message) {
  if (static_cast<unsigned>(level) < static_cast<unsigned>(g_level_filter)) return;
  LockGuard lock(g_log_mutex);
  if (g_sink) g_sink->Write(level, message);
  else DefaultWrite(level, message);
}

void LogSetLevelFilter(LogLevel min_level) {
  LockGuard lock(g_log_mutex);
  g_level_filter = min_level;
}

void LogSetStderrThreshold(LogLevel stderr_level) {
  LockGuard lock(g_log_mutex);
  g_stderr_threshold = stderr_level;
}

void LogSetSink(LogSink* sink) {
  LockGuard lock(g_log_mutex);
  g_sink = sink;
}

void Assert(bool condition) {
  if (condition) return;
  if (g_crash_handler) g_crash_handler("Assert failed");
  std::abort();
}

void SetCrashHandler(CrashHandlerFn fn) {
  g_crash_handler = fn;
}

}  // namespace core
}  // namespace te
