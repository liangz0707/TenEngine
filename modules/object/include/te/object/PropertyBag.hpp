/** PropertyBag (contract: 002-object-public-api.md fullversion-001) */

#ifndef TE_OBJECT_PROPERTY_BAG_HPP
#define TE_OBJECT_PROPERTY_BAG_HPP

#include "PropertyDescriptor.hpp"

namespace te::object {

/** Property bag: GetProperty, SetProperty, FindProperty (contract). */
class PropertyBag {
public:
    virtual ~PropertyBag() = default;
    virtual bool GetProperty(void* outValue, char const* name) const = 0;
    virtual bool SetProperty(void const* value, char const* name) = 0;
    virtual PropertyDescriptor const* FindProperty(char const* name) const = 0;
};

}  // namespace te::object

#endif
