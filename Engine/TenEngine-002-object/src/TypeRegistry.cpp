/**
 * TypeRegistry: RegisterType, GetTypeByName, GetTypeById, CreateInstance.
 * ABI: specs/_contracts/002-object-ABI.md. Uses 001-Core Alloc/Free only.
 */

#include <te/object/TypeRegistry.hpp>
#include <te/object/TypeDescriptor.hpp>
#include <te/object/detail/CoreMemory.hpp>
#include <te/core/containers.h>
#include <cstring>
#include <mutex>

namespace te {
namespace object {

namespace {

struct RegistryState {
  core::Map<TypeId, TypeDescriptor const*> by_id;
  core::Map<core::String, TypeDescriptor const*> by_name;
  core::Array<TypeDescriptor const*> owned;  // descriptors we allocated
  std::mutex mtx;
};
RegistryState& State() {
  static RegistryState s;
  return s;
}

}  // namespace

bool TypeRegistry::RegisterType(TypeDescriptor const& desc) {
  if (desc.id == kInvalidTypeId || !desc.name) return false;
  auto& s = State();
  std::lock_guard<std::mutex> lock(s.mtx);
  if (s.by_id.count(desc.id)) return false;
  TypeDescriptor* copy = static_cast<TypeDescriptor*>(
      detail::Alloc(sizeof(TypeDescriptor), alignof(TypeDescriptor)));
  if (!copy) return false;
  *copy = desc;
  s.owned.push_back(copy);
  s.by_id[copy->id] = copy;
  s.by_name[copy->name] = copy;
  return true;
}

TypeDescriptor const* TypeRegistry::GetTypeByName(char const* name) {
  if (!name) return nullptr;
  auto& s = State();
  std::lock_guard<std::mutex> lock(s.mtx);
  auto it = s.by_name.find(name);
  return it != s.by_name.end() ? it->second : nullptr;
}

TypeDescriptor const* TypeRegistry::GetTypeById(TypeId id) {
  auto& s = State();
  std::lock_guard<std::mutex> lock(s.mtx);
  auto it = s.by_id.find(id);
  return it != s.by_id.end() ? it->second : nullptr;
}

void* TypeRegistry::CreateInstance(TypeId id) {
  TypeDescriptor const* desc = GetTypeById(id);
  if (!desc || desc->size == 0) return nullptr;
  return detail::Alloc(desc->size, alignof(std::max_align_t));
}

}  // namespace object
}  // namespace te
