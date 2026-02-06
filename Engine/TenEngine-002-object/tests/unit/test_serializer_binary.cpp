/**
 * @file test_serializer_binary.cpp
 * @brief Unit tests for BinarySerializer.
 */

#include "te/object/Serializer.h"
#include "te/object/TypeRegistry.h"
#include "te/object/TypeId.h"
#include "te/core/alloc.h"
#include <cassert>
#include <cstring>

namespace {

struct TestStruct {
    int value;
    float fvalue;
};

void* CreateTestStruct() {
    void* ptr = te::core::Alloc(sizeof(TestStruct), alignof(TestStruct));
    TestStruct* s = static_cast<TestStruct*>(ptr);
    s->value = 0;
    s->fvalue = 0.0f;
    return ptr;
}

} // namespace

int main() {
    using namespace te::object;
    
    // Register type
    PropertyDescriptor props[] = {
        {"value", 0, offsetof(TestStruct, value), sizeof(int), nullptr},
        {"fvalue", 0, offsetof(TestStruct, fvalue), sizeof(float), nullptr}
    };
    
    TypeDescriptor desc;
    desc.id = 1;
    desc.name = "TestStruct";
    desc.size = sizeof(TestStruct);
    desc.properties = props;
    desc.propertyCount = 2;
    desc.baseTypeId = kInvalidTypeId;
    desc.createInstance = CreateTestStruct;
    
    assert(TypeRegistry::RegisterType(desc));
    
    // Create test data
    TestStruct original;
    original.value = 42;
    original.fvalue = 3.14f;
    
    // Serialize
    ISerializer* serializer = CreateBinarySerializer();
    assert(serializer != nullptr);
    assert(serializer->GetFormat() == SerializationFormat::Binary);
    
    SerializedBuffer buf{};
    assert(serializer->Serialize(buf, &original, 1));
    assert(buf.IsValid());
    
    // Deserialize
    TestStruct deserialized;
    assert(serializer->Deserialize(buf, &deserialized, 1));
    assert(deserialized.value == original.value);
    assert(deserialized.fvalue == original.fvalue);
    
    // Cleanup
    if (buf.data) {
        te::core::Free(buf.data);
    }
    delete serializer;
    
    return 0;
}
