/**
 * @file MainLoop.h
 * @brief Main loop related types (contract: specs/_contracts/003-application-ABI.md).
 * Only types declared in the contract are exposed.
 */
#ifndef TE_APPLICATION_MAINLOOP_H
#define TE_APPLICATION_MAINLOOP_H

#include <cstdint>

namespace te {
namespace application {

/**
 * @brief Time step mode enumeration per contract.
 */
enum class TimeStepMode {
  Fixed,     // Fixed time step (suitable for physics simulation)
  Variable,  // Variable time step (suitable for game logic, default)
  Mixed      // Mixed mode (fixed update, variable render)
};

/**
 * @brief Tick callback ID type per contract.
 */
using TickCallbackId = uint64_t;

/**
 * @brief Tick callback function type per contract.
 * @param deltaTime Delta time in seconds
 */
using TickCallback = void (*)(float deltaTime);

}  // namespace application
}  // namespace te

#endif  // TE_APPLICATION_MAINLOOP_H
