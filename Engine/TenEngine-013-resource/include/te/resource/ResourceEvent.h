/**
 * @file ResourceEvent.h
 * @brief Resource event system for notifications and callbacks.
 * 
 * Provides functionality for:
 * - Resource state change events
 * - Load/unload notifications
 * - Hot reload events
 * - Error notifications
 * - Event subscription and broadcasting
 */
#ifndef TE_RESOURCE_RESOURCE_EVENT_H
#define TE_RESOURCE_RESOURCE_EVENT_H

#include <te/resource/ResourceId.h>
#include <te/resource/ResourceTypes.h>
#include <string>
#include <vector>
#include <functional>
#include <mutex>
#include <memory>

namespace te {
namespace resource {

class IResource;

/**
 * Event subscription handle.
 */
using EventSubscriptionHandle = void*;

/**
 * Invalid subscription handle.
 */
constexpr EventSubscriptionHandle InvalidEventSubscription = nullptr;

/**
 * Resource event data structure.
 * Contains all information about a resource event.
 */
struct ResourceEventData {
  ResourceId resourceId;          // Resource that triggered the event
  ResourceStateEvent eventType;   // Type of event
  LoadResult loadResult;          // Load result (for load events)
  std::string message;            // Human-readable message
  std::string path;               // Resource path
  ResourceType resourceType;      // Resource type
  void* userData;                 // User data from subscription
  std::size_t memoryFreed;        // Memory freed (for unload events)
  float progress;                 // Progress (for loading events)
  
  ResourceEventData()
    : resourceId()
    , eventType(ResourceStateEvent::Created)
    , loadResult(LoadResult::Ok)
    , userData(nullptr)
    , memoryFreed(0)
    , progress(0.0f) {}
};

/**
 * Resource event callback type.
 */
using ResourceEventCallback = void (*)(ResourceEventData const& event);

/**
 * Resource event filter function.
 * Returns true if the event should be delivered.
 */
using ResourceEventFilter = std::function<bool(ResourceEventData const&)>;

/**
 * Resource event subscription info.
 */
struct EventSubscription {
  EventSubscriptionHandle handle;
  ResourceEventCallback callback;
  ResourceEventFilter filter;
  void* userData;
  bool isGlobal;                  // Subscribe to all resources
  ResourceId specificResource;    // Subscribe to specific resource (if not global)
  bool isActive;
  
  EventSubscription()
    : handle(nullptr)
    , callback(nullptr)
    , userData(nullptr)
    , isGlobal(false)
    , isActive(true) {}
};

/**
 * Resource event manager interface.
 * Manages event subscriptions and broadcasting.
 */
class IResourceEventManager {
 public:
  virtual ~IResourceEventManager() = default;
  
  //==========================================================================
  // Subscription Management
  //==========================================================================
  
  /**
   * Subscribe to all resource events globally.
   * @param callback Event callback
   * @param userData User data passed to callback
   * @return Subscription handle
   */
  virtual EventSubscriptionHandle SubscribeGlobal(
      ResourceEventCallback callback,
      void* userData = nullptr) = 0;
  
  /**
   * Subscribe to events for a specific resource.
   * @param resourceId Resource ID to monitor
   * @param callback Event callback
   * @param userData User data passed to callback
   * @return Subscription handle
   */
  virtual EventSubscriptionHandle SubscribeResource(
      ResourceId resourceId,
      ResourceEventCallback callback,
      void* userData = nullptr) = 0;
  
  /**
   * Subscribe to specific event types globally.
   * @param eventTypes Event types to subscribe to
   * @param callback Event callback
   * @param userData User data passed to callback
   * @return Subscription handle
   */
  virtual EventSubscriptionHandle SubscribeEventTypes(
      std::vector<ResourceStateEvent> const& eventTypes,
      ResourceEventCallback callback,
      void* userData = nullptr) = 0;
  
  /**
   * Subscribe with a custom filter.
   * @param filter Filter function (return true to receive event)
   * @param callback Event callback
   * @param userData User data passed to callback
   * @return Subscription handle
   */
  virtual EventSubscriptionHandle SubscribeWithFilter(
      ResourceEventFilter filter,
      ResourceEventCallback callback,
      void* userData = nullptr) = 0;
  
  /**
   * Unsubscribe from events.
   * @param handle Subscription handle from Subscribe*
   */
  virtual void Unsubscribe(EventSubscriptionHandle handle) = 0;
  
  /**
   * Pause a subscription (temporarily stop receiving events).
   * @param handle Subscription handle
   */
  virtual void PauseSubscription(EventSubscriptionHandle handle) = 0;
  
  /**
   * Resume a paused subscription.
   * @param handle Subscription handle
   */
  virtual void ResumeSubscription(EventSubscriptionHandle handle) = 0;
  
