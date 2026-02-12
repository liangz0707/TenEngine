/**
 * @file ThreadQueue.cpp
 * @brief 020-Pipeline: SingleThreadQueue implementation.
 */

#include <te/pipeline/ThreadQueue.h>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

namespace te {
namespace pipeline {

struct SingleThreadQueue::Impl {
  std::mutex mutex;
  std::condition_variable cond;
  std::queue<std::function<void()>> queue;
  bool stop{false};
  std::thread thread;
};

SingleThreadQueue::SingleThreadQueue() : impl_(std::make_unique<Impl>()) {
  impl_->thread = std::thread([this]() {
    for (;;) {
      std::function<void()> task;
      {
        std::unique_lock<std::mutex> lock(impl_->mutex);
        impl_->cond.wait(lock, [this]() { return impl_->stop || !impl_->queue.empty(); });
        if (impl_->stop && impl_->queue.empty()) break;
        if (!impl_->queue.empty()) {
          task = std::move(impl_->queue.front());
          impl_->queue.pop();
        }
      }
      if (task) task();
    }
  });
}

SingleThreadQueue::~SingleThreadQueue() {
  {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    impl_->stop = true;
  }
  impl_->cond.notify_all();
  if (impl_->thread.joinable()) impl_->thread.join();
}

void SingleThreadQueue::Post(std::function<void()> task) {
  {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    impl_->queue.push(std::move(task));
  }
  impl_->cond.notify_one();
}

}  // namespace pipeline
}  // namespace te
