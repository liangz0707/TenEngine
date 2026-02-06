/**
 * @file Input.cpp
 * @brief Input implementation.
 */
#include "te/input/Input.h"
#include "InputState.h"
#include "InputBinding.h"
#include "te/application/Application.h"
#include "te/application/MainLoop.h"
#include "te/core/log.h"
#include "te/core/check.h"
#include <memory>
#include <algorithm>

namespace te {
namespace input {

/**
 * @brief Input implementation class.
 */
class Input : public IInput {
 public:
  Input() = default;
  
  explicit Input(te::application::IApplication* application)
    : m_application(application)
  {
    if (m_application) {
      // Optionally register tick callback for automatic event processing
      // For now, we'll use manual ProcessEvents call
    }
  }
  
  ~Input() override {
    if (m_application && m_tickCallbackId != 0) {
      m_application->UnregisterTickCallback(m_tickCallbackId);
    }
  }

  // ========== Event Processing ==========

  void ProcessEvents(te::application::EventQueue& eventQueue) override {
    te::application::Event event;
    while (eventQueue.Pop(event)) {
      // Update keyboard state
      m_keyboardState.UpdateFromEvent(event);
      
      // Update mouse state
      m_mouseState.UpdateFromEvent(event);
      
      // Update touch state
      m_touchState.UpdateFromEvent(event);
    }
    
    // Update gamepad state (poll devices)
    m_gamepadState.UpdateFromDevice();
    
    // End frame - update previous states
    m_keyboardState.EndFrame();
    m_mouseState.EndFrame();
    m_gamepadState.EndFrame();
    m_touchState.EndFrame();
  }

  // ========== Keyboard Input ==========

  bool GetKey(KeyCode code) const override {
    return m_keyboardState.GetKey(code);
  }

  bool GetKeyDown(KeyCode code) const override {
    return m_keyboardState.GetKeyDown(code);
  }

  bool GetKeyUp(KeyCode code) const override {
    return m_keyboardState.GetKeyUp(code);
  }

  // ========== Mouse Input ==========

  bool GetMouseButton(MouseButton button) const override {
    return m_mouseState.GetButton(button);
  }

  bool GetMouseButtonDown(MouseButton button) const override {
    return m_mouseState.GetButtonDown(button);
  }

  bool GetMouseButtonUp(MouseButton button) const override {
    return m_mouseState.GetButtonUp(button);
  }

  void GetMousePosition(int32_t* x, int32_t* y) const override {
    m_mouseState.GetPosition(x, y);
  }

  void GetMouseDelta(int32_t* dx, int32_t* dy) const override {
    m_mouseState.GetDelta(dx, dy);
  }

  void SetMouseCapture(bool capture) override {
    m_mouseState.SetCapture(capture);
    // TODO: Forward to Application window if needed
  }

  void SetFocusWindow(te::application::WindowId windowId) override {
    m_focusWindowId = windowId;
    // TODO: Forward to Application if needed
  }

  // ========== Gamepad Input ==========

  uint32_t GetGamepadCount() const override {
    return m_gamepadState.GetGamepadCount();
  }

  bool GetGamepadButton(uint32_t deviceId, GamepadButton button) const override {
    return m_gamepadState.GetButton(deviceId, button);
  }

  float GetGamepadAxis(uint32_t deviceId, GamepadAxis axis) const override {
    return m_gamepadState.GetAxis(deviceId, axis);
  }

  void SetGamepadVibration(uint32_t deviceId, float leftMotor, float rightMotor) override {
    m_gamepadState.SetVibration(deviceId, leftMotor, rightMotor);
  }

  // ========== Touch Input ==========

  uint32_t GetTouchCount() const override {
    return m_touchState.GetTouchCount();
  }

  void GetTouch(uint32_t index, TouchState* out) const override {
    m_touchState.GetTouch(index, out);
  }

  // ========== Input Abstraction (Action/Axis) ==========

  bool GetActionState(ActionId actionId) const override {
    return m_binding.GetActionState(actionId, m_keyboardState);
  }

  float GetAxisValue(AxisId axisId) const override {
    return m_binding.GetAxisValue(axisId, m_keyboardState, m_gamepadState);
  }

  void RegisterAction(ActionId actionId, char const* name) override {
    m_binding.RegisterAction(actionId, name);
  }

  void RegisterAxis(AxisId axisId, char const* name) override {
    m_binding.RegisterAxis(axisId, name);
  }

  void BindActionToKey(ActionId actionId, KeyCode key) override {
    m_binding.BindActionToKey(actionId, key);
  }

  void BindAxisToKey(AxisId axisId, KeyCode key, float scale) override {
    m_binding.BindAxisToKey(axisId, key, scale);
  }

  void BindAxisToGamepadAxis(AxisId axisId, uint32_t deviceId, GamepadAxis axis, float scale) override {
    m_binding.BindAxisToGamepadAxis(axisId, deviceId, axis, scale);
  }

  bool LoadBindingConfig(char const* configPath) override {
    return m_binding.LoadBindingConfig(configPath);
  }

 private:
  KeyboardState m_keyboardState;
  MouseState m_mouseState;
  GamepadState m_gamepadState;
  TouchStateManager m_touchState;
  InputBinding m_binding;
  
  te::application::IApplication* m_application = nullptr;
  te::application::TickCallbackId m_tickCallbackId = 0;
  te::application::WindowId m_focusWindowId = te::application::InvalidWindowId;
};

IInput* CreateInput() {
  try {
    return new Input();
  } catch (...) {
    te::core::Log(te::core::LogLevel::Error, "Exception creating input");
    return nullptr;
  }
}

IInput* CreateInput(te::application::IApplication* application) {
  try {
    return new Input(application);
  } catch (...) {
    te::core::Log(te::core::LogLevel::Error, "Exception creating input with application");
    return nullptr;
  }
}

}  // namespace input
}  // namespace te
