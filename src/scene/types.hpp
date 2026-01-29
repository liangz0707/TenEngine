// 004-Scene: contract-only types (specs/_contracts/004-scene-public-api.md)
// WorldRef, NodeId, LevelHandle, Transform; no extra public fields beyond contract.

#ifndef TENENGINE_SCENE_TYPES_HPP
#define TENENGINE_SCENE_TYPES_HPP

#include <cstddef>
#include <cstdint>

namespace ten {
namespace scene {

// Opaque handle: scene/world container. Lifecycle: until unload.
using WorldRef = void*;

// Opaque handle: scene graph node id. Lifecycle: same as node.
using NodeId = void*;

// Opaque handle: level load/unload boundary; per 013-Resource contract.
using LevelHandle = void*;

// Local/world transform; aligned with Core.Math or shared math type (contract).
// Minimal struct when Core not available; alias to Core type when linked.
struct Transform {
  float position[3] = {0.f, 0.f, 0.f};
  float rotation[4] = {0.f, 0.f, 0.f, 1.f};  // xyzw quaternion
  float scale[3] = {1.f, 1.f, 1.f};
};

}  // namespace scene
}  // namespace ten

#endif
