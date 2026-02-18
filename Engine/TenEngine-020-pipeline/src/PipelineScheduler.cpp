/**
 * @file PipelineScheduler.cpp
 * @brief Implementation of PipelineScheduler.
 */

#include <te/pipeline/PipelineScheduler.h>

#include <te/pipeline/PipelineContext.h>
#include <te/pipeline/ThreadQueue.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace te::pipeline {

// === Thread Helpers ===

static std::thread::id sMainThreadId = std::this_thread::get_id();
static std::thread::id sGPUThreadId;

bool IsMainThread() {
  return std::this_thread::get_id() == sMainThreadId;
}

bool IsGPUThread() {
  return std::this_thread::get_id() == sGPUThreadId;
}

void SetCurrentThreadName(char const* name) {
  // Platform-specific thread naming
  // On Windows: SetThreadDescription
  // On Linux: pthread_setname_np
  // For now, just a placeholder
  (void)name;
}

// === PipelineScheduler::Impl ===

struct PhaseTiming {
  double totalTime{0.0};
  uint64_t sampleCount{0};
  double lastTime{0.0};

  void Record(double time) {
    lastTime = time;
    totalTime += time;
    sampleCount++;
  }

  double Average() const {
    return sampleCount > 0 ? totalTime / sampleCount : 0.0;
  }
};

struct ThreadPool {
  std::vector<std::thread> threads;
  std::deque<PipelineTask> tasks;
  std::mutex mutex;
  std::condition_variable cv;
  std::atomic<bool> running{false};

  void Start(uint32_t count, std::function<void()> onThreadStart = nullptr) {
    running = true;
    for (uint32_t i = 0; i < count; ++i) {
      threads.emplace_back([this, onThreadStart, i]() {
        if (onThreadStart) {
          onThreadStart();
        }
        WorkerLoop(i);
      });
    }
  }

  void Stop() {
    running = false;
    cv.notify_all();
    for (auto& t : threads) {
      if (t.joinable()) {
        t.join();
      }
    }
    threads.clear();
  }

  void Post(PipelineTask task) {
    {
      std::lock_guard<std::mutex> lock(mutex);
      tasks.push_back(std::move(task));
    }
    cv.notify_one();
  }

  void WorkerLoop(uint32_t index) {
    (void)index;
    while (running) {
      PipelineTask task;
      {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [this] { return !running || !tasks.empty(); });
        if (!running) break;
        if (tasks.empty()) continue;
        task = std::move(tasks.front());
        tasks.pop_front();
      }
      if (task) {
        task();
      }
    }
  }

  void WaitAll() {
    // Wait until task queue is empty
    while (true) {
      std::lock_guard<std::mutex> lock(mutex);
      if (tasks.empty()) break;
    }
  }
};

struct PipelineScheduler::Impl {
  SchedulerConfig config;
  SchedulerCallbacks callbacks;

  std::atomic<RenderPhase> currentPhase{RenderPhase::Idle};
  std::atomic<uint64_t> frameIndex{0};
  std::atomic<bool> isInitialized{false};

  ThreadPool poolA;  // Game update
  ThreadPool poolC;  // Collect

  std::unique_ptr<std::thread> threadD;  // GPU thread
  std::unique_ptr<SingleThreadQueue> gpuQueue;

  std::mutex frameMutex;
  std::condition_variable frameCV;
  std::atomic<bool> frameComplete{true};

  pipelinecore::FrameContext frameCtx;

  PhaseTiming phaseTimings[8];

  std::vector<double> frameTimeHistory;
  double totalFrameTime{0.0};

  void ExecutePhaseA() {
    auto start = std::chrono::high_resolution_clock::now();

    if (callbacks.onGameUpdate) {
      if (config.threadCountA > 0) {
        poolA.Post(callbacks.onGameUpdate);
      } else {
        callbacks.onGameUpdate();
      }
    }

    auto end = std::chrono::high_resolution_clock::now();
    phaseTimings[0].Record(
      std::chrono::duration<double, std::milli>(end - start).count());
  }

  void ExecutePhaseB() {
    auto start = std::chrono::high_resolution_clock::now();

    if (callbacks.onBuildPipeline) {
      callbacks.onBuildPipeline();
    }

    auto end = std::chrono::high_resolution_clock::now();
    phaseTimings[1].Record(
      std::chrono::duration<double, std::milli>(end - start).count());
  }

