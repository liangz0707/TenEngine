/**
 * @file BinarySerializer.cpp
 * @brief Implementation of BinarySerializer (contract: specs/_contracts/002-object-public-api.md).
 * Compact binary serialization format with version support and GUID references.
 */

#include "te/object/Serializer.h"
#include "te/object/TypeRegistry.h"
#include "te/object/Guid.h"
#include "te/core/alloc.h"
#include <cstring>
#include <vector>
#include <cstdint>
#include <string>

namespace te {
namespace object {

using te::core::Alloc;
using te::core::Free;

namespace {

// Binary format header
struct BinaryHeader {
    std::uint32_t magic;        // Magic number: 0x54454F42 ("TEOB")
    std::uint32_t version;      // Format version
    TypeId typeId;              // Type ID
    std::uint32_t dataSize;     // Size of serialized data (after header)
};

constexpr std::uint32_t kBinaryMagic = 0x54454F42;  // "TEOB"
constexpr std::uint32_t kCurrentVersion = 1;

// Type markers for binary format
enum class TypeMarker : std::uint8_t {
    Primitive = 0x01,      // Basic types (int, float, bool, etc.)
    GUID = 0x02,           // GUID (16 bytes)
    ObjectRef = 0x03,       // ObjectRef (contains GUID)
    NestedObject = 0x04,    // Nested object (recursive)
    String = 0x05,          // String (length-prefixed)
    Array = 0x06            // Array (count-prefixed)
};

// Binary writer: stream-like buffer writer
class BinaryWriter {
public:
    BinaryWriter() : data_(nullptr), size_(0), capacity_(0) {}
    
    ~BinaryWriter() {
        if (data_) {
            te::core::Free(data_);
        }
    }
    
    bool Write(void const* src, std::size_t bytes) {
        if (!Reserve(size_ + bytes)) {
            return false;
        }
        std::memcpy(static_cast<char*>(data_) + size_, src, bytes);
        size_ += bytes;
        return true;
    }
    
    template<typename T>
    bool WriteValue(T const& value) {
        return Write(&value, sizeof(T));
    }
    
    bool WriteString(char const* str) {
        if (!str) {
            return WriteValue<std::uint32_t>(0);
        }
        std::size_t len = std::strlen(str);
        if (len > 0xFFFFFFFF) {
            return false;  // String too long
        }
        if (!WriteValue<std::uint32_t>(static_cast<std::uint32_t>(len))) {
            return false;
        }
        return Write(str, len);
    }
    
    void* GetData() const { return data_; }
    std::size_t GetSize() const { return size_; }
    
    void TransferTo(SerializedBuffer& out) {
        out.data = data_;
        out.size = size_;
        out.capacity = capacity_;
        data_ = nullptr;
        size_ = 0;
        capacity_ = 0;
    }
    
private:
    bool Reserve(std::size_t newSize) {
        if (newSize <= capacity_) {
            return true;
        }
        
        std::size_t newCapacity = capacity_ == 0 ? 256 : capacity_ * 2;
        while (newCapacity < newSize) {
            newCapacity *= 2;
        }
        
        void* newData = te::core::Alloc(newCapacity, alignof(std::max_align_t));
        if (!newData) {
            return false;
        }
        
        if (data_) {
            std::memcpy(newData, data_, size_);
            te::core::Free(data_);
        }
        
        data_ = newData;
        capacity_ = newCapacity;
        return true;
    }
    
    void* data_;
    std::size_t size_;
    std::size_t capacity_;
};

// Binary reader: stream-like buffer reader
class BinaryReader {
public:
    BinaryReader(void const* data, std::size_t size) 
        : data_(static_cast<char const*>(data)), size_(size), pos_(0) {}
    
    bool Read(void* dst, std::size_t bytes) {
        if (pos_ + bytes > size_) {
            return false;
        }
        std::memcpy(dst, data_ + pos_, bytes);
        pos_ += bytes;
        return true;
    }
    
    template<typename T>
    bool ReadValue(T& value) {
        return Read(&value, sizeof(T));
    }
    
