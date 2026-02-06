/**
 * @file VersionMigration.h
 * @brief Version migration system (contract: specs/_contracts/002-object-public-api.md).
 * Supports migrating serialized data between versions.
 */
#ifndef TE_OBJECT_VERSION_MIGRATION_H
#define TE_OBJECT_VERSION_MIGRATION_H

#include "te/object/Serializer.h"

namespace te {
namespace object {

// IVersionMigration is already declared in Serializer.h
// This file provides additional utilities if needed

} // namespace object
} // namespace te

#endif // TE_OBJECT_VERSION_MIGRATION_H
