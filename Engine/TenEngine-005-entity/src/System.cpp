/**
 * @file System.cpp
 * @brief ECS System implementation
 */

#include <te/entity/System.h>
#include <algorithm>

namespace te {
namespace entity {

namespace {
    SystemManager* g_systemManager = nullptr;
}

SystemManager& SystemManager::GetInstance() {
    if (!g_systemManager) {
        g_systemManager = new SystemManager();
    }
    return *g_systemManager;
}

SystemManager* GetSystemManager() {
    return g_systemManager;
}

void SystemManager::RegisterSystem(std::unique_ptr<System> system) {
    if (!system) {
        return;
    }
    
    SystemExecutionOrder order = system->GetExecutionOrder();
    m_systems.emplace_back(std::move(system), order);
    
    // Sort systems by execution order
    SortSystems();
    
    // Initialize the system
    m_systems.back().system->Initialize();
}

void SystemManager::UnregisterSystem(System* system) {
    if (!system) {
        return;
    }
    
    auto it = std::find_if(m_systems.begin(), m_systems.end(),
        [system](SystemEntry const& entry) {
            return entry.system.get() == system;
        });
    
    if (it != m_systems.end()) {
        it->system->Shutdown();
        m_systems.erase(it);
    }
}

void SystemManager::SetExecutionOrder(System* system, SystemExecutionOrder order) {
    if (!system) {
        return;
    }
    
    auto it = std::find_if(m_systems.begin(), m_systems.end(),
        [system](SystemEntry const& entry) {
            return entry.system.get() == system;
        });
    
    if (it != m_systems.end()) {
        it->order = order;
        SortSystems();
    }
}

void SystemManager::Update(float deltaTime) {
    for (auto& entry : m_systems) {
        entry.system->Update(deltaTime);
    }
}

void SystemManager::Initialize() {
    for (auto& entry : m_systems) {
        entry.system->Initialize();
    }
}

void SystemManager::Shutdown() {
    for (auto& entry : m_systems) {
        entry.system->Shutdown();
    }
    m_systems.clear();
}

void SystemManager::SortSystems() {
    std::sort(m_systems.begin(), m_systems.end(),
        [](SystemEntry const& a, SystemEntry const& b) {
            return static_cast<int>(a.order) < static_cast<int>(b.order);
        });
}

}  // namespace entity
}  // namespace te
