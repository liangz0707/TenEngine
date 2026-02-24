/**
 * @file PrefabSystem.cpp
 * @brief Prefab system implementation (024-Editor).
 */
#include <te/editor/PrefabSystem.h>
#include <te/core/log.h>
#include <imgui.h>
#include <map>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <sstream>

namespace te {
namespace editor {

class PrefabSystemImpl : public IPrefabSystem {
public:
  PrefabSystemImpl()
    : m_nextPrefabId(1)
  {
  }

  // === Creation ===

  uint64_t CreatePrefabFromEntity(te::entity::EntityId entityId,
                                  char const* path) override {
    if (!path) return 0;

    // TODO: Integrate with Entity system to extract component data
    // For now, create a basic prefab entry

    uint64_t prefabId = m_nextPrefabId++;

    PrefabData data;
    data.id = prefabId;
    data.path = path;

    // Extract name from path
    std::string pathStr(path);
    size_t lastSlash = pathStr.find_last_of("/\\");
    size_t lastDot = pathStr.find_last_of('.');
    if (lastSlash != std::string::npos && lastDot != std::string::npos) {
      data.name = pathStr.substr(lastSlash + 1, lastDot - lastSlash - 1);
    }

    m_prefabs[prefabId] = data;

    // Store entity as template (placeholder)
    m_templateEntities[prefabId] = entityId;

    te::core::Log(te::core::LogLevel::Info,
                  ("PrefabSystem: Created prefab " + std::string(path)).c_str());

    if (m_onPrefabChanged) {
      m_onPrefabChanged(prefabId);
    }

    return prefabId;
  }

  uint64_t CreateEmptyPrefab(char const* path) override {
    if (!path) return 0;

    uint64_t prefabId = m_nextPrefabId++;

    PrefabData data;
    data.id = prefabId;
    data.path = path;

    // Extract name from path
    std::string pathStr(path);
    size_t lastSlash = pathStr.find_last_of("/\\");
    size_t lastDot = pathStr.find_last_of('.');
    if (lastSlash != std::string::npos && lastDot != std::string::npos) {
      data.name = pathStr.substr(lastSlash + 1, lastDot - lastSlash - 1);
    }

    m_prefabs[prefabId] = data;

    return prefabId;
  }

  te::entity::EntityId InstantiatePrefab(uint64_t prefabId,
                                         math::Vec3 const& position) override {
    auto it = m_prefabs.find(prefabId);
    if (it == m_prefabs.end()) {
      te::core::Log(te::core::LogLevel::Warn,
                    "PrefabSystem: Prefab not found");
      return te::entity::EntityId();  // Invalid entity ID
    }

    // TODO: Integrate with EntityManager to create actual entity
    // For now, return placeholder

    PrefabInstance instance;
    instance.prefabId = prefabId;
    instance.localPosition[0] = position.x;
    instance.localPosition[1] = position.y;
    instance.localPosition[2] = position.z;
    instance.isDirty = false;

    // Create new entity ID (placeholder)
    te::entity::EntityId instanceId(reinterpret_cast<void*>(static_cast<uintptr_t>(m_nextPrefabId + 10000)));
    m_instances[instanceId] = instance;

    te::core::Log(te::core::LogLevel::Info,
                  ("PrefabSystem: Instantiated prefab " + it->second.name).c_str());

    if (m_onInstanceChanged) {
      m_onInstanceChanged(instanceId);
    }

    return instanceId;
  }

  // === Query ===

  PrefabData const* GetPrefab(uint64_t prefabId) const override {
    auto it = m_prefabs.find(prefabId);
    if (it != m_prefabs.end()) {
      return &it->second;
    }
    return nullptr;
  }

  PrefabData const* GetPrefabByName(char const* name) const override {
    if (!name) return nullptr;

    for (auto const& pair : m_prefabs) {
      if (pair.second.name == name) {
        return &pair.second;
      }
    }
    return nullptr;
  }

  std::vector<PrefabData const*> GetAllPrefabs() const override {
    std::vector<PrefabData const*> result;
    for (auto const& pair : m_prefabs) {
      result.push_back(&pair.second);
    }
    return result;
  }

  bool IsPrefabInstance(te::entity::EntityId entityId) const override {
    return m_instances.find(entityId) != m_instances.end();
  }

  PrefabInstance const* GetPrefabInstance(te::entity::EntityId entityId) const override {
    auto it = m_instances.find(entityId);
    if (it != m_instances.end()) {
      return &it->second;
    }
    return nullptr;
  }

  uint64_t GetSourcePrefab(te::entity::EntityId entityId) const override {
    auto it = m_instances.find(entityId);
    if (it != m_instances.end()) {
      return it->second.prefabId;
    }
    return 0;
  }

  // === Editing ===

  bool ApplyToPrefab(te::entity::EntityId instanceId) override {
    auto instIt = m_instances.find(instanceId);
    if (instIt == m_instances.end()) return false;

    auto prefabIt = m_prefabs.find(instIt->second.prefabId);
    if (prefabIt == m_prefabs.end()) return false;

    // TODO: Copy instance data back to prefab template

    instIt->second.isDirty = false;

    if (m_onPrefabChanged) {
      m_onPrefabChanged(instIt->second.prefabId);
    }

    te::core::Log(te::core::LogLevel::Info,
                  "PrefabSystem: Applied instance changes to prefab");
    return true;
  }

