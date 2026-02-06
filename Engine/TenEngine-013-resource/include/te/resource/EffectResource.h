// 013-Resource: IEffectResource type view (te/resource/EffectResource.h)
#pragma once

#include "te/resource/Resource.h"

namespace te {
namespace resource {

// Effect resource view; cast from IResource* when GetResourceType() == ResourceType::Effect.
struct IEffectResource : IResource {};

} // namespace resource
} // namespace te
