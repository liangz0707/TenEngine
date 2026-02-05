/**
 * @file log.h
 * @brief LogLevel, LogSink, Log, Assert, CrashHandler (contract: 001-core-public-api.md).
 * Only contract-declared types and API are exposed.
 */
#ifndef TE_CORE_LOG_H
#define TE_CORE_LOG_H

#include <cstddef>

namespace te {
namespace core {

/** Log level per contract capability 4; extensible. */
enum class LogLevel : unsigned {
  Debug = 0,
  Info  = 1,
  Warn  = 2,
  Error = 3,
};

/** Output channel abstraction; redirectable and filterable per contract. */
class LogSink {
 public:
  virtual ~LogSink() = default;
  /** Write one message atomically. */
  virtual void Write(LogLevel level, char const* message) = 0;
};

/** Thread-safe log; single message atomic. Level filter and stderr threshold configurable. */
void Log(LogLevel level, char const* message);

/** Set minimum level to output (messages below are dropped). Default: Debug. */
void LogSetLevelFilter(LogLevel min_level);

/** Set level threshold for stderr (>= this level goes to stderr). Default: Warn. */
void LogSetStderrThreshold(LogLevel stderr_level);

/** Set custom sink; nullptr restores default (stdout/stderr). */
void LogSetSink(LogSink* sink);

/** Assert: if \a condition is false, call CrashHandler then abort. */
void Assert(bool condition);

/** Crash handler callback type: (message) -> void. */
using CrashHandlerFn = void (*)(char const* message);

/** Set crash report hook; called on Assert failure before abort. */
void SetCrashHandler(CrashHandlerFn fn);

}  // namespace core
}  // namespace te

#endif  // TE_CORE_LOG_H
