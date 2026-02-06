/**
 * @file Guid.h
 * @brief GUID (Globally Unique Identifier) system (contract: specs/_contracts/002-object-public-api.md).
 * Supports GUID generation, parsing, and conversion.
 */
#ifndef TE_OBJECT_GUID_H
#define TE_OBJECT_GUID_H

#include <cstdint>
#include <string>

namespace te {
namespace object {

/**
 * GUID: 16 bytes (128 bits) globally unique identifier.
 */
struct GUID {
    std::uint8_t data[16];
    
    /**
     * Generate a new GUID.
     */
    static GUID Generate();
    
    /**
     * Parse GUID from string (supports standard UUID format).
     * Returns null GUID on failure.
     */
    static GUID FromString(char const* str);
    
    /**
     * Convert GUID to string (standard UUID format).
     */
    std::string ToString() const;
    
    /**
     * Equality comparison.
     */
    bool operator==(GUID const& other) const;
    
    /**
     * Inequality comparison.
     */
    bool operator!=(GUID const& other) const;
    
    /**
     * Less-than comparison (for ordering).
     */
    bool operator<(GUID const& other) const;
    
    /**
     * Check if GUID is null (all zeros).
     */
    bool IsNull() const;
};

/**
 * Object reference: used for cross-resource references.
 */
struct ObjectRef {
    GUID guid;
    
    ObjectRef() : guid{} {}
    explicit ObjectRef(GUID const& g) : guid(g) {}
    
    /**
     * Check if reference is null.
     */
    bool IsNull() const { return guid.IsNull(); }
};

} // namespace object
} // namespace te

#endif // TE_OBJECT_GUID_H