    bool ReadString(std::string& str) {
        std::uint32_t len = 0;
        if (!ReadValue<std::uint32_t>(len)) {
            return false;
        }
        if (len == 0) {
            str.clear();
            return true;
        }
        if (pos_ + len > size_) {
            return false;
        }
        str.assign(data_ + pos_, data_ + pos_ + len);
        pos_ += len;
        return true;
    }
    
    std::size_t GetPosition() const { return pos_; }
    std::size_t GetRemaining() const { return size_ - pos_; }
    bool IsAtEnd() const { return pos_ >= size_; }
    
private:
    char const* data_;
    std::size_t size_;
    std::size_t pos_;
};

// Check if a type is GUID
bool IsGUIDType(TypeId typeId) {
    // GUID is a struct with 16 bytes
    TypeDescriptor const* desc = TypeRegistry::GetTypeById(typeId);
    if (!desc) {
        return false;
    }
    // Check by size (GUID is 16 bytes)
    return desc->size == sizeof(GUID) && desc->propertyCount == 0;
}

// Check if a type is ObjectRef
bool IsObjectRefType(TypeId typeId) {
    TypeDescriptor const* desc = TypeRegistry::GetTypeById(typeId);
    if (!desc) {
        return false;
    }
    // ObjectRef contains a GUID field
    return desc->size == sizeof(ObjectRef) && desc->propertyCount == 1 &&
           desc->properties[0].name && std::strcmp(desc->properties[0].name, "guid") == 0;
}

// Forward declarations
bool SerializeObject(BinaryWriter& writer, void const* obj, TypeDescriptor const* desc);
bool DeserializeObject(BinaryReader& reader, void* obj, TypeDescriptor const* desc);

// Serialize a property value recursively
bool SerializeProperty(BinaryWriter& writer, void const* obj, PropertyDescriptor const* prop) {
    void const* propPtr = static_cast<char const*>(obj) + prop->offset;
    
    // Check for GUID
    if (prop->size == sizeof(GUID)) {
        GUID const* guid = static_cast<GUID const*>(propPtr);
        if (!writer.WriteValue<TypeMarker>(TypeMarker::GUID)) {
            return false;
        }
        return writer.Write(guid->data, sizeof(GUID));
    }
    
    // Check for ObjectRef
    if (prop->size == sizeof(ObjectRef)) {
        ObjectRef const* ref = static_cast<ObjectRef const*>(propPtr);
        if (!writer.WriteValue<TypeMarker>(TypeMarker::ObjectRef)) {
            return false;
        }
        return writer.Write(ref->guid.data, sizeof(GUID));
    }
    
    // Check for nested object
    if (prop->valueTypeId != kInvalidTypeId) {
        TypeDescriptor const* nestedDesc = TypeRegistry::GetTypeById(prop->valueTypeId);
        if (nestedDesc && nestedDesc->propertyCount > 0) {
            // Nested object: serialize recursively
            if (!writer.WriteValue<TypeMarker>(TypeMarker::NestedObject)) {
                return false;
            }
            if (!writer.WriteValue<TypeId>(prop->valueTypeId)) {
                return false;
            }
            return SerializeObject(writer, propPtr, nestedDesc);
        }
    }
    
    // Primitive type: write marker and data
    if (!writer.WriteValue<TypeMarker>(TypeMarker::Primitive)) {
        return false;
    }
    return writer.Write(propPtr, prop->size);
}

// Serialize an object recursively
bool SerializeObject(BinaryWriter& writer, void const* obj, TypeDescriptor const* desc) {
    // Write property count
    if (!writer.WriteValue<std::uint32_t>(static_cast<std::uint32_t>(desc->propertyCount))) {
        return false;
    }
    
    // Serialize each property
    for (std::size_t i = 0; i < desc->propertyCount; ++i) {
        PropertyDescriptor const* prop = &desc->properties[i];
        if (!prop->name) {
            continue;
        }
        
        // Write property name
        if (!writer.WriteString(prop->name)) {
            return false;
        }
        
        // Write property value
        if (!SerializeProperty(writer, obj, prop)) {
            return false;
        }
    }
    
    return true;
}

// Deserialize a property value recursively
bool DeserializeProperty(BinaryReader& reader, void* obj, PropertyDescriptor const* prop) {
    void* propPtr = static_cast<char*>(obj) + prop->offset;
    
    TypeMarker marker;
    if (!reader.ReadValue<TypeMarker>(marker)) {
        return false;
    }
    
    switch (marker) {
        case TypeMarker::GUID: {
            if (prop->size == sizeof(GUID)) {
                return reader.Read(propPtr, sizeof(GUID));
            }
            return false;
        }
        
        case TypeMarker::ObjectRef: {
            if (prop->size == sizeof(ObjectRef)) {
                ObjectRef* ref = static_cast<ObjectRef*>(propPtr);
                return reader.Read(ref->guid.data, sizeof(GUID));
            }
            return false;
        }
        
        case TypeMarker::NestedObject: {
            TypeId nestedTypeId = kInvalidTypeId;
            if (!reader.ReadValue<TypeId>(nestedTypeId)) {
                return false;
            }
            TypeDescriptor const* nestedDesc = TypeRegistry::GetTypeById(nestedTypeId);
            if (!nestedDesc) {
                return false;
            }
            return DeserializeObject(reader, propPtr, nestedDesc);
        }
        
        case TypeMarker::Primitive: {
            return reader.Read(propPtr, prop->size);
        }
        
        case TypeMarker::String: {
            std::string str;
            if (!reader.ReadString(str)) {
                return false;
            }
            // For simplicity, assume string is stored as pointer or inline
            // In full implementation, would need proper string handling
            if (prop->size >= sizeof(char*)) {
                // Store as pointer (simplified)
                char** strPtr = static_cast<char**>(propPtr);
                *strPtr = nullptr;  // Would need to allocate and copy in full implementation
            }
            return true;
        }
        
        case TypeMarker::Array: {
            // Array handling would go here
            // For now, skip arrays
            return false;
        }
        
        default:
            return false;
    }
}

// Deserialize an object recursively
bool DeserializeObject(BinaryReader& reader, void* obj, TypeDescriptor const* desc) {
    // Read property count
    std::uint32_t propertyCount = 0;
    if (!reader.ReadValue<std::uint32_t>(propertyCount)) {
        return false;
    }
    
    // Create a map of property names to descriptors for efficient lookup
    // For simplicity, we'll iterate through properties
    std::size_t propertiesRead = 0;
    
    for (std::uint32_t i = 0; i < propertyCount; ++i) {
        // Read property name
        std::string propName;
        if (!reader.ReadString(propName)) {
            return false;
        }
        
        // Find property descriptor
        PropertyDescriptor const* prop = nullptr;
        for (std::size_t j = 0; j < desc->propertyCount; ++j) {
            if (desc->properties[j].name && propName == desc->properties[j].name) {
                prop = &desc->properties[j];
                break;
            }
        }
        
        if (prop) {
            // Deserialize property value
            if (!DeserializeProperty(reader, obj, prop)) {
                return false;
            }
            ++propertiesRead;
        } else {
            // Unknown property: skip it by reading the marker and skipping data
            TypeMarker marker;
            if (!reader.ReadValue<TypeMarker>(marker)) {
                return false;
            }
            // Skip based on marker type (simplified - would need full skip logic)
            // For now, fail on unknown properties
            return false;
        }
    }
    
    return true;
}

} // namespace

class BinarySerializerImpl : public ISerializer {
public:
    BinarySerializerImpl() : version_(kCurrentVersion), migration_(nullptr) {}
    
