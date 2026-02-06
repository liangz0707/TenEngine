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
        
        // Template method for registration (called from header template)
        template<typename T>
        void RegisterComponentTypeTemplate(char const* name) {
            RegisterComponentTypeInternal<T>(name);
        }
        
    private:
        std::unordered_map<te::object::TypeId, std::unique_ptr<IComponentTypeInfo>> m_typeInfos;
        std::unordered_map<std::string, te::object::TypeId> m_nameToTypeId;
        
        template<typename T>
        void RegisterComponentTypeInternal(char const* name) {
            // Register with Object TypeRegistry
            te::object::TypeDescriptor desc;
            desc.name = name;
            desc.size = sizeof(T);
            // Generate a type ID (simplified - in production use proper ID generation)
            static te::object::TypeId s_nextTypeId = 1000;
            desc.id = s_nextTypeId++;
            
            te::object::TypeRegistry::RegisterType(desc);
            
            // Store in component registry
            te::object::TypeId typeId = desc.id;
            if (typeId != 0) {
                m_typeInfos[typeId] = std::make_unique<IComponentTypeInfo>(
                    typeId, name, sizeof(T));
                m_nameToTypeId[std::string(name)] = typeId;
            }
        }
    };
    
    ComponentRegistryImpl* g_componentRegistry = nullptr;
}

IComponentRegistry* GetComponentRegistry() {
    if (!g_componentRegistry) {
        g_componentRegistry = new ComponentRegistryImpl();
    }
    return g_componentRegistry;
}

// Helper function to register component type (called from header template)
// Must be in te::entity namespace for template linking
template<typename T>
void RegisterComponentTypeHelper(char const* name) {
    // Access the implementation through GetComponentRegistry()
    IComponentRegistry* registry = GetComponentRegistry();
    if (registry) {
        // Cast to ComponentRegistryImpl (which is in anonymous namespace)
        // We know GetComponentRegistry() returns ComponentRegistryImpl*
        // Since ComponentRegistryImpl is in anonymous namespace, we need to
        // access it through a type-erased interface
        // For now, we'll use a workaround: store a function pointer
        static void(*s_registerFunc)(IComponentRegistry*, char const*) = nullptr;
        if (!s_registerFunc) {
            // Initialize: get the implementation and store a lambda
            ComponentRegistryImpl* impl = static_cast<ComponentRegistryImpl*>(registry);
            if (impl) {
                // We can't store template function pointer directly
                // So we'll call it directly here
                impl->RegisterComponentTypeTemplate<T>(name);
                return;
            }
        }
    }
}

}  // namespace entity
}  // namespace te
