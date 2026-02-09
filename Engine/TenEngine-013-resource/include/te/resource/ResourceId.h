/**
 * @file ResourceId.h
 * @brief Resource global unique ID / GUID (contract: specs/_contracts/013-resource-ABI.md).
 * 
 * ResourceId is an alias for object::GUID, providing unified resource identification
 * across the engine. All resource references use ResourceId/GUID.
 */
#ifndef TE_RESOURCE_RESOURCE_ID_H
#define TE_RESOURCE_RESOURCE_ID_H

#include <te/object/Guid.h>
#include <functional>
#include <cstddef>

namespace te {
namespace resource {

/**
 * Resource global unique identifier.
 * 
 * ResourceId is an alias for object::GUID, providing direct access to all GUID functionality
 * including generation, parsing, and comparison operations.
 * 
 * Usage:
 *   ResourceId id = ResourceId::Generate();
 *   ResourceId id2 = ResourceId::FromString("...");
 *   std::string str = id.ToString();
 */
using ResourceId = object::GUID;

}  // namespace resource
}  // namespace te

// Hash specialization for ResourceId (GUID) to use in unordered_map
namespace std {
template<>
struct hash<te::resource::ResourceId> {
    std::size_t operator()(te::resource::ResourceId const& id) const noexcept {
        // Hash the 16-byte GUID data
        std::size_t h = 0;
        for (std::size_t i = 0; i < 16; ++i) {
            h ^= std::hash<std::uint8_t>{}(id.data[i]) + 0x9e3779b9 + (h << 6) + (h >> 2);
        }
        return h;
    }
};
} // namespace std

#endif  // TE_RESOURCE_RESOURCE_ID_H