    bool Serialize(SerializedBuffer& out, void const* obj, TypeId typeId) override {
        if (!obj || typeId == kInvalidTypeId) {
            return false;
        }
        
        TypeDescriptor const* desc = TypeRegistry::GetTypeById(typeId);
        if (!desc) {
            return false;
        }
        
        return SerializeInternal(out, obj, desc);
    }
    
    bool Serialize(SerializedBuffer& out, void const* obj, char const* typeName) override {
        if (!obj || !typeName) {
            return false;
        }
        
        TypeDescriptor const* desc = TypeRegistry::GetTypeByName(typeName);
        if (!desc) {
            return false;
        }
        
        return SerializeInternal(out, obj, desc);
    }
    
    bool Deserialize(SerializedBuffer const& buf, void* obj, TypeId typeId) override {
        if (!buf.IsValid() || !obj || typeId == kInvalidTypeId) {
            return false;
        }
        
        TypeDescriptor const* desc = TypeRegistry::GetTypeById(typeId);
        if (!desc) {
            return false;
        }
        
        return DeserializeInternal(buf, obj, desc);
    }
    
    bool Deserialize(SerializedBuffer const& buf, void* obj, char const* typeName) override {
        if (!buf.IsValid() || !obj || !typeName) {
            return false;
        }
        
        TypeDescriptor const* desc = TypeRegistry::GetTypeByName(typeName);
        if (!desc) {
            return false;
        }
        
        return DeserializeInternal(buf, obj, desc);
    }
    
