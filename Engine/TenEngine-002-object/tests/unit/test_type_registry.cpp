/**
 * @file test_type_registry.cpp
 * @brief Unit tests for TypeRegistry.
 */

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
    return te::core::Alloc(sizeof(TestStruct), alignof(TestStruct));
}

} // namespace

int main() {
    using namespace te::object;
    
    // Test type registration
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
    assert(!TypeRegistry::RegisterType(desc));  // Should fail (duplicate)
    
    // Test type query
    TypeDescriptor const* found = TypeRegistry::GetTypeByName("TestStruct");
    assert(found != nullptr);
    assert(found->id == 1);
    assert(std::strcmp(found->name, "TestStruct") == 0);
    
    found = TypeRegistry::GetTypeById(1);
    assert(found != nullptr);
    assert(std::strcmp(found->name, "TestStruct") == 0);
    
    // Test type check
    assert(TypeRegistry::IsTypeRegistered(1));
    assert(TypeRegistry::IsTypeRegistered("TestStruct"));
    assert(!TypeRegistry::IsTypeRegistered(999));
    assert(!TypeRegistry::IsTypeRegistered("NonExistent"));
    
    // Test instance creation
    void* instance = TypeRegistry::CreateInstance(1);
    assert(instance != nullptr);
    te::core::Free(instance);
    
    instance = TypeRegistry::CreateInstance("TestStruct");
    assert(instance != nullptr);
    te::core::Free(instance);
    
    return 0;
}
