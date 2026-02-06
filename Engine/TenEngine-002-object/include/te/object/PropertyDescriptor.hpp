/** @file PropertyDescriptor.hpp
 *  ABI: specs/_contracts/002-object-ABI.md
 */
#ifndef TE_OBJECT_PROPERTY_DESCRIPTOR_HPP
#define TE_OBJECT_PROPERTY_DESCRIPTOR_HPP

#include <te/object/TypeId.hpp>

namespace te {
namespace object {

struct PropertyDescriptor {
  char const* name = nullptr;
  TypeId valueTypeId = kInvalidTypeId;
  void const* defaultValue = nullptr;  // optional / opaque
};

}  // namespace object
}  // namespace te

#endif  // TE_OBJECT_PROPERTY_DESCRIPTOR_HPP
