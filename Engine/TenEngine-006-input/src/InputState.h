/**
 * @file InputState.h
 * @brief Input state management classes (internal).
 */
#ifndef TE_INPUT_INPUT_STATE_H
#define TE_INPUT_INPUT_STATE_H

#include "te/input/InputTypes.h"
#include "te/core/containers.h"
#include "te/core/thread.h"
#include <cstdint>

namespace te {
namespace application {
struct Event;
}  // namespace application

namespace input {

/**
 * @brief Keyboard state management.
 */
class KeyboardState {
 public:
  KeyboardState();
  
  void UpdateFromEvent(te::application::Event const& event);
  void EndFrame();  // Copy current state to previous state
  
  bool GetKey(KeyCode code) const;
  bool GetKeyDown(KeyCode code) const;
  bool GetKeyUp(KeyCode code) const;
  
 private:
  te::core::Map<KeyCode, bool> m_currentKeys;
  te::core::Map<KeyCode, bool> m_previousKeys;
  mutable te::core::Mutex m_mutex;
};

/**
 * @brief Mouse state management.
 */
class MouseState {
 public:
  MouseState();
  
  void UpdateFromEvent(te::application::Event const& event);
  void EndFrame();  // Update delta and copy position to previous
  
  bool GetButton(MouseButton button) const;
  bool GetButtonDown(MouseButton button) const;
  bool GetButtonUp(MouseButton button) const;
  void GetPosition(int32_t* x, int32_t* y) const;
  void GetDelta(int32_t* dx, int32_t* dy) const;
  
  void SetCapture(bool capture);
  bool IsCaptured() const;
  
 private:
  int32_t m_x;
  int32_t m_y;
  int32_t m_previousX;
  int32_t m_previousY;
  int32_t m_deltaX;
  int32_t m_deltaY;
  te::core::Map<MouseButton, bool> m_currentButtons;
  te::core::Map<MouseButton, bool> m_previousButtons;
  bool m_captured;
  mutable te::core::Mutex m_mutex;
};

/**
 * @brief Gamepad state management.
 */
class GamepadState {
 public:
  GamepadState();
  
  void UpdateFromDevice();  // Poll gamepad devices (platform-specific)
  void EndFrame();
  
  uint32_t GetGamepadCount() const;
  bool GetButton(uint32_t deviceId, GamepadButton button) const;
  float GetAxis(uint32_t deviceId, GamepadAxis axis) const;
  
  void SetVibration(uint32_t deviceId, float leftMotor, float rightMotor);
  
 private:
  struct GamepadDevice {
    bool connected = false;
    te::core::Map<GamepadButton, bool> currentButtons;
    te::core::Map<GamepadButton, bool> previousButtons;
    te::core::Map<GamepadAxis, float> axes;
  };
  
  te::core::Array<GamepadDevice> m_gamepads;
  mutable te::core::Mutex m_mutex;
};

/**
 * @brief Touch state management.
 */
class TouchStateManager {
 public:
  TouchStateManager();
  
  void UpdateFromEvent(te::application::Event const& event);
  void EndFrame();  // Update touch phases
  
  uint32_t GetTouchCount() const;
  void GetTouch(uint32_t index, TouchState* out) const;
  
 private:
  te::core::Array<TouchState> m_touches;
  mutable te::core::Mutex m_mutex;
};

}  // namespace input
}  // namespace te

#endif  // TE_INPUT_INPUT_STATE_H
