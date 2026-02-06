/** @file ObjectRef.hpp
 *  ABI: specs/_contracts/002-object-ABI.md
 */
#ifndef TE_OBJECT_OBJECT_REF_HPP
#define TE_OBJECT_OBJECT_REF_HPP

#include <cstdint>

namespace te {
namespace object {

struct ObjectRef {
  uint8_t guid[16] = {};
};

}  // namespace object
}  // namespace te

#endif  // TE_OBJECT_OBJECT_REF_HPP
