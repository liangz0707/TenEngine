/**
 * @file InputBinding.h
 * @brief Input binding management (internal).
 */
#ifndef TE_INPUT_INPUT_BINDING_H
#define TE_INPUT_INPUT_BINDING_H

#include "te/input/InputTypes.h"
#include "te/core/containers.h"
#include "te/core/thread.h"
#include <cstdint>

namespace te {
namespace input {

// Forward declarations
class KeyboardState;
class GamepadState;

/**
 * @brief Action binding entry.
 */
struct ActionBinding {
  KeyCode key = KeyCode::Count;  // Invalid key
};

/**
 * @brief Axis binding entry.
 */
struct AxisBinding {
  KeyCode key = KeyCode::Count;  // Invalid key
  float scale = 1.0f;
  
  uint32_t gamepadDeviceId = UINT32_MAX;  // Invalid device
  GamepadAxis gamepadAxis = GamepadAxis::Count;  // Invalid axis
  float gamepadScale = 1.0f;
};

/**
 * @brief Input binding manager.
 */
class InputBinding {
 public:
  InputBinding();
  
  void RegisterAction(ActionId actionId, char const* name);
  void RegisterAxis(AxisId axisId, char const* name);
  void BindActionToKey(ActionId actionId, KeyCode key);
  void BindAxisToKey(AxisId axisId, KeyCode key, float scale);
  void BindAxisToGamepadAxis(AxisId axisId, uint32_t deviceId, GamepadAxis axis, float scale);
  bool LoadBindingConfig(char const* configPath);
  
  bool GetActionState(ActionId actionId, KeyboardState const& keyboardState) const;
  float GetAxisValue(AxisId axisId, KeyboardState const& keyboardState, GamepadState const& gamepadState) const;
  
 private:
  te::core::Map<ActionId, ActionBinding> m_actionBindings;
  te::core::Map<AxisId, AxisBinding> m_axisBindings;
  te::core::Map<ActionId, te::core::String> m_actionNames;
  te::core::Map<AxisId, te::core::String> m_axisNames;
  mutable te::core::Mutex m_mutex;
};

}  // namespace input
}  // namespace te

#endif  // TE_INPUT_INPUT_BINDING_H
