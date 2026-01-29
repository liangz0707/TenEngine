/** ISerializer (contract: 002-object-public-api.md fullversion-001) */

#ifndef TE_OBJECT_SERIALIZER_HPP
#define TE_OBJECT_SERIALIZER_HPP

#include "SerializedBuffer.hpp"
#include "TypeId.hpp"
#include "VersionMigration.hpp"
#include <cstddef>
#include <cstdint>

namespace te::object {

/** Serializer interface: Serialize, Deserialize, GetCurrentVersion, SetVersionMigration (contract). */
class ISerializer {
public:
    virtual ~ISerializer() = default;
    virtual bool Serialize(SerializedBuffer& out, void const* obj, TypeId typeId) = 0;
    virtual bool Deserialize(SerializedBuffer const& buf, void* obj, TypeId typeId) = 0;
    virtual uint32_t GetCurrentVersion() const = 0;
    virtual void SetVersionMigration(IVersionMigration* migration) = 0;
};

}  // namespace te::object

#endif
