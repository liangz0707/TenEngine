/**
 * @file check.h
 * @brief CheckWarning, CheckError macros (contract: specs/_contracts/001-core-ABI.md).
 * Only contract-declared macros are exposed.
 */
#ifndef TE_CORE_CHECK_H
#define TE_CORE_CHECK_H

#include "te/core/log.h"
#include <cstdlib>

namespace te {
namespace core {

namespace detail {
inline void CheckWarningImpl(char const* message) {
  Log(LogLevel::Warn, message ? message : "Check failed");
}
inline void CheckErrorImpl(char const* message) {
  Log(LogLevel::Error, message ? message : "Check failed");
  std::abort();
}
}  // namespace detail

}  // namespace core
}  // namespace te

#define TE_CHECK_GET_MACRO(_1, _2, NAME, ...) NAME
/** If condition is false, log Warning (optional message). */
#define CheckWarning1(c) do { if (!(c)) te::core::detail::CheckWarningImpl(nullptr); } while (0)
#define CheckWarning2(c, m) do { if (!(c)) te::core::detail::CheckWarningImpl(m); } while (0)
#define CheckWarning(...) TE_CHECK_GET_MACRO(__VA_ARGS__, CheckWarning2, CheckWarning1)(__VA_ARGS__)

/** If condition is false, log Error and abort (optional message). */
#define CheckError1(c) do { if (!(c)) te::core::detail::CheckErrorImpl(nullptr); } while (0)
#define CheckError2(c, m) do { if (!(c)) te::core::detail::CheckErrorImpl(m); } while (0)
#define CheckError(...) TE_CHECK_GET_MACRO(__VA_ARGS__, CheckError2, CheckError1)(__VA_ARGS__)

#endif  // TE_CORE_CHECK_H
