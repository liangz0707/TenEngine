/**
 * @file SerializerHelpers.cpp
 * @brief File serialization helper functions (contract: specs/_contracts/002-object-public-api.md).
 * Convenience functions for serializing to/from files using Core file I/O.
 */

#include "te/object/Serializer.h"
#include "te/core/platform.h"
#include "te/core/alloc.h"
#include <cstring>
#include <vector>

namespace te {
namespace object {

bool SerializeToFile(char const* path, void const* obj, TypeId typeId, SerializationFormat format) {
    if (!path || !obj || typeId == kInvalidTypeId) {
        return false;
    }
    
    // Create serializer
    ISerializer* serializer = nullptr;
    switch (format) {
        case SerializationFormat::Binary:
            serializer = CreateBinarySerializer();
            break;
        case SerializationFormat::JSON:
            serializer = CreateJSONSerializer();
            break;
        case SerializationFormat::XML:
            serializer = CreateXMLSerializer();
            break;
        default:
            return false;
    }
    
    if (!serializer) {
        return false;
    }
    
    // Serialize to buffer
    SerializedBuffer buf{};
    bool success = serializer->Serialize(buf, obj, typeId);
    
    if (success && buf.IsValid()) {
        // Write to file
        std::vector<std::uint8_t> data(static_cast<std::uint8_t const*>(buf.data), 
                                       static_cast<std::uint8_t const*>(buf.data) + buf.size);
        success = te::core::FileWrite(path, data);
    }
    
    // Cleanup
    if (buf.data) {
        te::core::Free(buf.data);
    }
    delete serializer;
    
    return success;
}

bool DeserializeFromFile(char const* path, void* obj, TypeId typeId, SerializationFormat format) {
    if (!path || !obj || typeId == kInvalidTypeId) {
        return false;
    }
    
    // Read file
    auto dataOpt = te::core::FileRead(path);
    if (!dataOpt.has_value()) {
        return false;
    }
    
    std::vector<std::uint8_t> const& data = dataOpt.value();
    if (data.empty()) {
        return false;
    }
    
    // Create buffer
    SerializedBuffer buf{};
    buf.size = data.size();
    buf.capacity = data.size();
    buf.data = te::core::Alloc(data.size(), alignof(std::max_align_t));
    if (!buf.data) {
        return false;
    }
    
    std::memcpy(buf.data, data.data(), data.size());
    
    // Create serializer
    ISerializer* serializer = nullptr;
    switch (format) {
        case SerializationFormat::Binary:
            serializer = CreateBinarySerializer();
            break;
        case SerializationFormat::JSON:
            serializer = CreateJSONSerializer();
            break;
        case SerializationFormat::XML:
            serializer = CreateXMLSerializer();
            break;
        default:
            te::core::Free(buf.data);
            return false;
    }
    
    if (!serializer) {
        te::core::Free(buf.data);
        return false;
    }
    
    // Deserialize
    bool success = serializer->Deserialize(buf, obj, typeId);
    
    // Cleanup
    te::core::Free(buf.data);
    delete serializer;
    
    return success;
}

bool DeserializeFromFile(char const* path, void* obj, char const* typeName, SerializationFormat format) {
    if (!path || !obj || !typeName) {
        return false;
    }
    
    // Read file
    auto dataOpt = te::core::FileRead(path);
    if (!dataOpt.has_value()) {
        return false;
    }
    
    std::vector<std::uint8_t> const& data = dataOpt.value();
    if (data.empty()) {
        return false;
    }
    
    // Create buffer
    SerializedBuffer buf{};
    buf.size = data.size();
    buf.capacity = data.size();
    buf.data = te::core::Alloc(data.size(), alignof(std::max_align_t));
    if (!buf.data) {
        return false;
    }
    
    std::memcpy(buf.data, data.data(), data.size());
    
    // Create serializer
    ISerializer* serializer = nullptr;
    switch (format) {
        case SerializationFormat::Binary:
            serializer = CreateBinarySerializer();
            break;
        case SerializationFormat::JSON:
            serializer = CreateJSONSerializer();
            break;
        case SerializationFormat::XML:
            serializer = CreateXMLSerializer();
            break;
        default:
            te::core::Free(buf.data);
            return false;
    }
    
    if (!serializer) {
        te::core::Free(buf.data);
        return false;
    }
    
    // Deserialize
    bool success = serializer->Deserialize(buf, obj, typeName);
    
    // Cleanup
    te::core::Free(buf.data);
    delete serializer;
    
    return success;
}

} // namespace object
} // namespace te
