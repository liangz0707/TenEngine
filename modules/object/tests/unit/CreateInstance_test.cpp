/** Unit test: CreateInstance and lifecycle (contract: 002-object-public-api.md US4). Uses Core Alloc/Free per contract. */

#include "te/object/TypeDescriptor.hpp"
#include "te/object/TypeRegistry.hpp"
#include "te/object/TypeId.hpp"
#include "te/object/detail/CoreMemory.hpp"
#include <cstring>
#include <iostream>

int main() {
    using namespace te::object;

    TypeDescriptor desc{};
    desc.id = 3u;
    desc.name = "InstanceType";
    desc.size = 16u;
    desc.properties = nullptr;
    desc.propertyCount = 0;
    desc.methods = nullptr;
    desc.methodCount = 0;
    desc.baseTypeId = kInvalidTypeId;

    if (!TypeRegistry::RegisterType(desc)) { std::cerr << "Register failed\n"; return 1; }

    void* p = TypeRegistry::CreateInstance(3u);
    if (!p) { std::cerr << "CreateInstance(3u) returned null\n"; return 1; }
    std::memset(p, 0, 16u);
    te::object::detail::Free(p);  // free with same allocator as CreateInstance (Core Alloc/Free per contract)

    if (TypeRegistry::CreateInstance(999u) != nullptr) { std::cerr << "CreateInstance(999u) should be null\n"; return 1; }

    std::cout << "CreateInstance_test passed.\n";
    return 0;
}
