/**
 * @file DResource.h
 * @brief GPU-side resource (contract: specs/_contracts/013-resource-ABI.md).
 */
#ifndef TE_RESOURCE_DRESOURCE_H
#define TE_RESOURCE_DRESOURCE_H

/** DResource: GPU resource; stored inside RResource, managed by 011/012/028; 013 does not create. */
namespace te {
namespace resource {
struct DResource;  // Concept; 013 does not create or hold.
}  // namespace resource
}  // namespace te

#endif  // TE_RESOURCE_DRESOURCE_H
