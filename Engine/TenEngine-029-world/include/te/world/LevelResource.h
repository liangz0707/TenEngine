/**
 * @file LevelResource.h
 * @brief 029-World: ILevelResource and Level resource for 013 LoadSync(Level).
 * 013 Level factory returns IResource*; caller may cast to ILevelResource* to get LevelAssetDesc.
 */

#ifndef TE_WORLD_LEVEL_RESOURCE_H
#define TE_WORLD_LEVEL_RESOURCE_H

#include <te/world/LevelAssetDesc.h>

#include <te/resource/ResourceTypes.h>

namespace te {
namespace resource {
class IResource;
}
namespace world {

/**
 * @brief Level resource view: exposes LevelAssetDesc from a loaded Level resource.
 * 029 owns; 013 LoadSync(path, ResourceType::Level) returns IResource* that may implement this.
 */
class ILevelResource {
public:
    virtual ~ILevelResource() = default;
    /** @return Level asset description (valid for lifetime of this resource). */
    virtual LevelAssetDesc const& GetLevelAssetDesc() const = 0;
};

/** Factory for 013 RegisterResourceFactory(ResourceType::Level, ...). Returns IResource*. */
struct LevelResourceFactory {
    static te::resource::IResource* Create(te::resource::ResourceType type);
};

/**
 * @brief Create LevelResource from LevelAssetDesc for Save (Editor export flow).
 * Caller must Release() the returned resource after Save.
 */
te::resource::IResource* CreateLevelResourceFromDesc(LevelAssetDesc const& desc);

}  // namespace world
}  // namespace te

#endif  // TE_WORLD_LEVEL_RESOURCE_H
