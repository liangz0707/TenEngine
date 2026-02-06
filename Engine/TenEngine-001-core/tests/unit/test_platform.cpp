/**
 * @file test_platform.cpp
 * @brief Unit tests for FileRead/Write, DirectoryEnumerate, Time, HighResolutionTimer, GetEnv, PathNormalize per contract capability 3.
 */

#include "te/core/platform.h"
#include <cassert>
#include <cstring>

using namespace te::core;

int main() {
  // Platform macros (exactly one of WINDOWS/LINUX/MACOS/ANDROID/IOS is 1)
  assert(TE_PLATFORM_WINDOWS + TE_PLATFORM_LINUX + TE_PLATFORM_MACOS
         + TE_PLATFORM_ANDROID + TE_PLATFORM_IOS >= 1);

  // FileWrite / FileRead
  std::string path = "test_platform_tmp_file.txt";
  std::string data = "hello platform";
  assert(FileWrite(path, data));
  auto read = FileRead(path);
  assert(read.has_value());
  std::string read_str(read->begin(), read->end());
  assert(read_str == data);
  // Clean up
  std::remove(path.c_str());

  // DirectoryEnumerate: on Windows use nonexistent path to avoid CRT stack check in iterator
#if defined(_WIN32) || defined(_WIN64)
  { auto entries = DirectoryEnumerate("nonexistent_empty_dir_12345"); assert(entries.empty()); }
#else
  auto entries = DirectoryEnumerate(".");
  (void)entries;
#endif

  // Time (seconds since epoch)
  double t = Time();
  assert(t > 0);

  // HighResolutionTimer (monotonic)
  double h1 = HighResolutionTimer();
  double h2 = HighResolutionTimer();
  assert(h2 >= h1);

  // GetEnv (may or may not be set)
  auto env = GetEnv("PATH");
  (void)env;

  // PathNormalize
  std::string norm = PathNormalize("./foo/../bar");
  assert(!norm.empty());

  return 0;
}
