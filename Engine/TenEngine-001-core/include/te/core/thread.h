/**
 * @file thread.h
 * @brief Thread, TLS, Atomic, Mutex, ConditionVariable, TaskQueue (contract: 001-core-public-api.md).
 * Only contract-declared types and API are exposed.
 */
#ifndef TE_CORE_THREAD_H
#define TE_CORE_THREAD_H

#include <atomic>
#include <cstddef>
#include <functional>
#include <memory>

namespace te {
namespace core {

/** Thread handle: create, join, detach per contract capability 2. */
class Thread {
 public:
  Thread();
  /** Creates a thread that runs \a fn. */
  explicit Thread(std::function<void()> fn);
  ~Thread();

  Thread(Thread const&) = delete;
  Thread& operator=(Thread const&) = delete;
  Thread(Thread&& other) noexcept;
  Thread& operator=(Thread&& other) noexcept;

  /** Wait for the thread to finish. */
  void Join();
  /** Detach the thread so it runs independently. */
  void Detach();
  /** True if the thread is joinable. */
  bool Joinable() const;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

/** Thread-local storage slot; semantics per contract capability 2. */
template <typename T>
class TLS {
 public:
  T* Get() { return &value_; }
  T const* Get() const { return &value_; }
  void Set(T const& value) { value_ = value; }

 private:
  thread_local static T value_;
};
template <typename T>
thread_local T TLS<T>::value_;

/** Atomic type wrapper per contract capability 2. */
template <typename T>
class Atomic {
 public:
  Atomic() = default;
  explicit Atomic(T value) : value_(value) {}
  T Load() const { return value_.load(); }
  void Store(T value) { value_.store(value); }
  T Exchange(T desired) { return value_.exchange(desired); }
  bool CompareExchangeStrong(T& expected, T desired) {
    return value_.compare_exchange_strong(expected, desired);
  }

 private:
  std::atomic<T> value_;
};

/** Mutex per contract capability 2. */
class Mutex {
 public:
  Mutex();
  ~Mutex();
  Mutex(Mutex const&) = delete;
  Mutex& operator=(Mutex const&) = delete;

  void Lock();
  void Unlock();
  bool TryLock();

 private:
  friend class ConditionVariable;
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

/** RAII lock guard for Mutex. */
class LockGuard {
 public:
  explicit LockGuard(Mutex& m);
  ~LockGuard();
  LockGuard(LockGuard const&) = delete;
  LockGuard& operator=(LockGuard const&) = delete;

 private:
  Mutex* mutex_;
};

/** Condition variable per contract capability 2. */
class ConditionVariable {
 public:
  ConditionVariable();
  ~ConditionVariable();
  ConditionVariable(ConditionVariable const&) = delete;
  ConditionVariable& operator=(ConditionVariable const&) = delete;

  /** Wait; caller must hold \a m locked. Unlocks \a m, waits, then re-locks before returning. */
  void Wait(Mutex& m);
  void NotifyOne();
  void NotifyAll();

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

/** Task queue skeleton: submit task, execution and sync semantics per contract capability 2. */
class TaskQueue {
 public:
  TaskQueue();
  ~TaskQueue();

  TaskQueue(TaskQueue const&) = delete;
  TaskQueue& operator=(TaskQueue const&) = delete;

  /** Submit a task to be executed. */
  void Submit(std::function<void()> task);
  /** Run one task from the queue (if any). Returns true if a task was run. */
  bool RunOne();
  /** Shut down the queue; no more tasks accepted. */
  void Shutdown();

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

/** Task callback type per ABI: executed on worker thread. */
using TaskCallback = void (*)(void* user_data);

/** Task ID type: opaque handle returned by SubmitTaskWithPriority. */
using TaskId = std::uint64_t;

/** Task status enumeration per ABI. */
enum class TaskStatus {
  Pending,
  Loading,
  Completed,
  Failed,
  Cancelled
};

/** Callback thread type enumeration per ABI. */
enum class CallbackThreadType {
  MainThread,
  WorkerThread
};

/** Thread pool interface per ABI; SubmitTask is thread-safe. */
struct IThreadPool {
  virtual void SubmitTask(TaskCallback callback, void* user_data) = 0;
  /** Submit task with priority. Higher priority values have higher priority. Returns TaskId. */
  virtual TaskId SubmitTaskWithPriority(TaskCallback callback, void* user_data, int priority) = 0;
  /** Cancel task. Returns true if successfully cancelled, false if task already completed or doesn't exist. */
  virtual bool CancelTask(TaskId taskId) = 0;
  /** Get task status. Returns Pending/Loading/Completed/Failed/Cancelled. */
  virtual TaskStatus GetTaskStatus(TaskId taskId) const = 0;
  /** Set callback thread type (MainThread or WorkerThread). */
  virtual void SetCallbackThread(CallbackThreadType threadType) = 0;
  virtual ~IThreadPool() = default;
};

/** Return global thread pool; caller does not own the pointer. */
IThreadPool* GetThreadPool();

}  // namespace core
}  // namespace te

#endif  // TE_CORE_THREAD_H
