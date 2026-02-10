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
#include <cctype>

namespace te {
namespace object {

namespace {
char const* FindLastDot(char const* path) {
    char const* last = nullptr;
    for (; *path; ++path) {
        if (*path == '.') last = path;
    }
    return last;
}
bool EndsWithIgnoreCase(char const* str, char const* suffix) {
    size_t n = std::strlen(str);
    size_t m = std::strlen(suffix);
    if (m > n) return false;
    for (size_t i = 0; i < m; ++i) {
        if (std::tolower(static_cast<unsigned char>(str[n - m + i])) != std::tolower(static_cast<unsigned char>(suffix[i])))
            return false;
    }
    return true;
}
}  // namespace

SerializationFormat GetFormatFromPath(char const* path) {
    if (!path || !*path) return SerializationFormat::Binary;
    char const* lastDot = FindLastDot(path);
    if (!lastDot || !*(lastDot + 1)) return SerializationFormat::Binary;
    if (EndsWithIgnoreCase(path, ".json")) return SerializationFormat::JSON;
    if (EndsWithIgnoreCase(path, ".xml")) return SerializationFormat::XML;
    return SerializationFormat::Binary;
}

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

bool DeserializeFromFile(char const* path, void* obj, char const* typeName) {
    return DeserializeFromFile(path, obj, typeName, GetFormatFromPath(path));
}

} // namespace object
} // namespace te
