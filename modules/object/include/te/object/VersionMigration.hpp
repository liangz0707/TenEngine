/** IVersionMigration (contract: 002-object-public-api.md fullversion-001) */

#ifndef TE_OBJECT_VERSION_MIGRATION_HPP
#define TE_OBJECT_VERSION_MIGRATION_HPP

#include "SerializedBuffer.hpp"
#include <cstdint>

namespace te::object {

/** Migrate buffer from old format to current; required by contract. */
class IVersionMigration {
public:
    virtual ~IVersionMigration() = default;
    virtual bool Migrate(SerializedBuffer& buf, uint32_t fromVersion, uint32_t toVersion) = 0;
};

}  // namespace te::object

#endif
