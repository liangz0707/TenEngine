/**
 * @file System.h
 * @brief ECS System base class and SystemManager
 * Contract: specs/_contracts/005-entity-public-api.md
 */

#ifndef TE_ENTITY_SYSTEM_H
#define TE_ENTITY_SYSTEM_H

#include <te/entity/Entity.h>
#include <memory>
#include <vector>
#include <functional>

namespace te {
namespace entity {

/**
 * @brief System execution order
 */
enum class SystemExecutionOrder {
    PreUpdate = 0,
    Update = 100,
    PostUpdate = 200,
    Render = 300,
    PostRender = 400
};

/**
 * @brief System base class
 * 
 * All ECS systems inherit from System.
 * Systems are registered with SystemManager and executed in order.
 */
class System {
public:
    virtual ~System() = default;
    
    /**
     * @brief Get system execution order
     * @return Execution order
     */
    virtual SystemExecutionOrder GetExecutionOrder() const { return SystemExecutionOrder::Update; }
    
    /**
     * @brief Update system
     * @param deltaTime Time since last update (seconds)
     * 
     * Called every frame by SystemManager.
     */
    virtual void Update(float deltaTime) {}
    
    /**
     * @brief Initialize system
     * 
     * Called once when system is registered.
     */
    virtual void Initialize() {}
    
    /**
     * @brief Shutdown system
     * 
     * Called once when system is unregistered.
     */
    virtual void Shutdown() {}
};

/**
 * @brief System manager
 * 
 * Manages system registration, execution order, and tick integration.
 */
class SystemManager {
public:
    /**
     * @brief Get singleton instance
     * @return System manager instance
     */
    static SystemManager& GetInstance();
    
    /**
     * @brief Register a system
     * @param system System to register (ownership transferred)
     * 
     * Systems are executed in order based on GetExecutionOrder().
     */
    void RegisterSystem(std::unique_ptr<System> system);
    
    /**
     * @brief Unregister a system
     * @param system System to unregister
     */
    void UnregisterSystem(System* system);
    
    /**
     * @brief Set execution order for a system
     * @param system System to set order for
     * @param order Execution order
     */
    void SetExecutionOrder(System* system, SystemExecutionOrder order);
    
    /**
     * @brief Update all systems
     * @param deltaTime Time since last update (seconds)
     * 
     * Systems are updated in order based on their execution order.
     */
    void Update(float deltaTime);
    
    /**
     * @brief Initialize all systems
     */
    void Initialize();
    
    /**
     * @brief Shutdown all systems
     */
    void Shutdown();
    
private:
    SystemManager() = default;
    ~SystemManager() = default;
    SystemManager(SystemManager const&) = delete;
    SystemManager& operator=(SystemManager const&) = delete;
    
    struct SystemEntry {
        std::unique_ptr<System> system;
        SystemExecutionOrder order;
        
        SystemEntry(std::unique_ptr<System> s, SystemExecutionOrder o)
            : system(std::move(s)), order(o) {}
    };
    
    std::vector<SystemEntry> m_systems;
    
    // Sort systems by execution order
    void SortSystems();
};

/**
 * @brief Get system manager instance
 * @return System manager pointer
 */
SystemManager* GetSystemManager();

}  // namespace entity
}  // namespace te

#endif  // TE_ENTITY_SYSTEM_H