    std::uint32_t GetCurrentVersion() const override {
        return version_;
    }
    
    void SetVersionMigration(IVersionMigration* migration) override {
        migration_ = migration;
    }
    
    SerializationFormat GetFormat() const override {
        return SerializationFormat::Binary;
    }
    
private:
    bool SerializeInternal(SerializedBuffer& out, void const* obj, TypeDescriptor const* desc) {
        BinaryWriter writer;
        
        // Write object data
        if (!SerializeObject(writer, obj, desc)) {
            return false;
        }
        
        // Calculate total size
        std::size_t headerSize = sizeof(BinaryHeader);
        std::size_t dataSize = writer.GetSize();
        std::size_t totalSize = headerSize + dataSize;
        
        // Allocate final buffer
        void* finalData = te::core::Alloc(totalSize, alignof(std::max_align_t));
        if (!finalData) {
            return false;
        }
        
        // Write header
        BinaryHeader* header = static_cast<BinaryHeader*>(finalData);
        header->magic = kBinaryMagic;
        header->version = version_;
        header->typeId = desc->id;
        header->dataSize = static_cast<std::uint32_t>(dataSize);
        
        // Copy serialized data
        std::memcpy(static_cast<char*>(finalData) + headerSize, writer.GetData(), dataSize);
        
        // Clean up writer data
        if (writer.GetData()) {
            te::core::Free(writer.GetData());
        }
        
        // Update output buffer
        if (out.data) {
            te::core::Free(out.data);
        }
        out.data = finalData;
        out.size = totalSize;
        out.capacity = totalSize;
        
        return true;
    }
    
    bool DeserializeInternal(SerializedBuffer const& buf, void* obj, TypeDescriptor const* desc) {
        if (buf.size < sizeof(BinaryHeader)) {
            return false;
        }
        
        BinaryHeader const* header = static_cast<BinaryHeader const*>(buf.data);
        
        // Check magic
        if (header->magic != kBinaryMagic) {
            return false;
        }
        
        // Check type
        if (header->typeId != desc->id) {
            return false;
        }
        
        // Version migration if needed
        SerializedBuffer mutableBuf = buf;
        if (header->version < version_ && migration_) {
            if (!migration_->Migrate(mutableBuf, header->version, version_)) {
                return false;
            }
            // Re-read header after migration
            header = static_cast<BinaryHeader const*>(mutableBuf.data);
        }
        
        // Check data size
        if (mutableBuf.size < sizeof(BinaryHeader) + header->dataSize) {
            return false;
        }
        
        // Create reader for data section
        void const* dataPtr = static_cast<char const*>(mutableBuf.data) + sizeof(BinaryHeader);
        BinaryReader reader(dataPtr, header->dataSize);
        
        // Deserialize object
        return DeserializeObject(reader, obj, desc);
    }
    
    std::uint32_t version_;
    IVersionMigration* migration_;
};

ISerializer* CreateBinarySerializer() {
    return new BinarySerializerImpl();
}

} // namespace object
} // namespace te
