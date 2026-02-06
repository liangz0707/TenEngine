/**
 * @file Input.h
 * @brief Input interface integrating keyboard, mouse, gamepad, touch, and input abstraction (contract: specs/_contracts/006-input-ABI.md).
 * Only types and functions declared in the contract are exposed.
 */
#ifndef TE_INPUT_INPUT_H
#define TE_INPUT_INPUT_H

#include "InputTypes.h"
#include "te/application/Event.h"
#include "te/application/Window.h"
#include <cstdint>

namespace te {
namespace application {
// Forward declaration
class IApplication;
class EventQueue;
}  // namespace application

namespace input {

/**
 * @brief Input interface per contract.
 * 
 * Integrates keyboard, mouse, gamepad, touch, and input abstraction functionality.
 * Design principle: Single interface with complete functionality, similar to Application module's IApplication pattern.
 */
class IInput {
 public:
  virtual ~IInput() = default;

  // ========== Event Processing ==========

  /**
   * @brief Process events from Application's EventQueue per contract.
   * Consumes events from the queue and updates internal input state.
   * @param eventQueue Event queue from Application module (non-const because Pop modifies the queue)
   */
  virtual void ProcessEvents(te::application::EventQueue& eventQueue) = 0;

  // ========== Keyboard Input ==========

  /**
   * @brief Get key state per contract (currently pressed this frame).
   * @param code Key code
   * @return true if key is currently pressed, false otherwise
   */
  virtual bool GetKey(KeyCode code) const = 0;

  /**
   * @brief Get key down state per contract (pressed this frame, was not pressed last frame).
   * @param code Key code
   * @return true if key was just pressed this frame, false otherwise
   */
  virtual bool GetKeyDown(KeyCode code) const = 0;

  /**
   * @brief Get key up state per contract (released this frame, was pressed last frame).
   * @param code Key code
   * @return true if key was just released this frame, false otherwise
   */
  virtual bool GetKeyUp(KeyCode code) const = 0;

  // ========== Mouse Input ==========

  /**
   * @brief Get mouse button state per contract (currently pressed this frame).
   * @param button Mouse button
   * @return true if button is currently pressed, false otherwise
   */
  virtual bool GetMouseButton(MouseButton button) const = 0;

  /**
   * @brief Get mouse button down state per contract (pressed this frame).
   * @param button Mouse button
   * @return true if button was just pressed this frame, false otherwise
   */
  virtual bool GetMouseButtonDown(MouseButton button) const = 0;

  /**
   * @brief Get mouse button up state per contract (released this frame).
   * @param button Mouse button
   * @return true if button was just released this frame, false otherwise
   */
  virtual bool GetMouseButtonUp(MouseButton button) const = 0;

  /**
   * @brief Get mouse position per contract.
   * @param x Output X position
   * @param y Output Y position
   */
  virtual void GetMousePosition(int32_t* x, int32_t* y) const = 0;

  /**
   * @brief Get mouse delta per contract (movement since last frame).
   * @param dx Output delta X
   * @param dy Output delta Y
   */
  virtual void GetMouseDelta(int32_t* dx, int32_t* dy) const = 0;

  /**
   * @brief Set mouse capture per contract (capture mouse input even when outside window).
   * @param capture true to capture, false to release
   */
  virtual void SetMouseCapture(bool capture) = 0;

  /**
   * @brief Set focus window per contract (input events will be associated with this window).
   * @param windowId Window ID from Application module
   */
  virtual void SetFocusWindow(te::application::WindowId windowId) = 0;

  // ========== Gamepad Input ==========

  /**
   * @brief Get gamepad count per contract.
   * @return Number of connected gamepads
   */
  virtual uint32_t GetGamepadCount() const = 0;

  /**
   * @brief Get gamepad button state per contract.
   * @param deviceId Gamepad device ID (0-based index)
   * @param button Gamepad button
   * @return true if button is pressed, false otherwise
   */
  virtual bool GetGamepadButton(uint32_t deviceId, GamepadButton button) const = 0;

  /**
   * @brief Get gamepad axis value per contract.
   * @param deviceId Gamepad device ID (0-based index)
   * @param axis Gamepad axis
   * @return Axis value in range [-1.0, 1.0] (or [0.0, 1.0] for triggers)
   */
  virtual float GetGamepadAxis(uint32_t deviceId, GamepadAxis axis) const = 0;

  /**
   * @brief Set gamepad vibration per contract (optional).
   * @param deviceId Gamepad device ID (0-based index)
   * @param leftMotor Left motor strength [0.0, 1.0]
   * @param rightMotor Right motor strength [0.0, 1.0]
   */
  virtual void SetGamepadVibration(uint32_t deviceId, float leftMotor, float rightMotor) = 0;

  // ========== Touch Input ==========

  /**
   * @brief Get touch count per contract.
   * @return Number of active touch points
   */
  virtual uint32_t GetTouchCount() const = 0;

  /**
   * @brief Get touch state per contract.
   * @param index Touch point index (0-based)
   * @param out Output touch state
   */
  virtual void GetTouch(uint32_t index, TouchState* out) const = 0;

  // ========== Input Abstraction (Action/Axis) ==========

  /**
   * @brief Get action state per contract.
   * @param actionId Action ID
   * @return true if action is active, false otherwise
   */
  virtual bool GetActionState(ActionId actionId) const = 0;

  /**
   * @brief Get axis value per contract.
   * @param axisId Axis ID
   * @return Axis value (mapped from bound input devices)
   */
  virtual float GetAxisValue(AxisId axisId) const = 0;

  /**
   * @brief Register action per contract.
   * @param actionId Action ID
   * @param name Action name (for debugging/config)
   */
  virtual void RegisterAction(ActionId actionId, char const* name) = 0;

  /**
   * @brief Register axis per contract.
   * @param axisId Axis ID
   * @param name Axis name (for debugging/config)
   */
  virtual void RegisterAxis(AxisId axisId, char const* name) = 0;

  /**
   * @brief Bind action to key per contract.
   * @param actionId Action ID
   * @param key Key code to bind
   */
  virtual void BindActionToKey(ActionId actionId, KeyCode key) = 0;

  /**
   * @brief Bind axis to key per contract.
   * @param axisId Axis ID
   * @param key Key code to bind
   * @param scale Scale factor (typically 1.0 or -1.0 for positive/negative)
   */
  virtual void BindAxisToKey(AxisId axisId, KeyCode key, float scale) = 0;

  /**
   * @brief Bind axis to gamepad axis per contract.
   * @param axisId Axis ID
   * @param deviceId Gamepad device ID
   * @param axis Gamepad axis to bind
   * @param scale Scale factor
   */
  virtual void BindAxisToGamepadAxis(AxisId axisId, uint32_t deviceId, GamepadAxis axis, float scale) = 0;

  /**
   * @brief Load binding configuration per contract.
   * @param configPath Path to configuration file
   * @return true on success, false on failure
   */
  virtual bool LoadBindingConfig(char const* configPath) = 0;
};

/**
 * @brief Create input instance per contract.
 * @return IInput pointer, or nullptr on failure. Caller responsible for deletion or engine manages.
 */
IInput* CreateInput();

/**
 * @brief Create input instance with Application reference per contract.
 * @param application Application instance (optional, for automatic event processing)
 * @return IInput pointer, or nullptr on failure. Caller responsible for deletion or engine manages.
 */
IInput* CreateInput(te::application::IApplication* application);

}  // namespace input
}  // namespace te

#endif  // TE_INPUT_INPUT_H
