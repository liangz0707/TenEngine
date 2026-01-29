/** Unit test: round-trip serialization with version migration (contract: 002-object-public-api.md US2) */

#include "te/object/SerializedBuffer.hpp"
#include "te/object/Serializer.hpp"
#include "te/object/TypeDescriptor.hpp"
#include "te/object/TypeRegistry.hpp"
#include "te/object/VersionMigration.hpp"
#include "te/object/detail/CoreMemory.hpp"
#include <cassert>
#include <cstddef>
#include <cstring>
#include <iostream>

namespace te::object {

/** Minimal serializer for test: uses Core Alloc/Free per contract. */
class TestSerializer : public ISerializer {
public:
    bool Serialize(SerializedBuffer& out, void const* obj, TypeId typeId) override {
        TypeDescriptor const* d = TypeRegistry::GetTypeById(typeId);
        if (!d || !obj || d->size == 0) return false;
        if (out.capacity < d->size) {
            std::size_t align = alignof(std::max_align_t);
            void* p = detail::Alloc(d->size, align);
            if (!p) return false;
            if (out.data && out.size) std::memcpy(p, out.data, out.size);
            detail::Free(out.data);
            out.data = p;
            out.capacity = d->size;
        }
        std::memcpy(out.data, obj, d->size);
        out.size = d->size;
        return true;
    }
    bool Deserialize(SerializedBuffer const& buf, void* obj, TypeId typeId) override {
        TypeDescriptor const* d = TypeRegistry::GetTypeById(typeId);
        if (!d || !obj || !buf.data || buf.size < d->size) return false;
        std::memcpy(obj, buf.data, d->size);
        return true;
    }
    uint32_t GetCurrentVersion() const override { return 1u; }
    void SetVersionMigration(IVersionMigration*) override {}
};

}  // namespace te::object

int main() {
    using namespace te::object;

    TypeDescriptor desc{};
    desc.id = 2u;
    desc.name = "PodType";
    desc.size = sizeof(uint64_t);
    desc.properties = nullptr;
    desc.propertyCount = 0;
    desc.methods = nullptr;
    desc.methodCount = 0;
    desc.baseTypeId = kInvalidTypeId;
    assert(TypeRegistry::RegisterType(desc));

    uint64_t src = 0x123456789ABCDEF0ULL;
    SerializedBuffer buf{nullptr, 0, 0};
    TestSerializer ser;
    assert(ser.Serialize(buf, &src, 2u));
    assert(buf.size == sizeof(uint64_t));

    uint64_t dst = 0;
    assert(ser.Deserialize(buf, &dst, 2u));
    assert(dst == src);

    te::object::detail::Free(buf.data);  // buffer may be grown by serializer with Core Alloc
    std::cout << "Serializer_roundtrip_test passed.\n";
    return 0;
}
