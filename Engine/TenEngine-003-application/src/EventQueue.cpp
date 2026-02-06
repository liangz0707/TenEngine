/**
 * @file EventQueue.cpp
 * @brief Event queue implementation (thread-safe).
 */
#include "te/application/Event.h"
#include "te/core/thread.h"
#include <cstddef>

namespace te {
namespace application {

bool EventQueue::Pop(Event& event) {
  te::core::LockGuard lock(m_mutex);
  if (m_queue.empty()) {
    return false;
  }
  event = m_queue.front();
  m_queue.erase(m_queue.begin());
  return true;
}

void EventQueue::Push(Event const& event) {
  te::core::LockGuard lock(m_mutex);
  m_queue.push_back(event);
}

bool EventQueue::Empty() const {
  te::core::LockGuard lock(m_mutex);
  return m_queue.empty();
}

std::size_t EventQueue::Size() const {
  te::core::LockGuard lock(m_mutex);
  return m_queue.size();
}

void EventQueue::Clear() {
  te::core::LockGuard lock(m_mutex);
  m_queue.clear();
}

}  // namespace application
}  // namespace te
