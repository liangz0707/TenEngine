// 013-Resource: IResource interface per ABI (te/resource/Resource.h)
#pragma once

#include "te/resource/ResourceTypes.h"

namespace te {
namespace resource {

class IResource {
public:
    virtual ~IResource() = default;
    virtual ResourceType GetResourceType() const = 0;
    virtual void Release() = 0;
};

} // namespace resource
} // namespace te
