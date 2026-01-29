/**
 * @file platform.cpp
 * @brief Implementation of FileRead/Write, DirectoryEnumerate, Time, HighResolutionTimer, GetEnv, PathNormalize per contract (001-core-public-api.md).
 * Uses C++17 std::filesystem and std::chrono. Comments in English.
 */

#include "te/core/platform.h"
#include <chrono>
#include <cstdlib>
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
  f.seekg(0);
  std::vector<std::uint8_t> buf(static_cast<std::size_t>(size));
  if (!f.read(reinterpret_cast<char*>(buf.data()), size)) return std::nullopt;
  return buf;
}

bool FileWrite(std::string const& path, std::vector<std::uint8_t> const& data) {
  std::ofstream f(path, std::ios::binary);
  if (!f) return false;
  return static_cast<bool>(f.write(reinterpret_cast<char const*>(data.data()), data.size()));
}

bool FileWrite(std::string const& path, std::string const& data) {
  std::ofstream f(path);
  if (!f) return false;
  return static_cast<bool>(f << data);
}

std::vector<DirEntry> DirectoryEnumerate(std::string const& path) {
  std::vector<DirEntry> out;
  std::error_code ec;
  for (auto const& e : fs::directory_iterator(path, ec)) {
    if (ec) return {};
    out.push_back(e.path().filename().string());
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
  return fs::path(path).lexically_normal().string();
}

}  // namespace core
}  // namespace te
