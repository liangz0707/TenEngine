/**
 * @file TouchState.cpp
 * @brief Touch state management implementation.
 */
#include "InputState.h"
#include "te/application/Event.h"
#include "te/core/log.h"
#include <algorithm>

namespace te {
namespace input {

TouchStateManager::TouchStateManager() = default;

void TouchStateManager::UpdateFromEvent(te::application::Event const& event) {
  te::core::LockGuard lock(m_mutex);
  
  switch (event.type) {
    case te::application::EventType::TouchDown: {
      TouchState touch;
      touch.touchId = event.touch.touchId;
      touch.x = event.touch.x;
      touch.y = event.touch.y;
      touch.phase = TouchPhase::Begin;
      
      // Find existing touch or add new
      bool found = false;
      for (auto& t : m_touches) {
        if (t.touchId == touch.touchId && t.phase == TouchPhase::End) {
          t = touch;
          found = true;
          break;
        }
      }
      if (!found) {
        m_touches.push_back(touch);
      }
      break;
    }
    case te::application::EventType::TouchMove: {
      for (auto& touch : m_touches) {
        if (touch.touchId == event.touch.touchId) {
          touch.x = event.touch.x;
          touch.y = event.touch.y;
          touch.phase = TouchPhase::Move;
          break;
        }
      }
      break;
    }
    case te::application::EventType::TouchUp: {
      for (auto& touch : m_touches) {
        if (touch.touchId == event.touch.touchId) {
          touch.x = event.touch.x;
          touch.y = event.touch.y;
          touch.phase = TouchPhase::End;
          break;
        }
      }
      break;
    }
    default:
      break;
  }
}

void TouchStateManager::EndFrame() {
  te::core::LockGuard lock(m_mutex);
  // Remove ended touches after a frame delay
  m_touches.erase(
    std::remove_if(m_touches.begin(), m_touches.end(),
                   [](TouchState const& touch) { return touch.phase == TouchPhase::End; }),
    m_touches.end()
  );
}

uint32_t TouchStateManager::GetTouchCount() const {
  te::core::LockGuard lock(m_mutex);
  uint32_t count = 0;
  for (auto const& touch : m_touches) {
    if (touch.phase != TouchPhase::End) {
      count++;
    }
  }
  return count;
}

void TouchStateManager::GetTouch(uint32_t index, TouchState* out) const {
  te::core::LockGuard lock(m_mutex);
  if (!out) {
    return;
  }
  
  uint32_t activeIndex = 0;
  for (auto const& touch : m_touches) {
    if (touch.phase != TouchPhase::End) {
      if (activeIndex == index) {
        *out = touch;
        return;
      }
      activeIndex++;
    }
  }
  
  // Not found, return default
  *out = TouchState{};
}

}  // namespace input
}  // namespace te
