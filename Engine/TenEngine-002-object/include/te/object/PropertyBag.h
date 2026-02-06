/**
 * @file PropertyBag.h
 * @brief Property bag system (contract: specs/_contracts/002-object-public-api.md).
 * Provides property access and manipulation based on type descriptors.
 */
#ifndef TE_OBJECT_PROPERTY_BAG_H
#define TE_OBJECT_PROPERTY_BAG_H

#include "te/object/TypeId.h"
#include <cstddef>

namespace te {
namespace object {

// Forward declaration
struct PropertyDescriptor;

/**
 * Property bag interface.
 */
class IPropertyBag {
public:
    virtual ~IPropertyBag() = default;
    
    /**
     * Get property value by name.
     */
    virtual bool GetProperty(void* outValue, char const* name) const = 0;
    
    /**
     * Get property value by name with type check.
     */
    virtual bool GetProperty(void* outValue, char const* name, TypeId typeId) const = 0;
    
    /**
     * Set property value by name.
     */
    virtual bool SetProperty(void const* value, char const* name) = 0;
    
    /**
     * Set property value by name with type check.
     */
    virtual bool SetProperty(void const* value, char const* name, TypeId typeId) = 0;
    
    /**
     * Find property descriptor by name.
     */
    virtual PropertyDescriptor const* FindProperty(char const* name) const = 0;
    
    /**
     * Get number of properties.
     */
    virtual std::size_t GetPropertyCount() const = 0;
    
    /**
     * Get property descriptor by index.
     */
    virtual PropertyDescriptor const* GetProperty(std::size_t index) const = 0;
};

// Forward declaration
struct TypeDescriptor;

/**
 * Property bag implementation based on TypeDescriptor.
 */
class PropertyBag : public IPropertyBag {
public:
    /**
     * Construct property bag for an instance with type descriptor.
     */
    PropertyBag(void* instance, TypeDescriptor const* typeDesc);
    
    // IPropertyBag implementation
    bool GetProperty(void* outValue, char const* name) const override;
    bool GetProperty(void* outValue, char const* name, TypeId typeId) const override;
    bool SetProperty(void const* value, char const* name) override;
    bool SetProperty(void const* value, char const* name, TypeId typeId) override;
    PropertyDescriptor const* FindProperty(char const* name) const override;
    std::size_t GetPropertyCount() const override;
    PropertyDescriptor const* GetProperty(std::size_t index) const override;
    
private:
    void* instance_;
    TypeDescriptor const* typeDesc_;
};

} // namespace object
} // namespace te

#endif // TE_OBJECT_PROPERTY_BAG_H
