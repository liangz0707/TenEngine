/** TypeRegistry implementation (contract: 002-object-public-api.md fullversion-001). Uses 001-Core Alloc/Free per contract. */

#include "te/object/TypeRegistry.hpp"
#include "te/object/detail/CoreMemory.hpp"
#include <cstddef>
#include <cstring>
#include <map>
#include <mutex>
#include <string>

namespace te::object {

namespace {

struct Registry {
    std::map<TypeId, TypeDescriptor> byId;
    std::map<std::string, TypeId> byName;
    std::mutex mtx;
};
Registry& getRegistry() {
    static Registry r;
    return r;
}

}  // namespace

bool TypeRegistry::RegisterType(TypeDescriptor const& desc) {
    auto& r = getRegistry();
    std::lock_guard<std::mutex> lock(r.mtx);
    if (desc.id == kInvalidTypeId) return false;
    if (r.byId.count(desc.id)) return false;  // duplicate TypeId rejected
    std::string name(desc.name ? desc.name : "");
    if (r.byName.count(name)) return false;   // duplicate name rejected (implementation choice)
    TypeDescriptor copy = desc;
    r.byId[copy.id] = copy;
    r.byName[name] = copy.id;
    return true;
}

TypeDescriptor const* TypeRegistry::GetTypeByName(char const* name) {
    auto& r = getRegistry();
    std::lock_guard<std::mutex> lock(r.mtx);
    if (!name) return nullptr;
    auto it = r.byName.find(name);
    if (it == r.byName.end()) return nullptr;
    auto it2 = r.byId.find(it->second);
    return it2 != r.byId.end() ? &it2->second : nullptr;
}

TypeDescriptor const* TypeRegistry::GetTypeById(TypeId id) {
    auto& r = getRegistry();
    std::lock_guard<std::mutex> lock(r.mtx);
    auto it = r.byId.find(id);
    return it != r.byId.end() ? &it->second : nullptr;
}

void* TypeRegistry::CreateInstance(TypeId id) {
    TypeDescriptor const* desc = GetTypeById(id);
    if (!desc || desc->size == 0) return nullptr;
    std::size_t align = alignof(std::max_align_t);
    void* p = detail::Alloc(desc->size, align);
    if (!p) return nullptr;
    std::memset(p, 0, desc->size);
    return p;
}

}  // namespace te::object
