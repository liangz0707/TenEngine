// 013-Resource: DResource concept — GPU representation (te/resource/DResource.h)
#pragma once

// DResource: GPU-side resource; not used as cross-object reference.
// Stored inside RResource; RResource manages lifecycle and binding.
// 013 does not create DResource; 008/011/012/028 create in EnsureDeviceResources.

namespace te {
namespace resource {

// Placeholder type for DResource concept; concrete D* types in owning modules.
struct DResource {};

} // namespace resource
} // namespace te
