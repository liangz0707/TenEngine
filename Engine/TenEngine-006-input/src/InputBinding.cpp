/**
 * @file InputBinding.cpp
 * @brief Input binding management implementation.
 */
#include "InputBinding.h"
#include "InputState.h"
#include "te/core/log.h"
#include "te/core/platform.h"

namespace te {
namespace input {

InputBinding::InputBinding() = default;

void InputBinding::RegisterAction(ActionId actionId, char const* name) {
  te::core::LockGuard lock(m_mutex);
  m_actionBindings[actionId] = ActionBinding{};
  if (name) {
    m_actionNames[actionId] = name;
  }
}

void InputBinding::RegisterAxis(AxisId axisId, char const* name) {
  te::core::LockGuard lock(m_mutex);
  m_axisBindings[axisId] = AxisBinding{};
  if (name) {
    m_axisNames[axisId] = name;
  }
}

void InputBinding::BindActionToKey(ActionId actionId, KeyCode key) {
  te::core::LockGuard lock(m_mutex);
  auto it = m_actionBindings.find(actionId);
  if (it != m_actionBindings.end()) {
    it->second.key = key;
  }
}

void InputBinding::BindAxisToKey(AxisId axisId, KeyCode key, float scale) {
  te::core::LockGuard lock(m_mutex);
  auto it = m_axisBindings.find(axisId);
  if (it != m_axisBindings.end()) {
    it->second.key = key;
    it->second.scale = scale;
  }
}

void InputBinding::BindAxisToGamepadAxis(AxisId axisId, uint32_t deviceId, GamepadAxis axis, float scale) {
  te::core::LockGuard lock(m_mutex);
  auto it = m_axisBindings.find(axisId);
  if (it != m_axisBindings.end()) {
    it->second.gamepadDeviceId = deviceId;
    it->second.gamepadAxis = axis;
    it->second.gamepadScale = scale;
  }
}

bool InputBinding::LoadBindingConfig(char const* configPath) {
  if (!configPath) {
    te::core::Log(te::core::LogLevel::Error, "Invalid config path");
    return false;
  }
  
  // TODO: Implement config file loading
  // For now, placeholder
  te::core::Log(te::core::LogLevel::Warn, "LoadBindingConfig not yet implemented");
  return false;
}

bool InputBinding::GetActionState(ActionId actionId, KeyboardState const& keyboardState) const {
  te::core::LockGuard lock(m_mutex);
  auto it = m_actionBindings.find(actionId);
  if (it == m_actionBindings.end()) {
    return false;
  }
  
  ActionBinding const& binding = it->second;
  if (binding.key != KeyCode::Count) {
    return keyboardState.GetKey(binding.key);
  }
  
  return false;
}

float InputBinding::GetAxisValue(AxisId axisId, KeyboardState const& keyboardState, GamepadState const& gamepadState) const {
  te::core::LockGuard lock(m_mutex);
  auto it = m_axisBindings.find(axisId);
  if (it == m_axisBindings.end()) {
    return 0.0f;
  }
  
  AxisBinding const& binding = it->second;
  float value = 0.0f;
  
  // Check keyboard binding
  if (binding.key != KeyCode::Count) {
    if (keyboardState.GetKey(binding.key)) {
      value += binding.scale;
    }
  }
  
  // Check gamepad binding
  if (binding.gamepadDeviceId != UINT32_MAX && binding.gamepadAxis != GamepadAxis::Count) {
    float gamepadValue = gamepadState.GetAxis(binding.gamepadDeviceId, binding.gamepadAxis);
    value += gamepadValue * binding.gamepadScale;
  }
  
  return value;
}

}  // namespace input
}  // namespace te
