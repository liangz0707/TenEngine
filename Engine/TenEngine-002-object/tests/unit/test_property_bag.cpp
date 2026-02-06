/**
 * @file test_property_bag.cpp
 * @brief Unit tests for PropertyBag.
 */

#include "te/object/PropertyBag.h"
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
    
    // Create instance
    TestStruct* instance = static_cast<TestStruct*>(TypeRegistry::CreateInstance(1));
    assert(instance != nullptr);
    
    TypeDescriptor const* typeDesc = TypeRegistry::GetTypeById(1);
    assert(typeDesc != nullptr);
    
    // Test PropertyBag
    PropertyBag bag(instance, typeDesc);
    
    assert(bag.GetPropertyCount() == 2);
    
    // Test GetProperty
    int value = 0;
    assert(bag.GetProperty(&value, "value"));
    assert(value == 0);
    
    float fvalue = 0.0f;
    assert(bag.GetProperty(&fvalue, "fvalue"));
    assert(fvalue == 0.0f);
    
    // Test SetProperty
    value = 42;
    assert(bag.SetProperty(&value, "value"));
    assert(instance->value == 42);
    
    fvalue = 3.14f;
    assert(bag.SetProperty(&fvalue, "fvalue"));
    assert(instance->fvalue == 3.14f);
    
    // Test FindProperty
    PropertyDescriptor const* prop = bag.FindProperty("value");
    assert(prop != nullptr);
    assert(std::strcmp(prop->name, "value") == 0);
    
    // Test GetProperty by index
    prop = bag.GetProperty(0);
    assert(prop != nullptr);
    assert(std::strcmp(prop->name, "value") == 0 || std::strcmp(prop->name, "fvalue") == 0);
    
    te::core::Free(instance);
    
    return 0;
}
