/**
 * @file RResource.h
 * @brief Runtime/memory representation of resource (contract: specs/_contracts/013-resource-ABI.md).
 */
#ifndef TE_RESOURCE_RRESOURCE_H
#define TE_RESOURCE_RRESOURCE_H

/** RResource: in-memory resource representation; DResource is stored inside RResource. */
namespace te {
namespace resource {
struct RResource;  // Concept; IResource is the interface to RResource.
}  // namespace resource
}  // namespace te

#endif  // TE_RESOURCE_RRESOURCE_H
