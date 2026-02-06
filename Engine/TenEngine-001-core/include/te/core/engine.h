/**
 * @file engine.h
 * @brief Process-level Init/Shutdown, InitParams (contract: specs/_contracts/001-core-ABI.md).
 * Only types and functions declared in the contract are exposed.
 */
#ifndef TE_CORE_ENGINE_H
#define TE_CORE_ENGINE_H

namespace te {
namespace core {

/** Init parameters; optional log_path, allocator_policy per ABI. */
struct InitParams {
  char const* log_path = nullptr;
  char const* allocator_policy = nullptr;
};

/** Process-level init. Returns false on failure; idempotent on success. */
bool Init(InitParams const* params);

/** Process-level shutdown. Call once after Init, before process exit. */
void Shutdown();

}  // namespace core
}  // namespace te

#endif  // TE_CORE_ENGINE_H
