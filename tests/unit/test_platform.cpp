/**
 * @file test_platform.cpp
 * @brief Unit tests for FileRead/Write, DirectoryEnumerate, Time, HighResolutionTimer, GetEnv, PathNormalize per contract capability 3.
 */

#include "te/core/platform.h"
#include <cassert>
#include <cstring>

using namespace te::core;

int main() {
  // Platform macros
  assert(TE_PLATFORM_WINDOWS + TE_PLATFORM_LINUX + TE_PLATFORM_MACOS >= 1);

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

  // DirectoryEnumerate (current dir should list something or be empty)
  auto entries = DirectoryEnumerate(".");
  (void)entries;

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
