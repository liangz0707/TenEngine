/**
 * Unit test: Serialize then Deserialize roundtrip (minimal ISerializer impl).
 */

#include <te/object/TypeRegistry.hpp>
#include <te/object/TypeDescriptor.hpp>
#include <te/object/TypeId.hpp>
#include <te/object/Serializer.hpp>
#include <te/object/SerializedBuffer.hpp>
#include <te/core/engine.h>
#include <cassert>
#include <cstring>
#include <vector>

namespace {

struct SimplePayload {
  int a = 0;
  int b = 0;
};

class TrivialSerializer : public te::object::ISerializer {
 public:
  bool Serialize(te::object::SerializedBuffer& out, void const* obj,
                 te::object::TypeId typeId) override {
    te::object::TypeDescriptor const* d = te::object::TypeRegistry::GetTypeById(typeId);
    if (!d || !out.data || out.capacity < d->size || !obj) return false;
    std::memcpy(out.data, obj, d->size);
    out.size = d->size;
    return true;
  }
  bool Deserialize(te::object::SerializedBuffer const& buf, void* obj,
                   te::object::TypeId typeId) override {
    te::object::TypeDescriptor const* d = te::object::TypeRegistry::GetTypeById(typeId);
    if (!d || !buf.data || buf.size < d->size || !obj) return false;
    std::memcpy(obj, buf.data, d->size);
    return true;
  }
  uint32_t GetCurrentVersion() const override { return 1u; }
  void SetVersionMigration(te::object::IVersionMigration*) override {}
};

}  // namespace

int main() {
  if (!te::core::Init(nullptr)) return 1;

  te::object::TypeDescriptor desc{};
  desc.id = 300u;
  desc.name = "SimplePayload";
  desc.size = sizeof(SimplePayload);
  desc.properties = nullptr;
  desc.propertyCount = 0;
  desc.methods = nullptr;
  desc.methodCount = 0;
  desc.baseTypeId = te::object::kInvalidTypeId;
  assert(te::object::TypeRegistry::RegisterType(desc));

  SimplePayload src{42, 99};
  std::vector<char> buffer(256);
  te::object::SerializedBuffer out{};
  out.data = buffer.data();
  out.capacity = buffer.size();

  TrivialSerializer ser;
  bool ok = ser.Serialize(out, &src, 300u);
  assert(ok && out.size == sizeof(SimplePayload));

  void* obj = te::object::TypeRegistry::CreateInstance(300u);
  assert(obj);
  te::object::SerializedBuffer in{};
  in.data = buffer.data();
  in.size = out.size;
  in.capacity = buffer.size();
  ok = ser.Deserialize(in, obj, 300u);
  assert(ok);
  SimplePayload* dst = static_cast<SimplePayload*>(obj);
  assert(dst->a == 42 && dst->b == 99);

  te::core::Shutdown();
  return 0;
}
