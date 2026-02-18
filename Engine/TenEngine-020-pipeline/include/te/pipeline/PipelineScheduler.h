/**
 * @file PipelineScheduler.h
 * @brief 020-Pipeline: Multi-threaded pipeline scheduling.
 *
 * Coordinates execution across threads A, B, C, D:
 * - Thread A: Game update logic
 * - Thread B: Build logical pipeline
 * - Thread C: Parallel collect renderables
 * - Thread D: GPU operations (prepare, record, submit)
 */

#pragma once

#include <te/pipeline/RenderPipeline.h>
#include <te/pipelinecore/FrameContext.h>
#include <te/rendercore/types.hpp>
#include <cstdint>
#include <functional>
#include <memory>

namespace te::pipeline {

class PipelineContext;

/// Task function type
using PipelineTask = std::function<void()>;

/// Scheduler phase callbacks
struct SchedulerCallbacks {
  std::function<void()> onGameUpdate;           // Thread A
  std::function<void()> onBuildPipeline;        // Thread B
  std::function<void()> onCollectRenderables;   // Thread C
  std::function<void()> onPrepareResources;     // Thread D
  std::function<void()> onRecordCommands;       // Thread D
  std::function<void()> onSubmit;               // Thread D
  std::function<void()> onPresent;              // Thread D
};

/// Scheduler configuration
struct SchedulerConfig {
  uint32_t threadCountA{1};     // Game update threads
  uint32_t threadCountC{0};     // Collect threads (0 = auto)
  bool enableThreadD{true};     // Dedicated GPU thread
  bool enableParallelCollect{true};
  uint32_t maxFramesInFlight{2};
};

/**
 * @brief PipelineScheduler orchestrates multi-threaded rendering.
 *
 * Manages thread pools and task queues for each phase.
 * Ensures proper synchronization between phases.
 *
 * Usage:
 * 1. Set callbacks for each phase
 * 2. Call Tick() each frame to advance the pipeline
 * 3. Alternatively, use StartFrame()/WaitForFrame() for explicit control
 */
class PipelineScheduler {
public:
  PipelineScheduler();
  ~PipelineScheduler();

  PipelineScheduler(PipelineScheduler const&) = delete;
  PipelineScheduler& operator=(PipelineScheduler const&) = delete;

  /// Initialize with configuration
  void Initialize(SchedulerConfig const& config);

  /// Shutdown and wait for all threads
  void Shutdown();

  /// Check if initialized
  bool IsInitialized() const;

  // === Callbacks ===

  /// Set phase callbacks
  void SetCallbacks(SchedulerCallbacks const& callbacks);

  // === Scheduling ===

  /// Tick the scheduler (non-blocking)
  /// Advances through phases as dependencies are met
  void Tick();

  /// Start a new frame (triggers Phase A and B)
  void StartFrame(pipelinecore::FrameContext const& ctx);

  /// Wait for current frame to complete
  void WaitForFrame();

  /// Check if frame is complete
  bool IsFrameComplete() const;

  // === Phase Control ===

  /// Get current phase
  RenderPhase GetCurrentPhase() const;

  /// Advance to next phase (manual control)
  void AdvancePhase();

  /// Execute a phase immediately (blocking)
  void ExecutePhase(RenderPhase phase);

  // === Thread Access ===

  /// Get number of worker threads for phase
  uint32_t GetThreadCount(RenderPhase phase) const;

  /// Post a task to be executed (for custom work)
  void PostTask(RenderPhase phase, PipelineTask task);

  // === Statistics ===

  /// Get current frame index
  uint64_t GetFrameIndex() const;

  /// Get total time spent in phase (milliseconds)
  double GetPhaseTime(RenderPhase phase) const;

  /// Get average frame time
  double GetAverageFrameTime() const;

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

// === Thread Affinity Helpers ===

/// Check if current thread is the main thread
bool IsMainThread();

/// Check if current thread is the GPU thread (Thread D)
bool IsGPUThread();

/// Set current thread name (for debugging)
void SetCurrentThreadName(char const* name);

// === Free Functions ===

/// Create a pipeline scheduler
PipelineScheduler* CreatePipelineScheduler();

/// Destroy a pipeline scheduler
void DestroyPipelineScheduler(PipelineScheduler* scheduler);

}  // namespace te::pipeline
