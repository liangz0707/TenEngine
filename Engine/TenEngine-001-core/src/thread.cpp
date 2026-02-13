/**
 * @file thread.cpp
 * @brief Implementation of Thread, TLS, Mutex, ConditionVariable, TaskQueue, ITaskExecutor, IThreadPool per contract.
 * Uses std::thread, std::mutex, std::condition_variable. Comments in English.
 */

#include "te/core/thread.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <utility>
#include <unordered_map>
#include <atomic>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <memory>

namespace te {
namespace core {

// --- Thread ---
struct Thread::Impl {
  std::thread t;

  ~Impl() {
    if (t.joinable()) {
      t.detach();
    }
  }
};

Thread::Thread() : impl_(nullptr) {}

Thread::Thread(std::function<void()> fn) : impl_(std::make_unique<Impl>()) {
  impl_->t = std::thread(std::move(fn));
}

Thread::~Thread() {}

Thread::Thread(Thread&& other) noexcept : impl_(std::move(other.impl_)) {}

Thread& Thread::operator=(Thread&& other) noexcept {
  if (this != &other) {
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
LockGuard::~LockGuard() {
  if (mutex_) mutex_->Unlock();
}

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
  lock.release();
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

// --- TaskItem ---
struct TaskItem {
  TaskCallback callback;
  void* user_data;
  int priority;
  TaskId taskId;
  std::atomic<TaskStatus> status;
  bool cancelled;

  TaskItem(TaskCallback cb, void* ud, int prio, TaskId id)
      : callback(cb), user_data(ud), priority(prio), taskId(id),
        status(TaskStatus::Pending), cancelled(false) {}
};

// --- SingleThreadExecutor ---
class SingleThreadExecutor : public ITaskExecutor {
 public:
  SingleThreadExecutor() {
    worker_ = std::thread([this]() {
      while (true) {
        std::shared_ptr<TaskItem> item;
        {
          std::unique_lock<std::mutex> lock(m_);
          cv_.wait(lock, [this] {
            return shutdown_ || !priority_queue_.empty();
          });
          if (shutdown_ && priority_queue_.empty()) break;
          item = PopHighestPriorityTask();
          if (item && !item->cancelled) {
            item->status.store(TaskStatus::Loading);
          } else {
            item.reset();
          }
        }
        if (item && item->callback) {
          item->callback(item->user_data);
          item->status.store(TaskStatus::Completed);
          std::lock_guard<std::mutex> lock(m_);
          tasks_.erase(item->taskId);
        }
      }
    });
  }

  ~SingleThreadExecutor() override {
    {
      std::lock_guard<std::mutex> lock(m_);
      shutdown_ = true;
      cv_.notify_all();
    }
    if (worker_.joinable()) worker_.join();
  }

  TaskId SubmitTaskWithPriority(TaskCallback cb, void* ud, int prio) override {
    std::lock_guard<std::mutex> lock(m_);
    if (shutdown_) return 0;
    TaskId id = nextTaskId_.fetch_add(1);
    auto item = std::make_shared<TaskItem>(cb, ud, prio, id);
    tasks_[id] = item;
    priority_queue_.push_back(item);
    cv_.notify_one();
    return id;
  }

  void SubmitTask(TaskCallback cb, void* ud) override {
    (void)SubmitTaskWithPriority(cb, ud, 0);
  }

  bool CancelTask(TaskId id) override {
    std::lock_guard<std::mutex> lock(m_);
    auto it = tasks_.find(id);
    if (it == tasks_.end()) return false;
    auto& item = it->second;
    TaskStatus s = item->status.load();
    if (s == TaskStatus::Completed || s == TaskStatus::Failed) return false;
    item->cancelled = true;
    item->status.store(TaskStatus::Cancelled);
    tasks_.erase(it);
    return true;
  }

  TaskStatus GetTaskStatus(TaskId id) const override {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(m_));
    auto it = tasks_.find(id);
    if (it == tasks_.end()) return TaskStatus::Completed;
    return it->second->status.load();
  }

 private:
  std::shared_ptr<TaskItem> PopHighestPriorityTask() {
    if (priority_queue_.empty()) return nullptr;
    auto it = std::max_element(priority_queue_.begin(), priority_queue_.end(),
                               [](auto const& a, auto const& b) {
                                 return a->priority < b->priority;
                               });
    if (it == priority_queue_.end()) return nullptr;
    auto task = *it;
    priority_queue_.erase(it);
    return task;
  }

  std::thread worker_;
  mutable std::mutex m_;
  std::condition_variable cv_;
  std::vector<std::shared_ptr<TaskItem>> priority_queue_;
  std::unordered_map<TaskId, std::shared_ptr<TaskItem>> tasks_;
  std::atomic<TaskId> nextTaskId_{1};
  bool shutdown_ = false;
};

// --- DefaultThreadPool ---
struct DefaultThreadPool : IThreadPool {
  std::unique_ptr<SingleThreadExecutor> workerExecutor_;
  std::unique_ptr<SingleThreadExecutor> ioExecutor_;
  std::vector<ITaskExecutor*> executors_;
  std::mutex main_m_;
  std::queue<std::pair<TaskCallback, void*>> main_thread_callbacks_;
  CallbackThreadType callbackThreadType_ = CallbackThreadType::WorkerThread;

  DefaultThreadPool() {
    workerExecutor_ = std::make_unique<SingleThreadExecutor>();
    ioExecutor_ = std::make_unique<SingleThreadExecutor>();
    executors_.resize(static_cast<size_t>(ExecutorType::Count), nullptr);
    executors_[static_cast<size_t>(ExecutorType::Worker)] = workerExecutor_.get();
    executors_[static_cast<size_t>(ExecutorType::IO)] = ioExecutor_.get();
  }

  void SubmitTask(TaskCallback cb, void* ud) override {
    {
      std::lock_guard<std::mutex> lock(main_m_);
      if (callbackThreadType_ == CallbackThreadType::MainThread) {
        main_thread_callbacks_.push({cb, ud});
        return;
      }
    }
    workerExecutor_->SubmitTask(cb, ud);
  }

  void SetCallbackThread(CallbackThreadType type) override {
    std::lock_guard<std::mutex> lock(main_m_);
    callbackThreadType_ = type;
  }

  void ProcessMainThreadCallbacks() override {
    std::vector<std::pair<TaskCallback, void*>> batch;
    {
      std::lock_guard<std::mutex> lock(main_m_);
      while (!main_thread_callbacks_.empty()) {
        batch.push_back(main_thread_callbacks_.front());
        main_thread_callbacks_.pop();
      }
    }
    for (auto& p : batch) {
      if (p.first) p.first(p.second);
    }
  }

  ITaskExecutor* GetWorkerExecutor() override { return workerExecutor_.get(); }
  ITaskExecutor* GetIOExecutor() override { return ioExecutor_.get(); }

  ITaskExecutor* GetExecutor(ExecutorType type) override {
    size_t i = static_cast<size_t>(type);
    if (i >= executors_.size()) return nullptr;
    return executors_[i];
  }

  void RegisterExecutor(ExecutorType type, ITaskExecutor* executor) override {
    size_t i = static_cast<size_t>(type);
    if (i < executors_.size()) executors_[i] = executor;
  }

  void SpawnTask(TaskCallback cb, void* ud) override {
    std::thread([cb, ud]() {
      if (cb) cb(ud);
    }).detach();
  }
};

IThreadPool* GetThreadPool() {
  static DefaultThreadPool s_pool;
  return &s_pool;
}

}  // namespace core
}  // namespace te
