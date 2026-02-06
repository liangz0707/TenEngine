// 013-Resource: IResourceLoader interface per ABI (te/resource/ResourceLoader.h)
#pragma once

#include "te/resource/ResourceTypes.h"

namespace te {
namespace resource {

class IResource;
class IResourceManager;

class IResourceLoader {
public:
    virtual ~IResourceLoader() = default;
    // Creates IResource from opaque payload; 013 does not parse payload.
    virtual IResource* CreateFromPayload(ResourceType type, void* payload, IResourceManager* manager) = 0;
};

} // namespace resource
} // namespace te
