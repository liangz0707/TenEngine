/** Default binary serializer (contract: 002-object-public-api.md fullversion-001). Uses 001-Core Alloc/Free per contract. */

#include "te/object/Serializer.hpp"
#include "te/object/TypeRegistry.hpp"
#include "te/object/detail/CoreMemory.hpp"
#include <cstddef>
#include <cstring>

namespace te::object {

namespace {

uint32_t const kCurrentVersion = 1u;

}

class DefaultSerializer : public ISerializer {
public:
    DefaultSerializer() : migration_(nullptr) {}

    bool Serialize(SerializedBuffer& out, void const* obj, TypeId typeId) override {
        TypeDescriptor const* desc = TypeRegistry::GetTypeById(typeId);
        if (!desc || !obj || desc->size == 0) return false;
        std::size_t need = desc->size;
        std::size_t align = alignof(std::max_align_t);
        if (out.capacity < need) {
            void* newData = detail::Alloc(need, align);
            if (!newData) return false;
            if (out.data && out.size) std::memcpy(newData, out.data, out.size);
            detail::Free(out.data);
            out.data = newData;
            out.capacity = need;
        }
        std::memcpy(out.data, obj, need);
        out.size = need;
        return true;
    }

    bool Deserialize(SerializedBuffer const& buf, void* obj, TypeId typeId) override {
        TypeDescriptor const* desc = TypeRegistry::GetTypeById(typeId);
        if (!desc || !obj || !buf.data || buf.size < desc->size) return false;
        std::memcpy(obj, buf.data, desc->size);
        return true;
    }

    uint32_t GetCurrentVersion() const override { return kCurrentVersion; }

    void SetVersionMigration(IVersionMigration* migration) override { migration_ = migration; }

private:
    IVersionMigration* migration_;
};

}  // namespace te::object
