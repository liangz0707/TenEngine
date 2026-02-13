/**
 * @file platform.cpp
 * @brief Implementation of FileRead/Write, DirectoryEnumerate, Time, HighResolutionTimer, GetEnv, PathNormalize per contract (001-core-public-api.md).
 * Uses C++17 std::filesystem and std::chrono. Comments in English.
 */

#include "te/core/platform.h"
#include <chrono>
#include <cstdlib>
#include <climits>
#include <fstream>
#include <filesystem>
#include <system_error>

namespace te {
namespace core {

namespace fs = std::filesystem;

std::optional<std::vector<std::uint8_t>> FileRead(std::string const& path) {
  std::ifstream f(path, std::ios::binary | std::ios::ate);
  if (!f) return std::nullopt;
  auto size = f.tellg();
  if (size <= 0) return std::nullopt;
  f.seekg(0);
  std::vector<std::uint8_t> buf(static_cast<std::size_t>(size));
  if (!f.read(reinterpret_cast<char*>(buf.data()), size)) return std::nullopt;
  return buf;
}

static bool EnsureParentDirectoryExists(std::string const& path) {
  std::string dir = fs::path(path).parent_path().generic_string();
  if (dir.empty()) return true;
  std::error_code ec;
  fs::create_directories(dir, ec);
  return !ec;
}

bool FileWrite(std::string const& path, std::vector<std::uint8_t> const& data) {
  if (!EnsureParentDirectoryExists(path)) return false;
  std::ofstream f(path, std::ios::binary);
  if (!f) return false;
  return static_cast<bool>(f.write(reinterpret_cast<char const*>(data.data()), data.size()));
}

bool FileWrite(std::string const& path, std::string const& data) {
  if (!EnsureParentDirectoryExists(path)) return false;
  std::ofstream f(path);
  if (!f) return false;
  return static_cast<bool>(f << data);
}

std::vector<DirEntry> DirectoryEnumerate(std::string const& path) {
  std::vector<DirEntry> out;
  std::error_code ec;
  fs::directory_iterator it(path, ec);
  if (ec) return {};
  for (; it != fs::directory_iterator(); ++it) {
    out.push_back(it->path().filename().generic_string());
  }
  return out;
}

double Time() {
  auto now = std::chrono::system_clock::now();
  return std::chrono::duration<double>(now.time_since_epoch()).count();
}

double HighResolutionTimer() {
  static auto start = std::chrono::steady_clock::now();
  auto now = std::chrono::steady_clock::now();
  return std::chrono::duration<double>(now - start).count();
}

std::optional<std::string> GetEnv(std::string const& name) {
  char const* v = std::getenv(name.c_str());
  if (!v) return std::nullopt;
  return std::string(v);
}

std::string PathNormalize(std::string const& path) {
  return fs::path(path).lexically_normal().generic_string();
}

bool FileReadBinary(std::string const& path, void* outData, std::size_t* outSize, std::size_t offset, std::size_t size) {
  std::ifstream f(path, std::ios::binary);
  if (!f) return false;
  f.seekg(static_cast<std::streamoff>(offset), std::ios::beg);
  if (!f) return false;
  if (!f.read(reinterpret_cast<char*>(outData), static_cast<std::streamsize>(size))) {
    *outSize = static_cast<std::size_t>(f.gcount());
    return false;
  }
  *outSize = size;
  return true;
}

bool FileWriteBinary(std::string const& path, void const* data, std::size_t size, std::size_t offset) {
  if (!EnsureParentDirectoryExists(path)) return false;
  std::ios_base::openmode mode = std::ios::binary;
  if (offset == SIZE_MAX) {
    mode |= std::ios::app;
  }
  std::ofstream f(path, mode);
  if (!f) return false;
  if (offset != SIZE_MAX) {
    f.seekp(static_cast<std::streamoff>(offset), std::ios::beg);
    if (!f) return false;
  }
  return static_cast<bool>(f.write(reinterpret_cast<char const*>(data), static_cast<std::streamsize>(size)));
}

std::size_t FileGetSize(std::string const& path) {
  std::error_code ec;
  auto size = fs::file_size(path, ec);
  if (ec) return 0;
  return static_cast<std::size_t>(size);
}

bool FileExists(std::string const& path) {
  std::error_code ec;
  return fs::exists(path, ec) && !ec;
}

std::string PathJoin(std::string const& path1, std::string const& path2) {
  return (fs::path(path1) / path2).generic_string();
}

std::string PathGetDirectory(std::string const& path) {
  return fs::path(path).parent_path().generic_string();
}

std::string PathGetFileName(std::string const& path) {
  return fs::path(path).filename().generic_string();
}

std::string PathGetExtension(std::string const& path) {
  auto ext = fs::path(path).extension().generic_string();
  return ext.empty() ? ext : ext;  // extension() already includes the dot
}

std::string PathResolveRelative(std::string const& basePath, std::string const& relativePath) {
  fs::path base(basePath);
  if (!base.is_absolute()) {
    base = fs::absolute(base);
  }
  return (base / relativePath).lexically_normal().generic_string();
}

}  // namespace core
}  // namespace te
