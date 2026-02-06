/**
 * @file PropertyBag.cpp
 * @brief Implementation of PropertyBag (contract: specs/_contracts/002-object-public-api.md).
 * Property access and manipulation based on type descriptors.
 */

#include "te/object/PropertyBag.h"
#include "te/object/TypeRegistry.h"
#include <cstring>

namespace te {
namespace object {

PropertyBag::PropertyBag(void* instance, TypeDescriptor const* typeDesc)
    : instance_(instance), typeDesc_(typeDesc) {
}

bool PropertyBag::GetProperty(void* outValue, char const* name) const {
    if (!outValue || !name || !instance_ || !typeDesc_) {
        return false;
    }
    
    PropertyDescriptor const* prop = FindProperty(name);
    if (!prop) {
        return false;
    }
    
    void const* src = static_cast<char const*>(instance_) + prop->offset;
    std::memcpy(outValue, src, prop->size);
    return true;
}

bool PropertyBag::GetProperty(void* outValue, char const* name, TypeId typeId) const {
    if (!outValue || !name || !instance_ || !typeDesc_) {
        return false;
    }
    
    PropertyDescriptor const* prop = FindProperty(name);
    if (!prop || prop->valueTypeId != typeId) {
        return false;
    }
    
    void const* src = static_cast<char const*>(instance_) + prop->offset;
    std::memcpy(outValue, src, prop->size);
    return true;
}

bool PropertyBag::SetProperty(void const* value, char const* name) {
    if (!value || !name || !instance_ || !typeDesc_) {
        return false;
    }
    
    PropertyDescriptor const* prop = FindProperty(name);
    if (!prop) {
        return false;
    }
    
    void* dst = static_cast<char*>(instance_) + prop->offset;
    std::memcpy(dst, value, prop->size);
    return true;
}

bool PropertyBag::SetProperty(void const* value, char const* name, TypeId typeId) {
    if (!value || !name || !instance_ || !typeDesc_) {
        return false;
    }
    
    PropertyDescriptor const* prop = FindProperty(name);
    if (!prop || prop->valueTypeId != typeId) {
        return false;
    }
    
    void* dst = static_cast<char*>(instance_) + prop->offset;
    std::memcpy(dst, value, prop->size);
    return true;
}

PropertyDescriptor const* PropertyBag::FindProperty(char const* name) const {
    if (!name || !typeDesc_ || !typeDesc_->properties) {
        return nullptr;
    }
    
    for (std::size_t i = 0; i < typeDesc_->propertyCount; ++i) {
        if (typeDesc_->properties[i].name && std::strcmp(typeDesc_->properties[i].name, name) == 0) {
            return &typeDesc_->properties[i];
        }
    }
    
    return nullptr;
}

std::size_t PropertyBag::GetPropertyCount() const {
    return typeDesc_ ? typeDesc_->propertyCount : 0;
}

PropertyDescriptor const* PropertyBag::GetProperty(std::size_t index) const {
    if (!typeDesc_ || !typeDesc_->properties || index >= typeDesc_->propertyCount) {
        return nullptr;
    }
    
    return &typeDesc_->properties[index];
}

} // namespace object
} // namespace te
