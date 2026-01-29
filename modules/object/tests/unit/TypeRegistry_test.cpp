/** Unit test: register type and query by name/ID (contract: 002-object-public-api.md US1) */

#include "te/object/TypeDescriptor.hpp"
#include "te/object/TypeRegistry.hpp"
#include "te/object/TypeId.hpp"
#include <cassert>
#include <cstring>
#include <iostream>

int main() {
    using namespace te::object;

    TypeDescriptor desc{};
    desc.id = 1u;
    desc.name = "TestType";
    desc.size = 8u;
    desc.properties = nullptr;
    desc.propertyCount = 0;
    desc.methods = nullptr;
    desc.methodCount = 0;
    desc.baseTypeId = kInvalidTypeId;

    assert(TypeRegistry::RegisterType(desc));
    assert(!TypeRegistry::RegisterType(desc));  // duplicate TypeId rejected

    TypeDescriptor const* byName = TypeRegistry::GetTypeByName("TestType");
    assert(byName && byName->id == 1u && std::strcmp(byName->name, "TestType") == 0);

    TypeDescriptor const* byId = TypeRegistry::GetTypeById(1u);
    assert(byId && byId->id == 1u && byId->size == 8u);

    assert(!TypeRegistry::GetTypeByName("NoSuch"));
    assert(!TypeRegistry::GetTypeById(999u));

    std::cout << "TypeRegistry_test passed.\n";
    return 0;
}
