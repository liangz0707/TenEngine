// 013-Resource: FResource concept — disk representation (te/resource/FResource.h)
#pragma once

// FResource: resource on disk; references other resources only by GUID.
// Used during load from disk. Some resources may exist only in F form.
// This header declares the concept; concrete F* types are defined by owning modules.

namespace te {
namespace resource {

// Placeholder type for FResource concept; concrete disk formats per module.
struct FResource {};

} // namespace resource
} // namespace te