  //==========================================================================
  // Event Broadcasting
  //==========================================================================
  
  /**
   * Broadcast a resource event.
   * Called by ResourceManager when events occur.
   * @param event Event data
   */
  virtual void BroadcastEvent(ResourceEventData const& event) = 0;
  
  /**
   * Broadcast a simple event.
   * @param resourceId Resource ID
   * @param eventType Event type
   * @param message Optional message
   */
  virtual void BroadcastSimpleEvent(ResourceId resourceId,
                                    ResourceStateEvent eventType,
                                    std::string const& message = "") = 0;
  
  /**
   * Broadcast a load event.
   * @param resourceId Resource ID
   * @param result Load result
   * @param path Resource path
   * @param resourceType Resource type
   */
  virtual void BroadcastLoadEvent(ResourceId resourceId,
                                  LoadResult result,
                                  std::string const& path,
                                  ResourceType resourceType) = 0;
  
  /**
   * Broadcast an unload event.
   * @param resourceId Resource ID
   * @param memoryFreed Memory freed in bytes
   */
  virtual void BroadcastUnloadEvent(ResourceId resourceId,
                                    std::size_t memoryFreed) = 0;
  
  /**
   * Broadcast an error event.
   * @param resourceId Resource ID
   * @param message Error message
   * @param path Resource path
   */
  virtual void BroadcastErrorEvent(ResourceId resourceId,
                                   std::string const& message,
                                   std::string const& path = "") = 0;
  
  //==========================================================================
  // Event Processing
  //==========================================================================
  
  /**
   * Process pending events.
   * Should be called on the main thread to dispatch events.
   * @param maxEvents Maximum events to process (0 = all)
   */
  virtual void ProcessPendingEvents(std::size_t maxEvents = 0) = 0;
  
  /**
   * Get number of pending events.
   * @return Number of events waiting to be processed
   */
  virtual std::size_t GetPendingEventCount() const = 0;
  
  /**
   * Set whether events are processed immediately or queued.
   * @param immediate If true, events are dispatched immediately in BroadcastEvent.
   *                  If false, events are queued and dispatched in ProcessPendingEvents.
   */
  virtual void SetImmediateDispatch(bool immediate) = 0;
  
  //==========================================================================
  // Statistics
  //==========================================================================
  
  /**
   * Get total number of events broadcasted.
   * @return Total event count since startup
   */
  virtual std::size_t GetTotalEventCount() const = 0;
  
  /**
   * Get event count by type.
   * @param eventType Event type
   * @return Count of this event type
   */
  virtual std::size_t GetEventCountByType(ResourceStateEvent eventType) const = 0;
  
  /**
   * Get number of active subscriptions.
   * @return Active subscription count
   */
  virtual std::size_t GetActiveSubscriptionCount() const = 0;
};

/**
 * Get the global resource event manager.
 */
IResourceEventManager* GetResourceEventManager();

/**
 * RAII helper for event subscriptions.
 * Automatically unsubscribes when destroyed.
 */
class ScopedEventSubscription {
 public:
  ScopedEventSubscription()
    : manager_(nullptr)
    , handle_(InvalidEventSubscription) {}
  
  ScopedEventSubscription(IResourceEventManager* manager, EventSubscriptionHandle handle)
    : manager_(manager)
    , handle_(handle) {}
  
  ~ScopedEventSubscription() {
    if (manager_ && handle_ != InvalidEventSubscription) {
      manager_->Unsubscribe(handle_);
    }
  }
  
  // Move-only
  ScopedEventSubscription(ScopedEventSubscription&& other) noexcept
    : manager_(other.manager_)
    , handle_(other.handle_) {
    other.manager_ = nullptr;
    other.handle_ = InvalidEventSubscription;
  }
  
  ScopedEventSubscription& operator=(ScopedEventSubscription&& other) noexcept {
    if (this != &other) {
      if (manager_ && handle_ != InvalidEventSubscription) {
        manager_->Unsubscribe(handle_);
      }
      manager_ = other.manager_;
      handle_ = other.handle_;
      other.manager_ = nullptr;
      other.handle_ = InvalidEventSubscription;
    }
    return *this;
  }
  
  // Non-copyable
  ScopedEventSubscription(ScopedEventSubscription const&) = delete;
  ScopedEventSubscription& operator=(ScopedEventSubscription const&) = delete;
  
  bool IsValid() const { return handle_ != InvalidEventSubscription; }
  EventSubscriptionHandle GetHandle() const { return handle_; }
  
  void Release() {
    manager_ = nullptr;
    handle_ = InvalidEventSubscription;
  }
  
 private:
  IResourceEventManager* manager_;
  EventSubscriptionHandle handle_;
};

}  // namespace resource
}  // namespace te

#endif  // TE_RESOURCE_RESOURCE_EVENT_H
