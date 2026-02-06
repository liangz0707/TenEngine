// 013-Resource: IMeshResource type view (te/resource/MeshResource.h)
#pragma once

#include "te/resource/Resource.h"

namespace te {
namespace resource {

// Mesh resource view; cast from IResource* when GetResourceType() == ResourceType::Mesh.
// Concrete fields (vertices, indices, submeshes, LOD) defined by 012-Mesh.
struct IMeshResource : IResource {};

} // namespace resource
} // namespace te
