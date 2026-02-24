/**
 * @file Entity.h
 * @brief Entity class - implements ISceneNode for scene integration
 * Contract: specs/_contracts/005-entity-public-api.md
 */

#ifndef TE_ENTITY_ENTITY_H
#define TE_ENTITY_ENTITY_H

#include <te/scene/ISceneNode.h>
#include <te/scene/SceneTypes.h>
#include <te/entity/EntityId.h>
#include <te/entity/Component.h>
#include <te/entity/ComponentRegistry.h>
#include <te/object/TypeId.h>
#include <te/object/TypeRegistry.h>
#include <te/core/math.h>
#include <memory>
#include <unordered_map>
#include <vector>
#include <string>
#include <type_traits>
#include <typeindex>

namespace te {
namespace scene {
class SceneManager;
}  // namespace scene

namespace entity {

// Forward declarations
class EntityManager;

/**
 * @brief Entity class - implements ISceneNode interface
 * 
 * Entity represents a scene unit that can have components attached.
 * Entity implements ISceneNode interface to be managed by Scene module.
 * Each Entity corresponds to a Scene node (1:1 mapping).
 */
class Entity : public te::scene::ISceneNode {
public:
    /**
     * @brief Create a new Entity
     * @param world World reference
     * @param name Entity name (optional)
     * @return Created Entity, or nullptr on failure
     */
    static Entity* Create(te::scene::WorldRef world, char const* name = nullptr);
    
    /**
     * @brief Create Entity from existing Scene node
     * @param nodeId Scene node ID
     * @param world World reference
     * @return Created Entity, or nullptr on failure
     * 
     * Note: Resource-related operations should be handled by World module.
     */
    static Entity* CreateFromNode(te::scene::NodeId nodeId, te::scene::WorldRef world);
    
    /**
     * @brief Destroy Entity
     * 
     * Unregisters from Scene, cleans up components, and releases resources.
     */
    void Destroy();
    
    /**
     * @brief Get Entity ID
     * @return Entity ID
     */
    EntityId GetEntityId() const { return m_entityId; }
    
    /**
     * @brief Get Scene node (this Entity implements ISceneNode)
     * @return ISceneNode pointer (this)
     */
    te::scene::ISceneNode* GetSceneNode() { return this; }
    
    /**
     * @brief Get World reference
     * @return World reference
     */
    te::scene::WorldRef GetWorldRef() const { return m_world; }
    
    /**
     * @brief Set Entity enabled state
     * @param enabled Enabled state
     * 
     * This is equivalent to ISceneNode::SetActive.
     */
    void SetEnabled(bool enabled);
    
    /**
     * @brief Check if Entity is enabled
     * @return true if enabled
     * 
     * This is equivalent to ISceneNode::IsActive.
     */
    bool IsEnabled() const;
    
    // ========== Component Management ==========
    
    /**
     * @brief Add a component to this Entity
     * @tparam T Component type
     * @return Pointer to added component, or nullptr on failure
     */
    template<typename T>
    T* AddComponent();
    
    /**
     * @brief Get a component from this Entity
     * @tparam T Component type
     * @return Pointer to component, or nullptr if not found
     */
    template<typename T>
    T* GetComponent();
    
    /**
     * @brief Get a component (const version)
     * @tparam T Component type
     * @return Pointer to component, or nullptr if not found
     */
    template<typename T>
    T const* GetComponent() const;
    
    /**
     * @brief Remove a component from this Entity
     * @tparam T Component type
     */
    template<typename T>
    void RemoveComponent();
    
    /**
     * @brief Check if Entity has a component
     * @tparam T Component type
     * @return true if Entity has the component
     */
    template<typename T>
    bool HasComponent() const;

    /**
     * @brief Check if Entity has a component by TypeId (for Editor/reflection).
     * @param typeId Component type ID from TypeRegistry
     * @return true if Entity has the component
     */
    bool HasComponent(te::object::TypeId typeId) const { return HasComponentInternal(typeId); }

    /**
     * @brief Get a component by TypeId (for Editor/reflection).
     * @param typeId Component type ID from TypeRegistry
     * @return Pointer to component, or nullptr if not found
     */
    Component* GetComponent(te::object::TypeId typeId) { return GetComponentInternal(typeId); }
    Component const* GetComponent(te::object::TypeId typeId) const { return GetComponentInternal(typeId); }

    // ========== ISceneNode Interface Implementation ==========
    
    // Hierarchy
    te::scene::ISceneNode* GetParent() const override;
    void SetParent(te::scene::ISceneNode* parent) override;
    void GetChildren(std::vector<te::scene::ISceneNode*>& out) const override;
    size_t GetChildCount() const override;
    
    // Transform
    te::scene::Transform const& GetLocalTransform() const override;
    void SetLocalTransform(te::scene::Transform const& t) override;
    te::scene::Transform const& GetWorldTransform() const override;
    te::core::Matrix4 const& GetWorldMatrix() const override;
    
    // Node Identity
    te::scene::NodeId GetNodeId() const override;
    char const* GetName() const override;
    
    // Active State
    bool IsActive() const override;
    void SetActive(bool active) override;
    
    // Node Type
    te::scene::NodeType GetNodeType() const override;
    
    // AABB (optional)
    bool HasAABB() const override;
    te::core::AABB GetAABB() const override;
    
    // Dirty Flag
    bool IsDirty() const override;
    void SetDirty(bool dirty) override;
    
private:
    Entity(te::scene::WorldRef world, char const* name = nullptr);
    ~Entity() override;
    
