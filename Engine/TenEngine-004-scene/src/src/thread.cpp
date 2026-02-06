/**
 * @file thread.cpp
 * @brief Implementation of Thread, TLS, Atomic, Mutex, ConditionVariable, TaskQueue, IThreadPool, GetThreadPool per contract (001-core-ABI.md).
 * Uses std::thread, std::mutex, std::condition_variable. Comments in English.
 */

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <utility>
#include "te/core/thread.h"

namespace te {
namespace core {

// --- Thread ---
struct Thread::Impl {
  std::thread t;
};

Thread::Thread(std::function<void()> fn) : impl_(std::make_unique<Impl>()) {
  impl_->t = std::thread(std::move(fn));
}

Thread::~Thread() {
  if (impl_ && impl_->t.joinable()) impl_->t.detach();
}

Thread::Thread(Thread&& other) noexcept : impl_(std::move(other.impl_)) {}

Thread& Thread::operator=(Thread&& other) noexcept {
  if (this != &other) {
    if (impl_ && impl_->t.joinable()) impl_->t.detach();
    impl_ = std::move(other.impl_);
  }
  return *this;
}

void Thread::Join() {
  if (impl_ && impl_->t.joinable()) impl_->t.join();
}

void Thread::Detach() {
  if (impl_ && impl_->t.joinable()) impl_->t.detach();
}

bool Thread::Joinable() const {
  return impl_ && impl_->t.joinable();
}

// --- Mutex ---
struct Mutex::Impl {
  std::mutex m;
};

Mutex::Mutex() : impl_(std::make_unique<Impl>()) {}
Mutex::~Mutex() = default;

void Mutex::Lock() { impl_->m.lock(); }
void Mutex::Unlock() { impl_->m.unlock(); }
bool Mutex::TryLock() { return impl_->m.try_lock(); }

// --- LockGuard ---
LockGuard::LockGuard(Mutex& m) : mutex_(&m) { mutex_->Lock(); }
LockGuard::~LockGuard() { if (mutex_) mutex_->Unlock(); }

// --- ConditionVariable ---
struct ConditionVariable::Impl {
  std::condition_variable cv;
};

ConditionVariable::ConditionVariable() : impl_(std::make_unique<Impl>()) {}
ConditionVariable::~ConditionVariable() = default;

void ConditionVariable::Wait(Mutex& m) {
  m.Unlock();
  std::unique_lock<std::mutex> lock(m.impl_->m);
  impl_->cv.wait(lock);
  lock.release();  // leave mutex locked for caller
}

void ConditionVariable::NotifyOne() { impl_->cv.notify_one(); }
void ConditionVariable::NotifyAll() { impl_->cv.notify_all(); }

// --- TaskQueue ---
struct TaskQueue::Impl {
  std::mutex m;
  std::condition_variable cv;
  std::queue<std::function<void()>> queue;
  bool shutdown = false;
};

TaskQueue::TaskQueue() : impl_(std::make_unique<Impl>()) {}

TaskQueue::~TaskQueue() { Shutdown(); }

void TaskQueue::Submit(std::function<void()> task) {
  if (!impl_ || impl_->shutdown) return;
  std::lock_guard<std::mutex> lock(impl_->m);
  if (impl_->shutdown) return;
  impl_->queue.push(std::move(task));
  impl_->cv.notify_one();
}

bool TaskQueue::RunOne() {
  if (!impl_) return false;
  std::function<void()> task;
  {
    std::lock_guard<std::mutex> lock(impl_->m);
    if (impl_->queue.empty()) return false;
    task = std::move(impl_->queue.front());
    impl_->queue.pop();
  }
  if (task) task();
  return true;
}

void TaskQueue::Shutdown() {
  if (!impl_) return;
  std::lock_guard<std::mutex> lock(impl_->m);
  impl_->shutdown = true;
  impl_->cv.notify_all();
}

// --- IThreadPool / GetThreadPool ---
struct DefaultThreadPool : IThreadPool {
  std::thread worker;
  std::mutex m;
  std::condition_variable cv;
  std::queue<std::pair<TaskCallback, void*>> queue;
  bool shutdown = false;

  DefaultThreadPool() {
    worker = std::thread([this]() {
      while (true) {
        std::pair<TaskCallback, void*> item{nullptr, nullptr};
        {
          std::unique_lock<std::mutex> lock(m);
          cv.wait(lock, [this] { return shutdown || !queue.empty(); });
          if (shutdown && queue.empty()) break;
          if (!queue.empty()) {
            item = std::move(queue.front());
            queue.pop();
          }
        }
        if (item.first) item.first(item.second);
      }
    });
  }

  ~DefaultThreadPool() override {
    {
      std::lock_guard<std::mutex> lock(m);
      shutdown = true;
      cv.notify_all();
    }
    if (worker.joinable()) worker.join();
  }

  void SubmitTask(TaskCallback callback, void* user_data) override {
    std::lock_guard<std::mutex> lock(m);
    if (shutdown) return;
    queue.push({callback, user_data});
    cv.notify_one();
  }
};

IThreadPool* GetThreadPool() {
  static DefaultThreadPool s_pool;
  return &s_pool;
}

}  // namespace core
}  // namespace te
