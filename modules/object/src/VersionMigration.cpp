/** Default/no-op version migration (contract: 002-object-public-api.md) */

#include "te/object/VersionMigration.hpp"

namespace te::object {

/** No-op implementation: same version passes; otherwise returns false. */
class NoOpVersionMigration : public IVersionMigration {
public:
    bool Migrate(SerializedBuffer& buf, uint32_t fromVersion, uint32_t toVersion) override {
        (void)buf;
        return fromVersion == toVersion;
    }
};

}  // namespace te::object
