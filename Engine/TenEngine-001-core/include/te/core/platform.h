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

#if defined(__ANDROID__)
#define TE_PLATFORM_WINDOWS 0
#define TE_PLATFORM_LINUX 0
#define TE_PLATFORM_MACOS 0
#define TE_PLATFORM_ANDROID 1
#define TE_PLATFORM_IOS 0
#elif defined(__APPLE__) && defined(__MACH__)
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE
#define TE_PLATFORM_WINDOWS 0
#define TE_PLATFORM_LINUX 0
#define TE_PLATFORM_MACOS 0
#define TE_PLATFORM_ANDROID 0
#define TE_PLATFORM_IOS 1
#else
#define TE_PLATFORM_WINDOWS 0
#define TE_PLATFORM_LINUX 0
#define TE_PLATFORM_MACOS 1
#define TE_PLATFORM_ANDROID 0
#define TE_PLATFORM_IOS 0
#endif
#elif defined(__linux__)
#define TE_PLATFORM_WINDOWS 0
#define TE_PLATFORM_LINUX 1
#define TE_PLATFORM_MACOS 0
#define TE_PLATFORM_ANDROID 0
#define TE_PLATFORM_IOS 0
#elif defined(_WIN32) || defined(_WIN64)
#define TE_PLATFORM_WINDOWS 1
#define TE_PLATFORM_LINUX 0
#define TE_PLATFORM_MACOS 0
#define TE_PLATFORM_ANDROID 0
#define TE_PLATFORM_IOS 0
#else
#define TE_PLATFORM_WINDOWS 0
#define TE_PLATFORM_LINUX 0
#define TE_PLATFORM_MACOS 0
#define TE_PLATFORM_ANDROID 0
#define TE_PLATFORM_IOS 0
#endif

namespace te {
namespace core {

/** Read entire file into bytes. Returns empty optional on failure. */
std::optional<std::vector<std::uint8_t>> FileRead(std::string const& path);

/** Write bytes to file. Returns false on failure. */
bool FileWrite(std::string const& path, std::vector<std::uint8_t> const& data);

/** Write string to file. Returns false on failure. */
bool FileWrite(std::string const& path, std::string const& data);

/** Read file binary data at specified offset and size. outData must be pre-allocated by caller, outSize returns actual bytes read. Returns false on failure. */
bool FileReadBinary(std::string const& path, void* outData, std::size_t* outSize, std::size_t offset, std::size_t size);

/** Write file binary data at specified offset. If offset is SIZE_MAX, append to end of file. Returns false on failure. */
bool FileWriteBinary(std::string const& path, void const* data, std::size_t size, std::size_t offset);

/** Get file size in bytes. Returns 0 on failure. */
std::size_t FileGetSize(std::string const& path);

/** Check if file exists. Returns true if file exists. */
bool FileExists(std::string const& path);

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

/** Join path components, automatically handling separators. */
std::string PathJoin(std::string const& path1, std::string const& path2);

/** Get directory part of path (without filename). */
std::string PathGetDirectory(std::string const& path);

/** Get filename part of path (with extension). */
std::string PathGetFileName(std::string const& path);

/** Get extension of path (with dot, e.g., ".txt"). */
std::string PathGetExtension(std::string const& path);

/** Resolve relative path to absolute path based on basePath. */
std::string PathResolveRelative(std::string const& basePath, std::string const& relativePath);

}  // namespace core
}  // namespace te

#endif  // TE_CORE_PLATFORM_H
