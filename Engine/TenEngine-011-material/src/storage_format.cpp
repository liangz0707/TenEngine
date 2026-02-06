// 011-Material storage format: magic TEMAT, version, shader path, uniform block, texture slots
#include "te/material/storage_format.hpp"
#include <cstring>
#include <fstream>
#include <vector>

namespace {

constexpr char const kMagic[] = "TEMAT";
constexpr size_t kMagicLen = 5;
constexpr uint32_t kCurrentVersion = 1;

}  // namespace

namespace te {
namespace material {

bool ParseMaterialMemory(void const* data, size_t size, ParsedMaterial& out) {
  out = {};
  if (!data || size < kMagicLen + sizeof(uint32_t) * 2) return false;
  char const* p = static_cast<char const*>(data);
  if (std::memcmp(p, kMagic, kMagicLen) != 0) return false;
  p += kMagicLen;
  size -= kMagicLen;

  uint32_t version = 0;
  if (size < sizeof(version)) return false;
  std::memcpy(&version, p, sizeof(version));
  p += sizeof(version);
  size -= sizeof(version);
  if (version != kCurrentVersion) return false;

  uint32_t shaderPathLen = 0;
  if (size < sizeof(shaderPathLen)) return false;
  std::memcpy(&shaderPathLen, p, sizeof(shaderPathLen));
  p += sizeof(shaderPathLen);
  size -= sizeof(shaderPathLen);
  if (size < shaderPathLen) return false;
  out.shaderPath.assign(p, p + shaderPathLen);
  p += shaderPathLen;
  size -= shaderPathLen;

  uint32_t uniformBlockSize = 0;
  if (size < sizeof(uniformBlockSize)) return false;
  std::memcpy(&uniformBlockSize, p, sizeof(uniformBlockSize));
  p += sizeof(uniformBlockSize);
  size -= sizeof(uniformBlockSize);
  if (size < uniformBlockSize) return false;
  out.defaultValues.assign(p, p + uniformBlockSize);
  p += uniformBlockSize;
  size -= uniformBlockSize;

  uint32_t textureSlotCount = 0;
  if (size < sizeof(textureSlotCount)) return false;
  std::memcpy(&textureSlotCount, p, sizeof(textureSlotCount));
  p += sizeof(textureSlotCount);
  size -= sizeof(textureSlotCount);
  out.textureSlots.resize(textureSlotCount);
  for (uint32_t i = 0; i < textureSlotCount; ++i) {
    if (size < sizeof(uint32_t) * 3) return false;
    std::memcpy(&out.textureSlots[i].slot.set, p, sizeof(uint32_t)); p += sizeof(uint32_t);
    std::memcpy(&out.textureSlots[i].slot.binding, p, sizeof(uint32_t)); p += sizeof(uint32_t);
    uint32_t pathLen = 0;
    std::memcpy(&pathLen, p, sizeof(pathLen)); p += sizeof(pathLen);
    size -= sizeof(uint32_t) * 3;
    if (size < pathLen) return false;
    out.textureSlots[i].path.assign(p, p + pathLen);
    p += pathLen;
    size -= pathLen;
  }
  return true;
}

bool ParseMaterialFile(char const* path, ParsedMaterial& out) {
  std::ifstream f(path, std::ios::binary | std::ios::ate);
  if (!f) return false;
  std::vector<char> buf(static_cast<size_t>(f.tellg()));
  f.seekg(0);
  if (!f.read(buf.data(), static_cast<std::streamsize>(buf.size()))) return false;
  return ParseMaterialMemory(buf.data(), buf.size(), out);
}

}  // namespace material
}  // namespace te
