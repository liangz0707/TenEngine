/**
 * @file FResource.h
 * @brief Disk representation of resource (contract: specs/_contracts/013-resource-ABI.md).
 */
#ifndef TE_RESOURCE_FRESOURCE_H
#define TE_RESOURCE_FRESOURCE_H

/** FResource: on-disk resource representation; references other resources only via global unique GUID. */
namespace te {
namespace resource {
struct FResource;  // Concept / type; concrete layout per asset format.
}  // namespace resource
}  // namespace te

#endif  // TE_RESOURCE_FRESOURCE_H
