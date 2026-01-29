/**
 * @file test_thread.cpp
 * @brief Unit tests for Thread, Mutex, ConditionVariable, TaskQueue per contract capability 2.
 */

#include "te/core/thread.h"
#include <cassert>
#include <atomic>

using namespace te::core;

int main() {
  // Thread: create, join
  int done = 0;
  Thread t([&done]() { done = 1; });
  assert(t.Joinable());
  t.Join();
  assert(done == 1);

  // Mutex + LockGuard
  Mutex m;
  int counter = 0;
  Thread t2([&m, &counter]() {
    LockGuard guard(m);
    ++counter;
  });
  t2.Join();
  assert(counter == 1);

  // Atomic<T>
  Atomic<int> a(0);
  assert(a.Load() == 0);
  a.Store(42);
  assert(a.Load() == 42);
  a.Exchange(10);
  assert(a.Load() == 10);

  // TaskQueue: submit and run one
  TaskQueue q;
  int task_ran = 0;
  q.Submit([&task_ran]() { task_ran = 1; });
  assert(q.RunOne());
  assert(task_ran == 1);
  assert(!q.RunOne());
  q.Shutdown();

  // TLS (per-thread value)
  TLS<int> tls;
  tls.Set(100);
  assert(tls.Get() != nullptr && *tls.Get() == 100);

  return 0;
}
