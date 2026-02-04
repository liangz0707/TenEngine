// 004-Scene: Level API (contract). LoadLevel, UnloadLevel, LevelHandle; per 013-Resource when agreed.

#ifndef TENENGINE_SCENE_LEVEL_HPP
#define TENENGINE_SCENE_LEVEL_HPP

#include "scene/types.hpp"

namespace te {
namespace scene {

LevelHandle LoadLevel(WorldRef world, void* resourceHandleOrPath);
void UnloadLevel(WorldRef world, LevelHandle level);

}  // namespace scene
}  // namespace te

#endif
