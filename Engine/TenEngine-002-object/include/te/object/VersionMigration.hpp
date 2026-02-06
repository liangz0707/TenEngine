/** @file VersionMigration.hpp
 *  ABI: specs/_contracts/002-object-ABI.md
 */
#ifndef TE_OBJECT_VERSION_MIGRATION_HPP
#define TE_OBJECT_VERSION_MIGRATION_HPP

#include <cstdint>
#include <te/object/SerializedBuffer.hpp>

namespace te {
namespace object {

struct IVersionMigration {
  virtual bool Migrate(SerializedBuffer& buf, uint32_t fromVersion, uint32_t toVersion) = 0;
  virtual ~IVersionMigration() = default;
};

}  // namespace object
}  // namespace te

#endif  // TE_OBJECT_VERSION_MIGRATION_HPP
