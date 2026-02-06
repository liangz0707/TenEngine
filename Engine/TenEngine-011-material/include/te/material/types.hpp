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
  uint32_t set{0};
  uint32_t binding{0};
};

}  // namespace material
}  // namespace te

#endif
