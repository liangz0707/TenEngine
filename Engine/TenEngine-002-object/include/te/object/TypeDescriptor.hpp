/** @file TypeDescriptor.hpp
 *  ABI: specs/_contracts/002-object-ABI.md
 */
#ifndef TE_OBJECT_TYPE_DESCRIPTOR_HPP
#define TE_OBJECT_TYPE_DESCRIPTOR_HPP

#include <cstddef>
#include <te/object/TypeId.hpp>
#include <te/object/PropertyDescriptor.hpp>

namespace te {
namespace object {

struct TypeDescriptor {
  TypeId id = kInvalidTypeId;
  char const* name = nullptr;
  size_t size = 0;
  PropertyDescriptor const* properties = nullptr;
  size_t propertyCount = 0;
  MethodDescriptor const* methods = nullptr;
  size_t methodCount = 0;
  TypeId baseTypeId = kInvalidTypeId;
};

}  // namespace object
}  // namespace te

#endif  // TE_OBJECT_TYPE_DESCRIPTOR_HPP
