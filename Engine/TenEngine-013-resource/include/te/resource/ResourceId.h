// 013-Resource: ResourceId (GUID equivalent) per ABI (te/resource/ResourceId.h)
#pragma once

#include <cstdint>
#include <cstddef>
#include <functional>

namespace te {
namespace resource {

// Opaque resource identifier; equivalent to GUID for FResource references and cache key.
struct ResourceId {
    static constexpr size_t kSize = 16;
    uint8_t data[kSize];

    bool operator==(ResourceId const& other) const;
    bool operator!=(ResourceId const& other) const { return !(*this == other); }
};

} // namespace resource
} // namespace te

// Hash support for use as map key
namespace std {
template<>
struct hash<te::resource::ResourceId> {
    size_t operator()(te::resource::ResourceId const& id) const;
};
} // namespace std
