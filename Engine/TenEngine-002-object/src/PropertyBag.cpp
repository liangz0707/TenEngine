/**
 * Default PropertyBag implementation backed by TypeDescriptor.
 * ABI: specs/_contracts/002-object-ABI.md
 */

#include <te/object/PropertyBag.hpp>
#include <te/object/TypeDescriptor.hpp>
#include <te/object/PropertyDescriptor.hpp>
#include <cstring>

namespace te {
namespace object {

namespace {

class DefaultPropertyBag : public PropertyBag {
 public:
  DefaultPropertyBag(TypeDescriptor const* desc, void* instance)
      : desc_(desc), instance_(instance) {}
  bool GetProperty(void* outValue, char const* name) const override {
    PropertyDescriptor const* p = FindProperty(name);
    if (!p || !outValue || !instance_) return false;
    size_t offset = 0;
    for (size_t i = 0; i < desc_->propertyCount; ++i) {
      if (std::strcmp(desc_->properties[i].name, name) == 0) {
        std::memcpy(outValue, static_cast<char*>(instance_) + offset, SizeForProperty(&desc_->properties[i]));
        return true;
      }
      offset += SizeForProperty(&desc_->properties[i]);
    }
    return false;
  }
  bool SetProperty(void const* value, char const* name) override {
    PropertyDescriptor const* p = FindProperty(name);
    if (!p || !value || !instance_) return false;
    size_t offset = 0;
    for (size_t i = 0; i < desc_->propertyCount; ++i) {
      if (std::strcmp(desc_->properties[i].name, name) == 0) {
        std::memcpy(static_cast<char*>(instance_) + offset, value, SizeForProperty(&desc_->properties[i]));
        return true;
      }
      offset += SizeForProperty(&desc_->properties[i]);
    }
    return false;
  }
  PropertyDescriptor const* FindProperty(char const* name) const override {
    if (!desc_ || !name) return nullptr;
    for (size_t i = 0; i < desc_->propertyCount; ++i) {
      if (std::strcmp(desc_->properties[i].name, name) == 0)
        return &desc_->properties[i];
    }
    return nullptr;
  }

 private:
  static size_t SizeForProperty(PropertyDescriptor const* p) {
    (void)p;
    return sizeof(void*);  // simplified: assume pointer-sized slot per property
  }
  TypeDescriptor const* desc_;
  void* instance_;
};

}  // namespace

}  // namespace object
}  // namespace te
