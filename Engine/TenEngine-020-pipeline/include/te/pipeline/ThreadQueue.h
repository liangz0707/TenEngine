/**
 * @file ThreadQueue.h
 * @brief 020-Pipeline: Single-thread task queue for thread scheduling.
 */

#ifndef TE_PIPELINE_THREAD_QUEUE_H
#define TE_PIPELINE_THREAD_QUEUE_H

#include <functional>
#include <memory>

namespace te {
namespace pipeline {

/// Single-thread task queue: spawns a dedicated worker thread that processes tasks via Post().
/// Tasks are executed sequentially on the worker thread. Thread-safe for Post().
class SingleThreadQueue {
 public:
  SingleThreadQueue();
  ~SingleThreadQueue();

  SingleThreadQueue(SingleThreadQueue const&) = delete;
  SingleThreadQueue& operator=(SingleThreadQueue const&) = delete;

  /// Post a task to be executed on the worker thread. Thread-safe.
  void Post(std::function<void()> task);

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace pipeline
}  // namespace te

#endif  // TE_PIPELINE_THREAD_QUEUE_H
