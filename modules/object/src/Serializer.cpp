/** Default binary serializer (contract: 002-object-public-api.md fullversion-001) */

#include "te/object/Serializer.hpp"
#include "te/object/TypeRegistry.hpp"
#include <cstring>
#include <cstdlib>
#include <vector>

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
        size_t need = desc->size;
        if (out.capacity < need) {
            void* newData = std::realloc(out.data, need);
            if (!newData) return false;
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