  void ExecutePhaseC() {
    auto start = std::chrono::high_resolution_clock::now();

    if (callbacks.onCollectRenderables) {
      if (config.enableParallelCollect && poolC.threads.size() > 0) {
        // Parallel collection handled by callback
        callbacks.onCollectRenderables();
      } else {
        callbacks.onCollectRenderables();
      }
    }

    auto end = std::chrono::high_resolution_clock::now();
    phaseTimings[2].Record(
      std::chrono::duration<double, std::milli>(end - start).count());
  }

  void ExecutePhaseD() {
    auto start = std::chrono::high_resolution_clock::now();

    // All D phases on GPU thread
    if (callbacks.onPrepareResources) {
      callbacks.onPrepareResources();
    }

    auto prepareEnd = std::chrono::high_resolution_clock::now();
    phaseTimings[3].Record(
      std::chrono::duration<double, std::milli>(prepareEnd - start).count());

    if (callbacks.onRecordCommands) {
      callbacks.onRecordCommands();
    }

    auto recordEnd = std::chrono::high_resolution_clock::now();
    phaseTimings[4].Record(
      std::chrono::duration<double, std::milli>(recordEnd - prepareEnd).count());

    if (callbacks.onSubmit) {
      callbacks.onSubmit();
    }

    auto submitEnd = std::chrono::high_resolution_clock::now();
    phaseTimings[5].Record(
      std::chrono::duration<double, std::milli>(submitEnd - recordEnd).count());

    if (callbacks.onPresent) {
      callbacks.onPresent();
    }

    auto presentEnd = std::chrono::high_resolution_clock::now();
    phaseTimings[6].Record(
      std::chrono::duration<double, std::milli>(presentEnd - submitEnd).count());

    phaseTimings[7].Record(
      std::chrono::duration<double, std::milli>(presentEnd - start).count());
  }

  void GPULoop() {
    sGPUThreadId = std::this_thread::get_id();
    SetCurrentThreadName("GPU Thread");

    while (isInitialized) {
      // Wait for work
      // Execute D phases when signaled
      if (gpuQueue) {
        // Process GPU queue
      }
    }
  }
};

// === PipelineScheduler ===

PipelineScheduler::PipelineScheduler()
  : impl_(std::make_unique<Impl>()) {
}

PipelineScheduler::~PipelineScheduler() {
  Shutdown();
}

void PipelineScheduler::Initialize(SchedulerConfig const& config) {
  impl_->config = config;

  // Initialize thread pools
  if (config.threadCountA > 0) {
    impl_->poolA.Start(config.threadCountA);
  }

  uint32_t threadCountC = config.threadCountC;
  if (threadCountC == 0) {
    threadCountC = std::thread::hardware_concurrency();
    if (threadCountC > 4) threadCountC = 4;
  }
  if (config.enableParallelCollect && threadCountC > 1) {
    impl_->poolC.Start(threadCountC);
  }

  // Initialize GPU thread
  if (config.enableThreadD) {
    impl_->gpuQueue = std::make_unique<SingleThreadQueue>();
    impl_->threadD = std::make_unique<std::thread>([this]() {
      impl_->GPULoop();
    });
  }

  impl_->isInitialized = true;
}

void PipelineScheduler::Shutdown() {
  if (!impl_->isInitialized) return;

  impl_->isInitialized = false;

  impl_->poolA.Stop();
  impl_->poolC.Stop();

  if (impl_->threadD && impl_->threadD->joinable()) {
    impl_->threadD->join();
  }

  impl_->gpuQueue.reset();
  impl_->threadD.reset();
}

bool PipelineScheduler::IsInitialized() const {
  return impl_->isInitialized;
}

void PipelineScheduler::SetCallbacks(SchedulerCallbacks const& callbacks) {
  impl_->callbacks = callbacks;
}

void PipelineScheduler::Tick() {
  if (!impl_->isInitialized) return;

  switch (impl_->currentPhase.load()) {
    case RenderPhase::Idle:
      // Ready for new frame
      break;

    case RenderPhase::GameUpdate:
      impl_->ExecutePhaseA();
      impl_->currentPhase = RenderPhase::BuildPipeline;
      break;

    case RenderPhase::BuildPipeline:
      impl_->ExecutePhaseB();
      impl_->currentPhase = RenderPhase::Collect;
      break;

    case RenderPhase::Collect:
      impl_->ExecutePhaseC();
      impl_->currentPhase = RenderPhase::Prepare;
      break;

    case RenderPhase::Prepare:
      if (impl_->callbacks.onPrepareResources) {
        impl_->callbacks.onPrepareResources();
      }
      impl_->currentPhase = RenderPhase::Record;
      break;

    case RenderPhase::Record:
      if (impl_->callbacks.onRecordCommands) {
        impl_->callbacks.onRecordCommands();
      }
      impl_->currentPhase = RenderPhase::Submit;
      break;

    case RenderPhase::Submit:
      if (impl_->callbacks.onSubmit) {
        impl_->callbacks.onSubmit();
      }
      impl_->currentPhase = RenderPhase::Present;
      break;

    case RenderPhase::Present:
      if (impl_->callbacks.onPresent) {
        impl_->callbacks.onPresent();
      }
      impl_->frameIndex++;
      impl_->currentPhase = RenderPhase::Idle;
      impl_->frameComplete = true;
      impl_->frameCV.notify_all();
      break;
  }
}

