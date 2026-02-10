/**
 * @file Serializer.h
 * @brief Serialization system interface (contract: specs/_contracts/002-object-public-api.md).
 * Supports binary, JSON, and XML serialization formats.
 */
#ifndef TE_OBJECT_SERIALIZER_H
#define TE_OBJECT_SERIALIZER_H

#include "te/object/TypeId.h"
#include <cstddef>
#include <cstdint>

namespace te {
namespace object {

/**
 * Serialized buffer: caller manages memory.
 */
struct SerializedBuffer {
    void* data = nullptr;
    std::size_t size = 0;
    std::size_t capacity = 0;
    
    /**
     * Check if buffer is valid.
     */
    bool IsValid() const { return data != nullptr && size > 0; }
    
    /**
     * Clear buffer size (does not free memory).
     */
    void Clear() { size = 0; }
};

/**
 * Serialization format enumeration.
 */
enum class SerializationFormat {
    Binary,  // Binary format (compact, fast)
    JSON,    // JSON format (readable, debuggable)
    XML      // XML format (structured, tool-compatible)
};

// Forward declaration
class IVersionMigration;

/**
 * Serializer interface.
 */
class ISerializer {
public:
    virtual ~ISerializer() = default;
    
    /**
     * Serialize object by TypeId.
     */
    virtual bool Serialize(SerializedBuffer& out, void const* obj, TypeId typeId) = 0;
    
    /**
     * Serialize object by type name.
     */
    virtual bool Serialize(SerializedBuffer& out, void const* obj, char const* typeName) = 0;
    
    /**
     * Deserialize object by TypeId.
     */
    virtual bool Deserialize(SerializedBuffer const& buf, void* obj, TypeId typeId) = 0;
    
    /**
     * Deserialize object by type name.
     */
    virtual bool Deserialize(SerializedBuffer const& buf, void* obj, char const* typeName) = 0;
    
    /**
     * Get current version.
     */
    virtual std::uint32_t GetCurrentVersion() const = 0;
    
    /**
     * Set version migration handler.
     */
    virtual void SetVersionMigration(IVersionMigration* migration) = 0;
    
    /**
     * Get serialization format.
     */
    virtual SerializationFormat GetFormat() const = 0;
};

/**
 * Version migration interface.
 */
class IVersionMigration {
public:
    virtual ~IVersionMigration() = default;
    
    /**
     * Migrate serialized buffer from one version to another.
     */
    virtual bool Migrate(SerializedBuffer& buf, std::uint32_t fromVersion, std::uint32_t toVersion) = 0;
};

/**
 * Create binary serializer.
 */
ISerializer* CreateBinarySerializer();

/**
 * Create JSON serializer.
 */
ISerializer* CreateJSONSerializer();

/**
 * Create XML serializer.
 */
ISerializer* CreateXMLSerializer();

/**
 * Infer serialization format from file path extension.
 * .json (case-insensitive) -> JSON, .xml -> XML, otherwise Binary.
 */
SerializationFormat GetFormatFromPath(char const* path);

/**
 * Serialize object to file (uses Core file I/O).
 */
bool SerializeToFile(char const* path, void const* obj, TypeId typeId, SerializationFormat format = SerializationFormat::Binary);

/**
 * Deserialize object from file by TypeId (uses Core file I/O).
 */
bool DeserializeFromFile(char const* path, void* obj, TypeId typeId, SerializationFormat format = SerializationFormat::Binary);

/**
 * Deserialize object from file by type name (uses Core file I/O).
 * For format inferred from path, use the 3-argument overload instead.
 */
bool DeserializeFromFile(char const* path, void* obj, char const* typeName, SerializationFormat format);

/**
 * Deserialize object from file by type name; format is inferred from path via GetFormatFromPath.
 */
bool DeserializeFromFile(char const* path, void* obj, char const* typeName);

} // namespace object
} // namespace te

#endif // TE_OBJECT_SERIALIZER_H