  bool RevertToPrefab(te::entity::EntityId instanceId) override {
    auto instIt = m_instances.find(instanceId);
    if (instIt == m_instances.end()) return false;

    // TODO: Restore from prefab template

    instIt->second.isDirty = false;

    if (m_onInstanceChanged) {
      m_onInstanceChanged(instanceId);
    }

    te::core::Log(te::core::LogLevel::Info,
                  "PrefabSystem: Reverted instance to prefab");
    return true;
  }

  bool BreakPrefabInstance(te::entity::EntityId instanceId) override {
    auto it = m_instances.find(instanceId);
    if (it == m_instances.end()) return false;

    m_instances.erase(it);

    te::core::Log(te::core::LogLevel::Info,
                  "PrefabSystem: Broke prefab connection");
    return true;
  }

  bool AddOverride(te::entity::EntityId instanceId,
                   PrefabOverride const& override) override {
    auto it = m_instances.find(instanceId);
    if (it == m_instances.end()) return false;

    m_overrides[instanceId].push_back(override);
    it->second.isDirty = true;

    return true;
  }

  std::vector<PrefabOverride> GetOverrides(te::entity::EntityId instanceId) const override {
    auto it = m_overrides.find(instanceId);
    if (it != m_overrides.end()) {
      return it->second;
    }
    return {};
  }

  // === Nested Prefabs ===

  bool IsNestedPrefab(uint64_t prefabId) const override {
    return m_nestedParents.find(prefabId) != m_nestedParents.end();
  }

  uint64_t GetParentPrefab(uint64_t prefabId) const override {
    auto it = m_nestedParents.find(prefabId);
    if (it != m_nestedParents.end()) {
      return it->second;
    }
    return 0;
  }

  std::vector<uint64_t> GetNestedPrefabs(uint64_t prefabId) const override {
    std::vector<uint64_t> result;
    auto it = m_nestedChildren.find(prefabId);
    if (it != m_nestedChildren.end()) {
      for (uint64_t child : it->second) {
        result.push_back(child);
      }
    }
    return result;
  }

  // === Variants ===

  uint64_t CreateVariant(uint64_t basePrefabId, char const* path) override {
    auto baseIt = m_prefabs.find(basePrefabId);
    if (baseIt == m_prefabs.end()) return 0;

    uint64_t variantId = CreateEmptyPrefab(path);
    if (variantId == 0) return 0;

    // Link variant to base
    m_variantBase[variantId] = basePrefabId;

    te::core::Log(te::core::LogLevel::Info,
                  "PrefabSystem: Created prefab variant");
    return variantId;
  }

  // === Persistence ===

  bool SavePrefab(uint64_t prefabId) override {
    auto it = m_prefabs.find(prefabId);
    if (it == m_prefabs.end()) return false;

    // TODO: Serialize prefab data to file
    // For now, just log
    te::core::Log(te::core::LogLevel::Info,
                  ("PrefabSystem: Saved prefab to " + it->second.path).c_str());
    return true;
  }

  uint64_t LoadPrefab(char const* path) override {
    if (!path) return 0;

    // Check if already loaded
    for (auto const& pair : m_prefabs) {
      if (pair.second.path == path) {
        return pair.first;
      }
    }

    // TODO: Deserialize from file
    // For now, create empty
    return CreateEmptyPrefab(path);
  }

  bool ReloadPrefab(uint64_t prefabId) override {
    auto it = m_prefabs.find(prefabId);
    if (it == m_prefabs.end()) return false;

    // TODO: Reload from disk

    // Update all instances
    for (auto& instPair : m_instances) {
      if (instPair.second.prefabId == prefabId) {
        instPair.second.isDirty = false;
        if (m_onInstanceChanged) {
          m_onInstanceChanged(instPair.first);
        }
      }
    }

    return true;
  }

  // === Events ===

  void SetOnPrefabChanged(std::function<void(uint64_t)> callback) override {
    m_onPrefabChanged = std::move(callback);
  }

  void SetOnInstanceChanged(std::function<void(te::entity::EntityId)> callback) override {
    m_onInstanceChanged = std::move(callback);
  }

private:
  uint64_t m_nextPrefabId;

  std::map<uint64_t, PrefabData> m_prefabs;
  std::unordered_map<te::entity::EntityId, PrefabInstance, te::entity::EntityId::Hash> m_instances;
  std::unordered_map<te::entity::EntityId, std::vector<PrefabOverride>, te::entity::EntityId::Hash> m_overrides;
  std::map<uint64_t, te::entity::EntityId> m_templateEntities;

  // Nested prefab relationships
  std::map<uint64_t, uint64_t> m_nestedParents;    // child -> parent
  std::map<uint64_t, std::vector<uint64_t>> m_nestedChildren;  // parent -> children

  // Variant relationships
  std::map<uint64_t, uint64_t> m_variantBase;  // variant -> base

  std::function<void(uint64_t)> m_onPrefabChanged;
  std::function<void(te::entity::EntityId)> m_onInstanceChanged;
};

IPrefabSystem* CreatePrefabSystem() {
  return new PrefabSystemImpl();
}

}  // namespace editor
}  // namespace te
