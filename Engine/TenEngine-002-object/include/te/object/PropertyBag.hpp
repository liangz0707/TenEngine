/** @file PropertyBag.hpp
 *  ABI: specs/_contracts/002-object-ABI.md
 */
#ifndef TE_OBJECT_PROPERTY_BAG_HPP
#define TE_OBJECT_PROPERTY_BAG_HPP

#include <te/object/PropertyDescriptor.hpp>

namespace te {
namespace object {

struct PropertyBag {
  virtual bool GetProperty(void* outValue, char const* name) const = 0;
  virtual bool SetProperty(void const* value, char const* name) = 0;
  virtual PropertyDescriptor const* FindProperty(char const* name) const = 0;
  virtual ~PropertyBag() = default;
};

}  // namespace object
}  // namespace te

#endif  // TE_OBJECT_PROPERTY_BAG_HPP
