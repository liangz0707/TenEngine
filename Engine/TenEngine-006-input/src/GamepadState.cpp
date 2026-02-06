/**
 * @file GamepadState.cpp
 * @brief Gamepad state management implementation.
 */
#include "InputState.h"
#include "te/core/log.h"

namespace te {
namespace input {

GamepadState::GamepadState() {
  // Initialize with max 4 gamepads
  m_gamepads.resize(4);
}

void GamepadState::UpdateFromDevice() {
  te::core::LockGuard lock(m_mutex);
  // TODO: Platform-specific gamepad polling
  // For now, placeholder - will be implemented with platform-specific code
}

void GamepadState::EndFrame() {
  te::core::LockGuard lock(m_mutex);
  for (auto& gamepad : m_gamepads) {
    gamepad.previousButtons = gamepad.currentButtons;
  }
}

uint32_t GamepadState::GetGamepadCount() const {
  te::core::LockGuard lock(m_mutex);
  uint32_t count = 0;
  for (auto const& gamepad : m_gamepads) {
    if (gamepad.connected) {
      count++;
    }
  }
  return count;
}

bool GamepadState::GetButton(uint32_t deviceId, GamepadButton button) const {
  te::core::LockGuard lock(m_mutex);
  if (deviceId >= m_gamepads.size()) {
    return false;
  }
  auto const& gamepad = m_gamepads[deviceId];
  if (!gamepad.connected) {
    return false;
  }
  auto it = gamepad.currentButtons.find(button);
  return it != gamepad.currentButtons.end() && it->second;
}

float GamepadState::GetAxis(uint32_t deviceId, GamepadAxis axis) const {
  te::core::LockGuard lock(m_mutex);
  if (deviceId >= m_gamepads.size()) {
    return 0.0f;
  }
  auto const& gamepad = m_gamepads[deviceId];
  if (!gamepad.connected) {
    return 0.0f;
  }
  auto it = gamepad.axes.find(axis);
  return it != gamepad.axes.end() ? it->second : 0.0f;
}

void GamepadState::SetVibration(uint32_t deviceId, float leftMotor, float rightMotor) {
  te::core::LockGuard lock(m_mutex);
  if (deviceId >= m_gamepads.size()) {
    return;
  }
  auto& gamepad = m_gamepads[deviceId];
  if (!gamepad.connected) {
    return;
  }
  // TODO: Platform-specific vibration implementation
  (void)leftMotor;
  (void)rightMotor;
}

}  // namespace input
}  // namespace te
