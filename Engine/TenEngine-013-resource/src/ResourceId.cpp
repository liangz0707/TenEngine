// 013-Resource: ResourceId implementation
#include "te/resource/ResourceId.h"
#include <cstring>

namespace te {
namespace resource {

bool ResourceId::operator==(ResourceId const& other) const {
    return std::memcmp(data, other.data, kSize) == 0;
}

} // namespace resource
} // namespace te

namespace std {
size_t hash<te::resource::ResourceId>::operator()(te::resource::ResourceId const& id) const {
    size_t h = 0;
    for (size_t i = 0; i < te::resource::ResourceId::kSize; ++i)
        h = h * 31 + static_cast<size_t>(id.data[i]);
    return h;
}
} // namespace std
