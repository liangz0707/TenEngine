/**
 * @file PropertyReflection.cpp
 * @brief Property reflection system implementation.
 */

#include <te/entity/PropertyReflection.h>
#include <unordered_map>
#include <mutex>
#include <vector>

namespace te {
namespace entity {

class PropertyRegistryImpl : public IPropertyRegistry {
public:
    void RegisterComponentMeta(const ComponentMeta& meta) override {
        std::lock_guard<std::mutex> lock(m_mutex);

        std::string key(meta.typeName);
        ComponentMeta copy = meta;
        copy.typeName = strdup(meta.typeName);  // Copy string for safety

        // Copy properties array
        if (meta.properties && meta.propertyCount > 0) {
            PropertyMeta* props = new PropertyMeta[meta.propertyCount];
            for (size_t i = 0; i < meta.propertyCount; ++i) {
                props[i] = meta.properties[i];
                if (meta.properties[i].name) {
                    props[i].name = strdup(meta.properties[i].name);
                }
            }
            copy.properties = props;
        }

        m_metas[key] = copy;
    }

    const ComponentMeta* GetComponentMeta(const char* typeName) const override {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_metas.find(typeName);
        if (it != m_metas.end()) {
            return &it->second;
        }
        return nullptr;
    }

    std::vector<const ComponentMeta*> GetAllComponentMetas() const override {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<const ComponentMeta*> result;
        for (auto const& pair : m_metas) {
            result.push_back(&pair.second);
        }
        return result;
    }

    bool HasComponentMeta(const char* typeName) const override {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_metas.find(typeName) != m_metas.end();
    }

private:
    mutable std::mutex m_mutex;
    std::unordered_map<std::string, ComponentMeta> m_metas;
};

static PropertyRegistryImpl* g_propertyRegistry = nullptr;

void InitializePropertyRegistry() {
    if (!g_propertyRegistry) {
        g_propertyRegistry = new PropertyRegistryImpl();
    }
}

void ShutdownPropertyRegistry() {
    delete g_propertyRegistry;
    g_propertyRegistry = nullptr;
}

IPropertyRegistry* GetPropertyRegistry() {
    return g_propertyRegistry;
}

}  // namespace entity
}  // namespace te
