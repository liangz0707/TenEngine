/**
 * Default binary serializer and version-migration hook.
 * ABI: specs/_contracts/002-object-ABI.md
 */

#include <te/object/Serializer.hpp>
#include <te/object/TypeRegistry.hpp>
#include <te/object/VersionMigration.hpp>
#include <cstring>
#include <cstdint>

namespace te {
namespace object {

namespace {

constexpr uint32_t kCurrentVersion = 1u;

class DefaultSerializer : public ISerializer {
 public:
  DefaultSerializer() : migration_(nullptr) {}
  bool Serialize(SerializedBuffer& out, void const* obj, TypeId typeId) override {
    if (!out.data || !obj) return false;
    TypeDescriptor const* desc = TypeRegistry::GetTypeById(typeId);
    if (!desc || desc->size == 0) return false;
    size_t need = sizeof(uint32_t) + desc->size;  // version + payload
    if (out.capacity < need) return false;
    uint32_t* p = static_cast<uint32_t*>(out.data);
    *p = kCurrentVersion;
    std::memcpy(p + 1, obj, desc->size);
    out.size = need;
    return true;
  }
  bool Deserialize(SerializedBuffer const& buf, void* obj, TypeId typeId) override {
    if (!buf.data || buf.size < sizeof(uint32_t) || !obj) return false;
    TypeDescriptor const* desc = TypeRegistry::GetTypeById(typeId);
    if (!desc || desc->size == 0) return false;
    uint32_t const* p = static_cast<uint32_t const*>(buf.data);
    uint32_t version = *p;
    if (version < kCurrentVersion && migration_) {
      SerializedBuffer mutable_buf;
      mutable_buf.data = const_cast<void*>(buf.data);
      mutable_buf.size = buf.size;
      mutable_buf.capacity = buf.capacity;
      if (!migration_->Migrate(mutable_buf, version, kCurrentVersion)) return false;
      p = static_cast<uint32_t const*>(mutable_buf.data);
      version = *p;
    }
    if (version != kCurrentVersion) return false;
    size_t payload_size = buf.size - sizeof(uint32_t);
    if (payload_size < desc->size) return false;
    std::memcpy(obj, p + 1, desc->size);
    return true;
  }
  uint32_t GetCurrentVersion() const override { return kCurrentVersion; }
  void SetVersionMigration(IVersionMigration* migration) override { migration_ = migration; }

 private:
  IVersionMigration* migration_;
};

}  // namespace

}  // namespace object
}  // namespace te
