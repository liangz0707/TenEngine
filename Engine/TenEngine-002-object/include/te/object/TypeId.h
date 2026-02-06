/**
 * @file TypeId.h
 * @brief Type identification and type descriptors (contract: specs/_contracts/002-object-public-api.md).
 * Only contract-declared types and API are exposed.
 */
#ifndef TE_OBJECT_TYPE_ID_H
#define TE_OBJECT_TYPE_ID_H

#include <cstddef>
#include <cstdint>

namespace te {
namespace object {

/**
 * Type identifier: uint32_t, 0 indicates invalid.
 */
using TypeId = std::uint32_t;

/**
 * Invalid type ID constant.
 */
constexpr TypeId kInvalidTypeId = 0;

/**
 * Property descriptor: describes a property of a type.
 */
struct PropertyDescriptor {
    char const* name = nullptr;              // Property name
    TypeId valueTypeId = kInvalidTypeId;    // Type ID of the property value
    std::size_t offset = 0;                 // Offset of property in object (bytes)
    std::size_t size = 0;                   // Size of property (bytes)
    void const* defaultValue = nullptr;      // Default value pointer (optional)
    // Metadata (range, enum, etc.) can be extended via PropertyBag
};

/**
 * Type descriptor: describes a registered type.
 */
struct TypeDescriptor {
    TypeId id = kInvalidTypeId;                    // Type ID
    char const* name = nullptr;                     // Type name
    std::size_t size = 0;                          // Size of type instance (bytes)
    PropertyDescriptor const* properties = nullptr; // Array of property descriptors
    std::size_t propertyCount = 0;                 // Number of properties
    TypeId baseTypeId = kInvalidTypeId;            // Base class type ID (for inheritance)
    void* (*createInstance)() = nullptr;            // Factory function to create instance
};

} // namespace object
} // namespace te

#endif // TE_OBJECT_TYPE_ID_H
