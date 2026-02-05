/**
 * @file test_log.cpp
 * @brief Unit tests for Log, LogLevel, LogSink, Assert, CrashHandler per contract capability 4.
 */

#include "te/core/log.h"
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

using namespace te::core;

namespace {

std::vector<std::string> g_captured;

class TestSink : public LogSink {
 public:
  void Write(LogLevel level, char const* message) override {
    g_captured.push_back(std::string(message));
  }
};

}  // namespace

int main() {
  // Log with default sink (no crash)
  Log(LogLevel::Debug, "debug msg");
  Log(LogLevel::Info, "info msg");
  Log(LogLevel::Warn, "warn msg");
  Log(LogLevel::Error, "error msg");

  // Level filter: only Info and above
  LogSetLevelFilter(LogLevel::Info);
  g_captured.clear();
  TestSink sink;
  LogSetSink(&sink);
  Log(LogLevel::Debug, "dropped");
  Log(LogLevel::Info, "kept");
  LogSetSink(nullptr);
  assert(g_captured.size() == 1u && g_captured[0] == "kept");

  // Stderr threshold (behavior: >= level to stderr; we only verify API)
  LogSetStderrThreshold(LogLevel::Error);

  // CrashHandler: set and call on assert failure (we only test set + Assert(true))
  static bool handler_called = false;
  auto handler = [](char const*) { handler_called = true; };
  SetCrashHandler(+handler);  // decay to function pointer (no capture)
  Assert(true);
  assert(!handler_called);
  SetCrashHandler(nullptr);

  return 0;
}
