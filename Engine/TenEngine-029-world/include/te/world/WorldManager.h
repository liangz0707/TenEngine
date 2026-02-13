/**
 * @file WorldManager.h
 * @brief 029-World: Level lifecycle, scene ref, traverse, collect renderables.
 * Contract: specs/_contracts/029-world-public-api.md
 */

#ifndef TE_WORLD_WORLD_MANAGER_H
#define TE_WORLD_WORLD_MANAGER_H

#include <te/world/WorldTypes.h>
#include <te/world/LevelAssetDesc.h>
#include <te/scene/SceneTypes.h>
#include <te/resource/ResourceId.h>
#include <te/scene/SceneManager.h>
#include <te/scene/SceneDesc.h>
#include <te/resource/ResourceManager.h>
#include <functional>
#include <vector>

namespace te {
namespace world {

/**
 * World manager: Level load/unload, current scene, traverse, collect renderables.
 * Single Level support in this version (GetCurrentLevelScene returns the only level's SceneRef).
 */
class WorldManager {
public:
    static WorldManager& GetInstance();

    /**
     * Create level from 029 LevelAssetDesc; converts to 004 SceneDesc, uses EntityManager and attaches ModelComponent per node.
     * @param indexType Spatial index type for the scene world
     * @param bounds World bounds
     * @param desc Level asset description (roots with name, transform, modelGuid, children)
     * @return LevelHandle (use GetSceneRef(handle) for SceneRef)
     */
    LevelHandle CreateLevelFromDesc(te::scene::SpatialIndexType indexType,
                                    te::core::AABB const& bounds,
                                    LevelAssetDesc const& desc);

    /**
     * Create level from resource ID; loads via 013 LoadSync(Level) then CreateLevelFromDesc(LevelAssetDesc).
     * @param indexType Spatial index type
     * @param bounds World bounds
     * @param levelResourceId Resource ID of .level (013 ResolvePath + LoadSync)
     * @return LevelHandle or invalid if load failed
     */
    LevelHandle CreateLevelFromDesc(te::scene::SpatialIndexType indexType,
                                    te::core::AABB const& bounds,
                                    te::resource::ResourceId levelResourceId);

    /** Unload level: destroy entities, then 004 UnloadScene, then release refs. */
    void UnloadLevel(LevelHandle handle);

    /** Get SceneRef for a level (for 004 Traverse / FindNodeByName). */
    te::scene::SceneRef GetSceneRef(LevelHandle handle) const;

    /** Get current level scene (single-level: returns the loaded level's SceneRef). */
    te::scene::SceneRef GetCurrentLevelScene() const;

    /** Get root nodes of a level (delegate to 004). */
    void GetRootNodes(LevelHandle handle, std::vector<te::scene::ISceneNode*>& out) const;

    /** Traverse level (delegate to 004). */
    void Traverse(LevelHandle handle, std::function<void(te::scene::ISceneNode*)> const& callback) const;

    /**
     * Export level scene to LevelAssetDesc (for Save).
     * @param handle Level handle
     * @param out Output LevelAssetDesc (roots with name, localTransform, modelGuid, children)
     * @return true on success
     */
    bool ExportLevelToDesc(LevelHandle handle, LevelAssetDesc& out) const;

    /**
     * Save level to file path.
     * Exports scene to LevelAssetDesc, creates LevelResource, calls IResourceManager::Save.
     * @param handle Level handle
     * @param path Output path (e.g. assets/levels/untitled.level)
     * @return true on success
     */
    bool SaveLevel(LevelHandle handle, char const* path) const;

private:
    WorldManager() = default;
    ~WorldManager() = default;
    WorldManager(WorldManager const&) = delete;
    WorldManager& operator=(WorldManager const&) = delete;

    struct LevelState {
        LevelHandle handle;
        te::scene::SceneRef sceneRef;
    };
    LevelState* FindLevel(LevelHandle handle) const;

    std::vector<LevelState> m_levels;
    LevelHandle m_currentLevel;
};

}  // namespace world
}  // namespace te

#endif  // TE_WORLD_WORLD_MANAGER_H
