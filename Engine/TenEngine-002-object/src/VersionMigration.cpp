/**
 * VersionMigration: optional no-op default; real impl per type.
 * ABI: specs/_contracts/002-object-ABI.md
 */

#include <te/object/VersionMigration.hpp>

namespace te {
namespace object {

// No default implementation; callers provide IVersionMigration impl.
// This file exists so the target has a translation unit if needed.

}  // namespace object
}  // namespace te
