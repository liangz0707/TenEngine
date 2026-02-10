/**
 * @file ComponentRegistry.cpp
 * @brief Component registry implementation
 */

#include <te/entity/ComponentRegistry.h>
#include <te/object/TypeRegistry.h>
#include <unordered_map>
#include <string>
#include <memory>

namespace te {
namespace entity {

namespace {
    class ComponentRegistryImpl : public IComponentRegistry {
    public:
        ComponentRegistryImpl() = default;
        ~ComponentRegistryImpl() override = default;
        
        IComponentTypeInfo const* GetComponentTypeInfo(te::object::TypeId id) const override {
            auto it = m_typeInfos.find(id);
            if (it != m_typeInfos.end()) {
                return it->second.get();
            }
            return nullptr;
        }
        
        IComponentTypeInfo const* GetComponentTypeInfo(char const* name) const override {
            auto it = m_nameToTypeId.find(std::string(name));
            if (it != m_nameToTypeId.end()) {
                return GetComponentTypeInfo(it->second);
            }
            return nullptr;
        }
        
        bool IsComponentTypeRegistered(te::object::TypeId id) const override {
            return m_typeInfos.find(id) != m_typeInfos.end();
        }

        void RegisterComponentTypeByNameAndSize(char const* name, std::size_t size) override {
            te::object::TypeDescriptor desc;
            desc.name = name;
            desc.size = size;
            static te::object::TypeId s_nextTypeId = 1000;
            desc.id = s_nextTypeId++;

            te::object::TypeRegistry::RegisterType(desc);

            te::object::TypeId typeId = desc.id;
            if (typeId != 0) {
                m_typeInfos[typeId] = std::make_unique<IComponentTypeInfo>(typeId, name, size);
                m_nameToTypeId[std::string(name)] = typeId;
            }
        }

    private:
        std::unordered_map<te::object::TypeId, std::unique_ptr<IComponentTypeInfo>> m_typeInfos;
        std::unordered_map<std::string, te::object::TypeId> m_nameToTypeId;
    };
    
    ComponentRegistryImpl* g_componentRegistry = nullptr;
}

IComponentRegistry* GetComponentRegistry() {
    if (!g_componentRegistry) {
        g_componentRegistry = new ComponentRegistryImpl();
    }
    return g_componentRegistry;
}

}  // namespace entity
}  // namespace te
