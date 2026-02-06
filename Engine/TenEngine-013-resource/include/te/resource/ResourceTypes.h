// 013-Resource: ResourceType enum per ABI (te/resource/ResourceTypes.h)
#pragma once

namespace te {
namespace resource {

enum class ResourceType {
    Texture,
    Mesh,
    Material,
    Model,
    Effect,
    Terrain,
    Shader,
    Audio,
    Custom,
};

} // namespace resource
} // namespace te
