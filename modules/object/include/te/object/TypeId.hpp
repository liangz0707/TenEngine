/** TypeId and MethodDescriptor (contract: 002-object-public-api.md fullversion-001) */

#ifndef TE_OBJECT_TYPEID_HPP
#define TE_OBJECT_TYPEID_HPP

#include <cstddef>
#include <cstdint>

namespace te::object {

/** Type unique identifier (contract: uint32_t or opaque handle). */
using TypeId = uint32_t;

/** Invalid TypeId sentinel (e.g. no base type). */
constexpr TypeId kInvalidTypeId = 0;

/** Minimal/placeholder method descriptor for full slice (contract: methods, methodCount). */
struct MethodDescriptor {
    char const* name;
    void const*  signatureOrPlaceholder;  // optional; contract allows minimal/placeholder
};

}  // namespace te::object

#endif