void PipelineScheduler::StartFrame(pipelinecore::FrameContext const& ctx) {
  impl_->frameCtx = ctx;
  impl_->frameComplete = false;
  impl_->currentPhase = RenderPhase::GameUpdate;
}

void PipelineScheduler::WaitForFrame() {
  std::unique_lock<std::mutex> lock(impl_->frameMutex);
  impl_->frameCV.wait(lock, [this] { return impl_->frameComplete.load(); });
}

bool PipelineScheduler::IsFrameComplete() const {
  return impl_->frameComplete;
}

RenderPhase PipelineScheduler::GetCurrentPhase() const {
  return impl_->currentPhase;
}

void PipelineScheduler::AdvancePhase() {
  switch (impl_->currentPhase) {
    case RenderPhase::Idle:
      impl_->currentPhase = RenderPhase::GameUpdate;
      break;
    case RenderPhase::GameUpdate:
      impl_->currentPhase = RenderPhase::BuildPipeline;
      break;
    case RenderPhase::BuildPipeline:
      impl_->currentPhase = RenderPhase::Collect;
      break;
    case RenderPhase::Collect:
      impl_->currentPhase = RenderPhase::Prepare;
      break;
    case RenderPhase::Prepare:
      impl_->currentPhase = RenderPhase::Record;
      break;
    case RenderPhase::Record:
      impl_->currentPhase = RenderPhase::Submit;
      break;
    case RenderPhase::Submit:
      impl_->currentPhase = RenderPhase::Present;
      break;
    case RenderPhase::Present:
      impl_->currentPhase = RenderPhase::Idle;
      impl_->frameComplete = true;
      impl_->frameCV.notify_all();
      break;
  }
}

void PipelineScheduler::ExecutePhase(RenderPhase phase) {
  switch (phase) {
    case RenderPhase::GameUpdate:
      impl_->ExecutePhaseA();
      break;
    case RenderPhase::BuildPipeline:
      impl_->ExecutePhaseB();
      break;
    case RenderPhase::Collect:
      impl_->ExecutePhaseC();
      break;
    case RenderPhase::Prepare:
    case RenderPhase::Record:
    case RenderPhase::Submit:
    case RenderPhase::Present:
      impl_->ExecutePhaseD();
      break;
    default:
      break;
  }
}

uint32_t PipelineScheduler::GetThreadCount(RenderPhase phase) const {
  switch (phase) {
    case RenderPhase::GameUpdate:
      return static_cast<uint32_t>(impl_->poolA.threads.size());
    case RenderPhase::Collect:
      return static_cast<uint32_t>(impl_->poolC.threads.size());
    default:
      return 1;
  }
}

void PipelineScheduler::PostTask(RenderPhase phase, PipelineTask task) {
  switch (phase) {
    case RenderPhase::GameUpdate:
      impl_->poolA.Post(std::move(task));
      break;
    case RenderPhase::Collect:
      impl_->poolC.Post(std::move(task));
      break;
    case RenderPhase::Prepare:
    case RenderPhase::Record:
    case RenderPhase::Submit:
    case RenderPhase::Present:
      if (impl_->gpuQueue) {
        impl_->gpuQueue->Post(std::move(task));
      }
      break;
    default:
      break;
  }
}

uint64_t PipelineScheduler::GetFrameIndex() const {
  return impl_->frameIndex;
}

double PipelineScheduler::GetPhaseTime(RenderPhase phase) const {
  uint32_t idx = static_cast<uint32_t>(phase);
  if (idx < 8) {
    return impl_->phaseTimings[idx].Average();
  }
  return 0.0;
}

double PipelineScheduler::GetAverageFrameTime() const {
  return impl_->phaseTimings[7].Average();
}

// === Free Functions ===

PipelineScheduler* CreatePipelineScheduler() {
  return new PipelineScheduler();
}

void DestroyPipelineScheduler(PipelineScheduler* scheduler) {
  delete scheduler;
}

}  // namespace te::pipeline
