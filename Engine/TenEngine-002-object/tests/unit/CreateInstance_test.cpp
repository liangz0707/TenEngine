/**
 * Unit test: TypeRegistry::CreateInstance (Core Alloc path).
 */

#include <te/object/TypeRegistry.hpp>
#include <te/object/TypeDescriptor.hpp>
#include <te/object/TypeId.hpp>
#include <te/core/engine.h>
#include <cassert>
#include <cstring>

int main() {
  if (!te::core::Init(nullptr)) return 1;

  te::object::TypeDescriptor desc{};
  desc.id = 200u;
  desc.name = "TestType";
  desc.size = 32u;
  desc.properties = nullptr;
  desc.propertyCount = 0;
  desc.methods = nullptr;
  desc.methodCount = 0;
  desc.baseTypeId = te::object::kInvalidTypeId;

  bool ok = te::object::TypeRegistry::RegisterType(desc);
  assert(ok);

  void* obj = te::object::TypeRegistry::CreateInstance(200u);
  assert(obj != nullptr && "CreateInstance must return non-null for registered type");
  std::memset(obj, 0, 32u);

  void* bad = te::object::TypeRegistry::CreateInstance(999u);
  assert(bad == nullptr && "CreateInstance must return nullptr for unregistered id");

  te::core::Shutdown();
  return 0;
}
