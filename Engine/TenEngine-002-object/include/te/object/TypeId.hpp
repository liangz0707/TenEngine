/** @file TypeId.hpp
 *  ABI: specs/_contracts/002-object-ABI.md
 */
#ifndef TE_OBJECT_TYPE_ID_HPP
#define TE_OBJECT_TYPE_ID_HPP

#include <cstdint>

namespace te {
namespace object {

using TypeId = uint32_t;

constexpr TypeId kInvalidTypeId = 0;

/** Method descriptor placeholder per ABI (minimal struct). */
struct MethodDescriptor {
  char const* name = nullptr;
};

}  // namespace object
}  // namespace te

#endif  // TE_OBJECT_TYPE_ID_HPP
