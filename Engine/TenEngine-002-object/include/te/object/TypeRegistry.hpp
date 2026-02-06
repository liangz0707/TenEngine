/** @file TypeRegistry.hpp
 *  ABI: specs/_contracts/002-object-ABI.md
 */
#ifndef TE_OBJECT_TYPE_REGISTRY_HPP
#define TE_OBJECT_TYPE_REGISTRY_HPP

#include <te/object/TypeDescriptor.hpp>
#include <te/object/TypeId.hpp>

namespace te {
namespace object {

struct TypeRegistry {
  static bool RegisterType(TypeDescriptor const& desc);
  static TypeDescriptor const* GetTypeByName(char const* name);
  static TypeDescriptor const* GetTypeById(TypeId id);
  static void* CreateInstance(TypeId id);
};

}  // namespace object
}  // namespace te

#endif  // TE_OBJECT_TYPE_REGISTRY_HPP
