// 013-Resource: IDeserializer (DEPRECATED — use IResourceSerializer and RegisterSerializer)
// 序列化与反序列化已统一为 IResourceSerializer（Deserialize + Serialize），见 te/resource/ResourceSerializer.h
#pragma once

#include <cstddef>

namespace te {
namespace resource {

class IDeserializer {
public:
    virtual ~IDeserializer() = default;
    virtual void* Deserialize(void const* buffer, size_t size) = 0;
};

} // namespace resource
} // namespace te
