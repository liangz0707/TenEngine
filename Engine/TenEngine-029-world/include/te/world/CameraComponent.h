/**
 * @file CameraComponent.h
 * @brief 029-World: Camera component (viewport, FOV, near/far).
 * Contract: specs/_contracts/029-world-public-api.md, 005-entity ComponentUsageGuide.
 */

#ifndef TE_WORLD_CAMERA_COMPONENT_H
#define TE_WORLD_CAMERA_COMPONENT_H

#include <te/entity/Component.h>

namespace te {
namespace world {

/**
 * @brief Component holding camera data; isActive indicates current rendering camera when FrameContext.camera is null.
 */
struct CameraComponent : public te::entity::Component {
  float fovY{1.0472f};      // ~60 deg
  float nearZ{0.1f};
  float farZ{1000.f};
  bool isActive{false};

  CameraComponent() = default;
};

}  // namespace world
}  // namespace te

#endif  // TE_WORLD_CAMERA_COMPONENT_H
