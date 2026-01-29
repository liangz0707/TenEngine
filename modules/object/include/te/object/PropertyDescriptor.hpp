/** PropertyDescriptor (contract: 002-object-public-api.md fullversion-001) */

#ifndef TE_OBJECT_PROPERTY_DESCRIPTOR_HPP
#define TE_OBJECT_PROPERTY_DESCRIPTOR_HPP

#include "TypeId.hpp"

namespace te::object {

/** Property description: name, type, optional default (contract API sketch). */
struct PropertyDescriptor {
    char const* name;
    TypeId      valueTypeId;
    void const* defaultValue;  // optional; null if none
};

}  // namespace te::object

#endif
