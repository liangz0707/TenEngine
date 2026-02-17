/**
 * @file PrefabSystem.h
 * @brief Prefab system interface for reusable entity templates.
 */
#ifndef TE_EDITOR_PREFAB_SYSTEM_H
#define TE_EDITOR_PREFAB_SYSTEM_H

#include <te/editor/EditorTypes.h>
#include <te/entity/EntityId.h>
#include <te/core/math.h>
#include <functional>
#include <vector>
#include <cstdint>

namespace te {
namespace editor {

/**
 * @brief Prefab instance data.
 */
struct PrefabInstance {
  uint64_t prefabId = 0;            ///< Source prefab ID
  te::entity::EntityId instanceEntity;
  bool isDirty = false;             ///< Has local overrides
  math::Vec3 localPosition = math::Vec3(0, 0, 0);
  math::Vec3 localRotation = math::Vec3(0, 0, 0);
  math::Vec3 localScale = math::Vec3(1, 1, 1);
};

/**
 * @brief Prefab override data.
 */
struct PrefabOverride {
  enum class Type { Property, Component, AddedComponent, RemovedComponent };
  Type type;
  uint64_t componentId = 0;
  const char* propertyName = nullptr;
  // Value storage would be handled by variant type in production
};

/**
 * @brief Prefab asset data.
 */
struct PrefabData {
  uint64_t id = 0;
  char name[64] = "";
  char path[256] = "";
  
  // Template data
  std::vector<uint64_t> componentTypes;
  // Component default values would be stored here
  
  // Metadata
  uint64_t lastModified = 0;
  uint32_t version = 1;
};

/**
 * @brief Prefab system interface.
 * 
 * Provides prefab creation, instantiation, and management similar to Unity's prefab system.
 */
class IPrefabSystem {
public:
  virtual ~IPrefabSystem() = default;
  
  // === Creation ===
  
  /**
   * @brief Create a prefab from an existing entity.
   * @param entityId Entity to create prefab from
   * @param path Path to save prefab
   * @return Prefab ID, 0 on failure
   */
  virtual uint64_t CreatePrefabFromEntity(te::entity::EntityId entityId, 
                                          char const* path) = 0;
  
  /**
   * @brief Create an empty prefab.
   * @param path Path for the prefab
   * @return Prefab ID, 0 on failure
   */
  virtual uint64_t CreateEmptyPrefab(char const* path) = 0;
  
  /**
   * @brief Instantiate a prefab in the scene.
   * @param prefabId Prefab to instantiate
   * @param position World position
   * @return Entity ID of the new instance
   */
  virtual te::entity::EntityId InstantiatePrefab(uint64_t prefabId,
                                                  math::Vec3 const& position) = 0;
  
  // === Query ===
  
  /**
   * @brief Get prefab data by ID.
   */
  virtual PrefabData const* GetPrefab(uint64_t prefabId) const = 0;
  
  /**
   * @brief Get prefab by name.
   */
  virtual PrefabData const* GetPrefabByName(char const* name) const = 0;
  
  /**
   * @brief Get all prefabs.
   */
  virtual std::vector<PrefabData const*> GetAllPrefabs() const = 0;
  
  /**
   * @brief Check if entity is a prefab instance.
   */
  virtual bool IsPrefabInstance(te::entity::EntityId entityId) const = 0;
  
  /**
   * @brief Get prefab instance data.
   */
  virtual PrefabInstance const* GetPrefabInstance(te::entity::EntityId entityId) const = 0;
  
  /**
   * @brief Get source prefab for an instance.
   */
  virtual uint64_t GetSourcePrefab(te::entity::EntityId entityId) const = 0;
  
  // === Editing ===
  
  /**
   * @brief Apply changes from instance to prefab.
   */
  virtual bool ApplyToPrefab(te::entity::EntityId instanceId) = 0;
  
  /**
   * @brief Revert instance to match prefab.
   */
  virtual bool RevertToPrefab(te::entity::EntityId instanceId) = 0;
  
  /**
   * @brief Break prefab connection.
   */
  virtual bool BreakPrefabInstance(te::entity::EntityId instanceId) = 0;
  
  /**
   * @brief Add override to instance.
   */
  virtual bool AddOverride(te::entity::EntityId instanceId, 
                          PrefabOverride const& override) = 0;
  
  /**
   * @brief Get overrides for an instance.
   */
  virtual std::vector<PrefabOverride> GetOverrides(te::entity::EntityId instanceId) const = 0;
  
  // === Nested Prefabs ===
  
  /**
   * @brief Check if prefab is nested.
   */
  virtual bool IsNestedPrefab(uint64_t prefabId) const = 0;
  
  /**
   * @brief Get parent prefab.
   */
  virtual uint64_t GetParentPrefab(uint64_t prefabId) const = 0;
  
  /**
   * @brief Get nested prefabs.
   */
  virtual std::vector<uint64_t> GetNestedPrefabs(uint64_t prefabId) const = 0;
  
  // === Variants ===
  
  /**
   * @brief Create a prefab variant.
   * @param basePrefabId Base prefab
   * @param path Path for variant
   * @return Variant prefab ID
   */
  virtual uint64_t CreateVariant(uint64_t basePrefabId, char const* path) = 0;
  
  // === Persistence ===
  
  /**
   * @brief Save prefab to disk.
   */
  virtual bool SavePrefab(uint64_t prefabId) = 0;
  
  /**
   * @brief Load prefab from disk.
   */
  virtual uint64_t LoadPrefab(char const* path) = 0;
  
  /**
   * @brief Reload prefab from disk.
   */
  virtual bool ReloadPrefab(uint64_t prefabId) = 0;
  
  // === Events ===
  
  /**
   * @brief Set callback for prefab changes.
   */
  virtual void SetOnPrefabChanged(std::function<void(uint64_t)> callback) = 0;
  
  /**
   * @brief Set callback for instance changes.
   */
  virtual void SetOnInstanceChanged(std::function<void(te::entity::EntityId)> callback) = 0;
};

/**
 * @brief Factory function to create prefab system.
 */
IPrefabSystem* CreatePrefabSystem();

}  // namespace editor
}  // namespace te

#endif  // TE_EDITOR_PREFAB_SYSTEM_H
