/**
 * Unit test: TypeRegistry RegisterType, GetTypeByName, GetTypeById.
 * Covers upstream: Core Init so Alloc is available for descriptor storage.
 */

#include <te/object/TypeRegistry.hpp>
#include <te/object/TypeDescriptor.hpp>
#include <te/object/TypeId.hpp>
#include <te/core/engine.h>
#include <cassert>
#include <cstring>

int main() {
  if (!te::core::Init(nullptr)) return 1;
  te::core::Init(nullptr);

  te::object::TypeDescriptor desc{};
  desc.id = 100u;
  desc.name = "MyAssetDesc";
  desc.size = 64u;
  desc.properties = nullptr;
  desc.propertyCount = 0;
  desc.methods = nullptr;
  desc.methodCount = 0;
  desc.baseTypeId = te::object::kInvalidTypeId;

  bool ok = te::object::TypeRegistry::RegisterType(desc);
  assert(ok && "first RegisterType must succeed");

  bool dup = te::object::TypeRegistry::RegisterType(desc);
  assert(!dup && "duplicate TypeId must be rejected");

  te::object::TypeDescriptor const* by_name = te::object::TypeRegistry::GetTypeByName("MyAssetDesc");
  assert(by_name && by_name->id == 100u && by_name->size == 64u);

  te::object::TypeDescriptor const* by_id = te::object::TypeRegistry::GetTypeById(100u);
  assert(by_id && by_id->name && strcmp(by_id->name, "MyAssetDesc") == 0);

  assert(te::object::TypeRegistry::GetTypeByName("NoSuch") == nullptr);
  assert(te::object::TypeRegistry::GetTypeById(999u) == nullptr);

  te::core::Shutdown();
  return 0;
}