    // Prevent copy and assignment
    Entity(Entity const&) = delete;
    Entity& operator=(Entity const&) = delete;
    
    // Internal component management
    Component* AddComponentInternal(te::object::TypeId typeId);
    Component* GetComponentInternal(te::object::TypeId typeId);
    Component const* GetComponentInternal(te::object::TypeId typeId) const;
    void RemoveComponentInternal(te::object::TypeId typeId);
    bool HasComponentInternal(te::object::TypeId typeId) const;
    
    // Component storage
    std::unordered_map<te::object::TypeId, std::unique_ptr<Component>> m_components;
    
    // Entity identity
    EntityId m_entityId;
    std::string m_name;
    
    // Scene node data
    te::scene::WorldRef m_world;
    te::scene::ISceneNode* m_parent;
    std::vector<Entity*> m_children;  // Children are also Entities
    
    // Transform data
    mutable te::scene::Transform m_localTransform;
    mutable te::scene::Transform m_worldTransform;
    mutable te::core::Matrix4 m_worldMatrix;
    mutable bool m_worldMatrixDirty;
    
    // State
    bool m_active;
    bool m_dirty;
    te::scene::NodeType m_nodeType;
    
    // AABB (optional)
    bool m_hasAABB;
    te::core::AABB m_aabb;
    
    // Friend classes
    friend class EntityManager;
};

// Helper function to get TypeId from component type
namespace detail {
    // Default implementation - returns empty string (will use typeid)
    template<typename T>
    struct ComponentTypeName {
        static constexpr char const* value = nullptr;
    };

    template<typename T>
    te::object::TypeId GetComponentTypeId() {
        // Try ComponentRegistry first with registered name
        IComponentRegistry* registry = GetComponentRegistry();

        // First try the explicit registered name if available
        if constexpr (ComponentTypeName<T>::value != nullptr) {
            if (registry) {
                IComponentTypeInfo const* info = registry->GetComponentTypeInfo(ComponentTypeName<T>::value);
                if (info) {
                    return info->typeId;
                }
            }

            // Try TypeRegistry with registered name
            te::object::TypeDescriptor const* desc = te::object::TypeRegistry::GetTypeByName(ComponentTypeName<T>::value);
            if (desc) {
                return desc->id;
            }
        }

        // Fallback: use typeid name
        if (registry) {
            char const* typeName = typeid(T).name();
            IComponentTypeInfo const* info = registry->GetComponentTypeInfo(typeName);
            if (info) {
                return info->typeId;
            }
        }

        // Fallback to TypeRegistry with typeid name
        char const* typeName = typeid(T).name();
        te::object::TypeDescriptor const* desc = te::object::TypeRegistry::GetTypeByName(typeName);
        if (desc) {
            return desc->id;
        }

        return 0;
    }
}

// Macro to register component type name for template lookup
// Must be used at global scope (outside any namespace)
#define TE_REGISTER_COMPONENT_TYPE_NAME(T, name) \
    namespace te { namespace entity { namespace detail { \
        template<> struct ComponentTypeName<T> { \
            static constexpr char const* value = name; \
        }; \
    }}}

// Template implementations
template<typename T>
T* Entity::AddComponent() {
    static_assert(std::is_base_of_v<Component, T>, "T must inherit from Component");
    
    te::object::TypeId typeId = detail::GetComponentTypeId<T>();
    if (typeId == 0) {
        return nullptr;
    }
    
    // Check if component already exists
    Component* existing = GetComponentInternal(typeId);
    if (existing) {
        return static_cast<T*>(existing);
    }
    
    // Create new component instance
    void* instance = te::object::TypeRegistry::CreateInstance(typeId);
    if (!instance) {
        return nullptr;
    }
    
    Component* comp = static_cast<Component*>(instance);
    m_components[typeId] = std::unique_ptr<Component>(comp);
    comp->OnAttached(this);
    
    return static_cast<T*>(comp);
}

template<typename T>
T* Entity::GetComponent() {
    static_assert(std::is_base_of_v<Component, T>, "T must inherit from Component");
    
    te::object::TypeId typeId = detail::GetComponentTypeId<T>();
    if (typeId == 0) {
        return nullptr;
    }
    
    return static_cast<T*>(GetComponentInternal(typeId));
}

template<typename T>
T const* Entity::GetComponent() const {
    static_assert(std::is_base_of_v<Component, T>, "T must inherit from Component");
    
    te::object::TypeId typeId = detail::GetComponentTypeId<T>();
    if (typeId == 0) {
        return nullptr;
    }
    
    return static_cast<T const*>(GetComponentInternal(typeId));
}

template<typename T>
void Entity::RemoveComponent() {
    static_assert(std::is_base_of_v<Component, T>, "T must inherit from Component");
    
    te::object::TypeId typeId = detail::GetComponentTypeId<T>();
    if (typeId == 0) {
        return;
    }
    
    Component* comp = GetComponentInternal(typeId);
    if (comp) {
        comp->OnDetached(this);
        RemoveComponentInternal(typeId);
    }
}

template<typename T>
bool Entity::HasComponent() const {
    static_assert(std::is_base_of_v<Component, T>, "T must inherit from Component");
    
    te::object::TypeId typeId = detail::GetComponentTypeId<T>();
    if (typeId == 0) {
        return false;
    }
    
    return HasComponentInternal(typeId);
}

}  // namespace entity
}  // namespace te

#endif  // TE_ENTITY_ENTITY_H
