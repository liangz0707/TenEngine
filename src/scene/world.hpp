// 004-Scene: World API (contract). GetCurrentWorld, SetActiveWorld, AddWorld.

#ifndef TENENGINE_SCENE_WORLD_HPP
#define TENENGINE_SCENE_WORLD_HPP

#include "scene/types.hpp"

namespace ten {
namespace scene {

WorldRef GetCurrentWorld();
void SetActiveWorld(WorldRef world);
WorldRef AddWorld();

// Internal: apply pending active world at start of UpdateTransforms (contract: effective next UpdateTransforms).
void ApplyPendingActiveWorld();

}  // namespace scene
}  // namespace ten

#endif
