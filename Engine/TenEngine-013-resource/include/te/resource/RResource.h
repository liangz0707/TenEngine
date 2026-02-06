// 013-Resource: RResource concept — runtime/memory representation (te/resource/RResource.h)
#pragma once

// RResource: resource in memory; references other resources by pointer.
// DResource is stored inside RResource; RResource manages lifetime.
// This header declares the concept; IResource is the runtime handle (see Resource.h).

namespace te {
namespace resource {

// Placeholder type for RResource concept; runtime handle is IResource*.
struct RResource {};

} // namespace resource
} // namespace te
