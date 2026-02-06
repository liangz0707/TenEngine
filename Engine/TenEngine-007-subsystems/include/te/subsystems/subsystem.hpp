/**
 * @file subsystem.hpp
 * @brief ISubsystem interface (contract: 007-subsystems-public-api.md).
 * Only contract-declared types and API.
 */
#ifndef TE_SUBSYSTEMS_SUBSYSTEM_HPP
#define TE_SUBSYSTEMS_SUBSYSTEM_HPP

namespace te {
namespace subsystems {

/**
 * Subsystem instance interface; lifecycle hooks called by main loop only.
 * Per-type singleton; GetSubsystem<T> returns the unique instance.
 */
class ISubsystem {
public:
    virtual ~ISubsystem() = default;
    virtual void Initialize() = 0;
    virtual void Start() = 0;
    virtual void Stop() = 0;
    virtual void Shutdown() = 0;
};

}  // namespace subsystems
}  // namespace te

#endif  // TE_SUBSYSTEMS_SUBSYSTEM_HPP
