/**
 * @file Event.h
 * @brief Event-related types and structures (contract: specs/_contracts/003-application-ABI.md).
 */
#ifndef TE_APPLICATION_EVENT_H
#define TE_APPLICATION_EVENT_H

#include <cstddef>
#include <cstdint>
#include "Window.h"
#include "te/core/containers.h"
#include "te/core/thread.h"

namespace te {
namespace application {

/**
 * @brief Event type enumeration.
 */
enum class EventType {
    // Window events
    WindowCreated,
    WindowDestroyed,
    WindowResized,
    WindowMoved,
    WindowFocused,
    WindowClosed,

    // Input events (decoupled from Input module)
    KeyDown,
    KeyUp,
    MouseMove,
    MouseButtonDown,
    MouseButtonUp,
    MouseWheel,
    TouchDown,
    TouchUp,
    TouchMove,

    // System events
    AppPaused,
    AppResumed,
    AppWillTerminate
};

/**
 * @brief Event structure.
 */
struct Event {
    EventType type;
    float timestamp;
    WindowId windowId;
    union {
        struct {
            uint32_t keyCode;
        } key;
        struct {
            int32_t x;
            int32_t y;
        } mouse;
        struct {
            uint32_t touchId;
            float x;
            float y;
        } touch;
        // Other event data...
    };
};

/**
 * @brief Thread-safe event queue for Input module to consume directly per contract.
 * 
 * This queue is thread-safe and can be accessed from multiple threads.
 * Events are pushed by the application's event pump and consumed by the Input module.
 */
class EventQueue {
 public:
  /**
   * @brief Pop event from queue (non-blocking).
   * @param event Output event
   * @return true if event popped, false if queue empty
   */
  bool Pop(Event& event);

  /**
   * @brief Push event to queue (thread-safe).
   * @param event Event to push
   */
  void Push(Event const& event);

  /**
   * @brief Check if queue is empty.
   * @return true if empty, false otherwise
   */
  bool Empty() const;

  /**
   * @brief Get queue size.
   * @return Number of events in queue
   */
  std::size_t Size() const;

  /**
   * @brief Clear all events from queue.
   */
  void Clear();

 private:
  mutable te::core::Mutex m_mutex;  // Mutex for thread safety
  te::core::Array<Event> m_queue;    // Event queue storage
};

}  // namespace application
}  // namespace te

#endif  // TE_APPLICATION_EVENT_H
