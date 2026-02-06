/**
 * @file KeyboardState.cpp
 * @brief Keyboard state management implementation.
 */
#include "InputState.h"
#include "te/application/Event.h"
#include "te/core/log.h"

namespace te {
namespace input {

KeyboardState::KeyboardState() = default;

void KeyboardState::UpdateFromEvent(te::application::Event const& event) {
  te::core::LockGuard lock(m_mutex);
  
  switch (event.type) {
    case te::application::EventType::KeyDown: {
      KeyCode keyCode = static_cast<KeyCode>(event.key.keyCode);
      m_currentKeys[keyCode] = true;
      break;
    }
    case te::application::EventType::KeyUp: {
      KeyCode keyCode = static_cast<KeyCode>(event.key.keyCode);
      m_currentKeys[keyCode] = false;
      break;
    }
    default:
      break;
  }
}

void KeyboardState::EndFrame() {
  te::core::LockGuard lock(m_mutex);
  m_previousKeys = m_currentKeys;
}

bool KeyboardState::GetKey(KeyCode code) const {
  te::core::LockGuard lock(m_mutex);
  auto it = m_currentKeys.find(code);
  return it != m_currentKeys.end() && it->second;
}

bool KeyboardState::GetKeyDown(KeyCode code) const {
  te::core::LockGuard lock(m_mutex);
  auto currentIt = m_currentKeys.find(code);
  auto previousIt = m_previousKeys.find(code);
  bool current = (currentIt != m_currentKeys.end() && currentIt->second);
  bool previous = (previousIt != m_previousKeys.end() && previousIt->second);
  return current && !previous;
}

bool KeyboardState::GetKeyUp(KeyCode code) const {
  te::core::LockGuard lock(m_mutex);
  auto currentIt = m_currentKeys.find(code);
  auto previousIt = m_previousKeys.find(code);
  bool current = (currentIt != m_currentKeys.end() && currentIt->second);
  bool previous = (previousIt != m_previousKeys.end() && previousIt->second);
  return !current && previous;
}

}  // namespace input
}  // namespace te
