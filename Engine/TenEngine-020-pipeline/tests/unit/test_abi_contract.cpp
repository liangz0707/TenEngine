/**
 * @file test_abi_contract.cpp
 * @brief Minimal ABI contract test: SingleThreadQueue.
 */

#include <te/pipeline/ThreadQueue.h>
#include <atomic>
#include <chrono>
#include <thread>

int main() {
  te::pipeline::SingleThreadQueue queue;

  std::atomic<int> counter{0};
  queue.Post([&counter]() { counter = 1; });
  queue.Post([&counter]() { counter = 2; });

  // Wait for tasks to complete (simple polling)
  for (int i = 0; i < 50 && counter.load() < 2; ++i) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  return (counter.load() == 2) ? 0 : 1;
}
