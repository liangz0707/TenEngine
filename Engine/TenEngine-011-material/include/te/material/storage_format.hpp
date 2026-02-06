// 011-Material storage format parser (engine-owned binary); per data-model.md §3
#ifndef TE_MATERIAL_STORAGE_FORMAT_HPP
#define TE_MATERIAL_STORAGE_FORMAT_HPP

#include "te/material/types.hpp"
#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>

namespace te {
namespace material {

struct ParsedMaterial {
  std::string shaderPath;
  std::vector<uint8_t> defaultValues;
  struct TextureSlot {
    ParameterSlot slot;
    std::string path;
  };
  std::vector<TextureSlot> textureSlots;
};

// Parse from file path; returns false and leaves out empty on failure.
bool ParseMaterialFile(char const* path, ParsedMaterial& out);

// Parse from memory (size bytes); returns false on failure.
bool ParseMaterialMemory(void const* data, size_t size, ParsedMaterial& out);

}  // namespace material
}  // namespace te

#endif
