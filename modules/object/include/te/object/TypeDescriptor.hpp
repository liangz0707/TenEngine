/** TypeDescriptor (contract: 002-object-public-api.md fullversion-001) */

#ifndef TE_OBJECT_TYPE_DESCRIPTOR_HPP
#define TE_OBJECT_TYPE_DESCRIPTOR_HPP

#include <cstddef>
#include "PropertyDescriptor.hpp"
#include "TypeId.hpp"

namespace te::object {

struct MethodDescriptor;

/** Type description: id, name, size, properties, methods, base type (contract API sketch). */
struct TypeDescriptor {
    TypeId    id;
    char const* name;
    size_t    size;
    PropertyDescriptor const* properties;
    size_t    propertyCount;
    MethodDescriptor const* methods;
    size_t    methodCount;
    TypeId    baseTypeId;  // 0 or kInvalidTypeId if no base
};

}  // namespace te::object

#endif
