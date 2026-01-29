/**
 * @file platform.h
 * @brief File, Dir, Time, GetEnv, PathNormalize, platform macros (contract: 001-core-public-api.md).
 * Only contract-declared types and API are exposed.
 */
#ifndef TE_CORE_PLATFORM_H
#define TE_CORE_PLATFORM_H

#include <chrono>
#include <cstdint>
#include <string>
#include <vector>
#include <optional>

#if defined(_WIN32) || defined(_WIN64)
#define TE_PLATFORM_WINDOWS 1
#define TE_PLATFORM_LINUX 0
#define TE_PLATFORM_MACOS 0
#elif defined(__APPLE__) && defined(__MACH__)
#define TE_PLATFORM_WINDOWS 0
#define TE_PLATFORM_LINUX 0
#define TE_PLATFORM_MACOS 1
#elif defined(__linux__)
#define TE_PLATFORM_WINDOWS 0
#define TE_PLATFORM_LINUX 1
#define TE_PLATFORM_MACOS 0
#else
#define TE_PLATFORM_WINDOWS 0
#define TE_PLATFORM_LINUX 0
#define TE_PLATFORM_MACOS 0
#endif

namespace te {
namespace core {

/** Read entire file into bytes. Returns empty optional on failure. */
std::optional<std::vector<std::uint8_t>> FileRead(std::string const& path);

/** Write bytes to file. Returns false on failure. */
bool FileWrite(std::string const& path, std::vector<std::uint8_t> const& data);

/** Write string to file. Returns false on failure. */
bool FileWrite(std::string const& path, std::string const& data);

/** Directory entry name (filename or subdir name). */
using DirEntry = std::string;

/** Enumerate directory entries (names only). Returns empty vector on failure. */
std::vector<DirEntry> DirectoryEnumerate(std::string const& path);

/** Wall-clock time in seconds since epoch. */
double Time();

/** High-resolution timer in seconds (monotonic). */
double HighResolutionTimer();

/** Get environment variable. Returns empty optional if not set. */
std::optional<std::string> GetEnv(std::string const& name);

/** Normalize path (resolve . and .., separators). */
std::string PathNormalize(std::string const& path);

}  // namespace core
}  // namespace te

#endif  // TE_CORE_PLATFORM_H
