/**
 * @file TypeRegistry.cpp
 * @brief Implementation of TypeRegistry (contract: specs/_contracts/002-object-public-api.md).
 * Thread-safe type registration and query system.
 */

#include "te/object/TypeRegistry.h"
#include "te/core/alloc.h"
#include <unordered_map>
#include <string>
#include <shared_mutex>
#include <vector>

namespace te {
namespace object {

namespace {

// Type registry storage
struct RegistryStorage {
    std::shared_mutex mutex;  // Read-write lock for thread safety
    std::unordered_map<TypeId, TypeDescriptor> typesById;
    std::unordered_map<std::string, TypeId> typesByName;
    std::vector<TypeDescriptor> descriptors;  // Owned storage for descriptors
};

RegistryStorage& GetRegistry() {
    static RegistryStorage s_registry;
    return s_registry;
}

} // namespace

bool TypeRegistry::RegisterType(TypeDescriptor const& desc) {
    if (desc.id == kInvalidTypeId || !desc.name) {
        return false;
    }
    
    auto& registry = GetRegistry();
    std::unique_lock<std::shared_mutex> lock(registry.mutex);
    
    // Check if already registered
    if (registry.typesById.find(desc.id) != registry.typesById.end()) {
        return false;  // Already registered
    }
    
    if (registry.typesByName.find(desc.name) != registry.typesByName.end()) {
        return false;  // Name already taken
    }
    
    // Allocate and copy descriptor (using Core Alloc)
    TypeDescriptor* descCopy = static_cast<TypeDescriptor*>(te::core::Alloc(sizeof(TypeDescriptor), alignof(TypeDescriptor)));
    if (!descCopy) {
        return false;
    }
    *descCopy = desc;
    
    // Allocate and copy properties if present
    if (desc.properties && desc.propertyCount > 0) {
        PropertyDescriptor* propsCopy = static_cast<PropertyDescriptor*>(
            te::core::Alloc(sizeof(PropertyDescriptor) * desc.propertyCount, alignof(PropertyDescriptor)));
        if (!propsCopy) {
            te::core::Free(descCopy);
            return false;
        }
        for (std::size_t i = 0; i < desc.propertyCount; ++i) {
            propsCopy[i] = desc.properties[i];
        }
        const_cast<TypeDescriptor*>(descCopy)->properties = propsCopy;
    }
    
    // Register
    registry.typesById[desc.id] = *descCopy;
    registry.typesByName[desc.name] = desc.id;
    registry.descriptors.push_back(*descCopy);
    
    return true;
}

TypeDescriptor const* TypeRegistry::GetTypeByName(char const* name) {
    if (!name) {
        return nullptr;
    }
    
    auto& registry = GetRegistry();
    std::shared_lock<std::shared_mutex> lock(registry.mutex);
    
    auto it = registry.typesByName.find(name);
    if (it == registry.typesByName.end()) {
        return nullptr;
    }
    
    auto it2 = registry.typesById.find(it->second);
    if (it2 == registry.typesById.end()) {
        return nullptr;
    }
    
    return &it2->second;
}

TypeDescriptor const* TypeRegistry::GetTypeById(TypeId id) {
    if (id == kInvalidTypeId) {
        return nullptr;
    }
    
    auto& registry = GetRegistry();
    std::shared_lock<std::shared_mutex> lock(registry.mutex);
    
    auto it = registry.typesById.find(id);
    if (it == registry.typesById.end()) {
        return nullptr;
    }
    
    return &it->second;
}

void* TypeRegistry::CreateInstance(TypeId id) {
    TypeDescriptor const* desc = GetTypeById(id);
    if (!desc || !desc->createInstance) {
        return nullptr;
    }
    
    return desc->createInstance();
}

void* TypeRegistry::CreateInstance(char const* typeName) {
    TypeDescriptor const* desc = GetTypeByName(typeName);
    if (!desc || !desc->createInstance) {
        return nullptr;
    }
    
    return desc->createInstance();
}

bool TypeRegistry::IsTypeRegistered(TypeId id) {
    return GetTypeById(id) != nullptr;
}

bool TypeRegistry::IsTypeRegistered(char const* name) {
    return GetTypeByName(name) != nullptr;
}

void TypeRegistry::EnumerateTypes(void (*callback)(TypeDescriptor const*, void*), void* userData) {
    if (!callback) {
        return;
    }
    
    auto& registry = GetRegistry();
    std::shared_lock<std::shared_mutex> lock(registry.mutex);
    
    for (auto const& pair : registry.typesById) {
        callback(&pair.second, userData);
    }
}

} // namespace object
} // namespace te
