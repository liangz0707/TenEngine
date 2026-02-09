/**
 * @file Resource.inl
 * @brief Template method implementations for IResource.
 */

#ifndef TE_RESOURCE_RESOURCE_INL
#define TE_RESOURCE_RESOURCE_INL

#include <te/resource/Resource.h>
#include <te/resource/ResourceManager.h>  // For LoadResult, LoadCompleteCallback
#include <te/object/Serializer.h>
#include <te/object/TypeRegistry.h>
#include <te/core/platform.h>
#include <te/core/alloc.h>
#include <te/core/thread.h>
#include <memory>
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>

namespace te {
namespace resource {

// Type trait for AssetDesc type names
// Resource modules should specialize this for their AssetDesc types
template<typename T>
struct AssetDescTypeName {
    // Default: return nullptr (must be specialized)
    static const char* Get() { return nullptr; }
};

template<typename T>
std::unique_ptr<T> IResource::LoadAssetDesc(char const* path) {
    if (!path) {
        return nullptr;
    }

    // Get type name from type trait
    const char* typeName = AssetDescTypeName<T>::Get();
    if (!typeName) {
        // Type name not provided - cannot deserialize
        return nullptr;
    }

    // Create instance
    std::unique_ptr<T> desc = std::make_unique<T>();
    if (!desc) {
        return nullptr;
    }

    // Use DeserializeFromFile helper (reads file and deserializes)
    if (!te::object::DeserializeFromFile(path, desc.get(), typeName,
                                         te::object::SerializationFormat::Binary)) {
        return nullptr;
    }

    return desc;
}

template<typename T>
bool IResource::SaveAssetDesc(char const* path, T const* desc) {
    if (!path || !desc) {
        return false;
    }

    // Get type name from type trait
    const char* typeName = AssetDescTypeName<T>::Get();
    if (!typeName) {
        return false;
    }

    // Get type descriptor to get TypeId
    te::object::TypeDescriptor const* typeDesc = te::object::TypeRegistry::GetTypeByName(typeName);
    if (!typeDesc) {
        return false;
    }

    // Create serializer (internal creation)
    std::unique_ptr<te::object::ISerializer> serializer(te::object::CreateBinarySerializer());
    if (!serializer) {
        return false;
    }

    // Serialize
    te::object::SerializedBuffer buffer{};
    if (!serializer->Serialize(buffer, desc, typeDesc->id)) {
        return false;
    }

    // Write to file (001-Core)
    if (!te::core::FileWrite(path, std::vector<std::uint8_t>(
            static_cast<std::uint8_t const*>(buffer.data),
            static_cast<std::uint8_t const*>(buffer.data) + buffer.size))) {
        // Free buffer
        if (buffer.data) {
            te::core::Free(buffer.data);
        }
        return false;
    }

    // Free buffer
    if (buffer.data) {
        te::core::Free(buffer.data);
    }

    return true;
}

template<typename T, typename GetDepsFn>
bool IResource::LoadDependencies(T const* desc, GetDepsFn getDeps, IResourceManager* manager) {
    if (!desc || !manager) {
        return false;
    }

    // Extract dependency list using function object
    std::vector<ResourceId> deps = getDeps(desc);
    if (deps.empty()) {
        return true;  // No dependencies
    }

    // Load dependencies (sync mode in Load, async mode in LoadAsync)
    // Check if we're in async context
    if (m_isLoadingAsync) {
        // Async mode: load dependencies asynchronously
        // For now, load synchronously (can be enhanced later)
        for (ResourceId const& depId : deps) {
            IResource* dep = LoadDependency(depId, manager);
            if (!dep) {
                return false;
            }
        }
    } else {
        // Sync mode: load dependencies synchronously
        for (ResourceId const& depId : deps) {
            IResource* dep = LoadDependency(depId, manager);
            if (!dep) {
                return false;
            }
        }
    }

    return true;
}

}  // namespace resource
}  // namespace te

#endif  // TE_RESOURCE_RESOURCE_INL
