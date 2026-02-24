// 011-Material types per contracts/011-material-ABI-full.md
#ifndef TE_MATERIAL_TYPES_HPP
#define TE_MATERIAL_TYPES_HPP

#include <cstdint>
#include <cstddef>

namespace te {
namespace material {

struct MaterialHandle {
  uint64_t id{0};
  bool IsValid() const { return id != 0; }
};

struct MaterialInstanceHandle {
  uint64_t id{0};
  bool IsValid() const { return id != 0; }
};

struct ParameterSlot {
  char const* name{nullptr};       // Parameter name for scalar/vector params
  uint32_t type{0};                // UniformMemberType value
  uint32_t count{1};               // Element count (0 or 1 = scalar)
  uint32_t set{0};                 // Descriptor set for textures
  uint32_t binding{0};             // Binding slot for textures
};

}  // namespace material
}  // namespace te

#endif
