/**
 * @file MouseState.cpp
 * @brief Mouse state management implementation.
 */
#include "InputState.h"
#include "te/application/Event.h"
#include "te/core/log.h"

namespace te {
namespace input {

MouseState::MouseState()
  : m_x(0)
  , m_y(0)
  , m_previousX(0)
  , m_previousY(0)
  , m_deltaX(0)
  , m_deltaY(0)
  , m_captured(false)
{}

void MouseState::UpdateFromEvent(te::application::Event const& event) {
  te::core::LockGuard lock(m_mutex);
  
  switch (event.type) {
    case te::application::EventType::MouseMove: {
      m_x = event.mouse.x;
      m_y = event.mouse.y;
      break;
    }
    case te::application::EventType::MouseButtonDown: {
      // Map event to MouseButton (simplified - assumes left button for now)
      // TODO: Extract button from event data
      MouseButton button = MouseButton::Left;
      m_currentButtons[button] = true;
      break;
    }
    case te::application::EventType::MouseButtonUp: {
      // Map event to MouseButton (simplified)
      MouseButton button = MouseButton::Left;
      m_currentButtons[button] = false;
      break;
    }
    default:
      break;
  }
}

void MouseState::EndFrame() {
  te::core::LockGuard lock(m_mutex);
  m_deltaX = m_x - m_previousX;
  m_deltaY = m_y - m_previousY;
  m_previousX = m_x;
  m_previousY = m_y;
  m_previousButtons = m_currentButtons;
}

bool MouseState::GetButton(MouseButton button) const {
  te::core::LockGuard lock(m_mutex);
  auto it = m_currentButtons.find(button);
  return it != m_currentButtons.end() && it->second;
}

bool MouseState::GetButtonDown(MouseButton button) const {
  te::core::LockGuard lock(m_mutex);
  auto currentIt = m_currentButtons.find(button);
  auto previousIt = m_previousButtons.find(button);
  bool current = (currentIt != m_currentButtons.end() && currentIt->second);
  bool previous = (previousIt != m_previousButtons.end() && previousIt->second);
  return current && !previous;
}

bool MouseState::GetButtonUp(MouseButton button) const {
  te::core::LockGuard lock(m_mutex);
  auto currentIt = m_currentButtons.find(button);
  auto previousIt = m_previousButtons.find(button);
  bool current = (currentIt != m_currentButtons.end() && currentIt->second);
  bool previous = (previousIt != m_previousButtons.end() && previousIt->second);
  return !current && previous;
}

void MouseState::GetPosition(int32_t* x, int32_t* y) const {
  te::core::LockGuard lock(m_mutex);
  if (x) *x = m_x;
  if (y) *y = m_y;
}

void MouseState::GetDelta(int32_t* dx, int32_t* dy) const {
  te::core::LockGuard lock(m_mutex);
  if (dx) *dx = m_deltaX;
  if (dy) *dy = m_deltaY;
}

void MouseState::SetCapture(bool capture) {
  te::core::LockGuard lock(m_mutex);
  m_captured = capture;
}

bool MouseState::IsCaptured() const {
  te::core::LockGuard lock(m_mutex);
  return m_captured;
}

}  // namespace input
}  // namespace te
