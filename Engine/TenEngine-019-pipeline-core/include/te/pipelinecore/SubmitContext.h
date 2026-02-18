/**
 * @file SubmitContext.h
 * @brief 019-PipelineCore: Submission context and multi-queue synchronization.
 *
 * Provides:
 * - SyncPoint abstraction for GPU synchronization
 * - SubmitContext for queue submission management
 * - MultiQueueScheduler for Graphics/Compute/Copy queue orchestration
 */

#pragma once

#include <te/pipelinecore/Config.h>
#include <te/rhi/types.hpp>
#include <cstdint>
#include <vector>
#include <functional>

namespace te::rhi {
struct IDevice;
struct IQueue;
struct ICommandList;
struct IFence;
struct ISemaphore;
}

namespace te::pipelinecore {

/// Queue type identifier
enum class QueueId : uint8_t {
  Graphics = 0,
  Compute = 1,
  Copy = 2,
  Count = 3
};

/// Synchronization primitive type
enum class SyncPrimitiveType : uint8_t {
  Fence,      // CPU-GPU sync
  Semaphore   // GPU-GPU sync
};

/**
 * @brief SyncPoint represents a synchronization point on the GPU timeline.
 *
 * Can be either a Fence (CPU waitable) or Semaphore (GPU waitable).
 * Used for frame synchronization and queue synchronization.
 */
class SyncPoint {
public:
  SyncPoint();
  ~SyncPoint();

  SyncPoint(SyncPoint const&) = delete;
  SyncPoint& operator=(SyncPoint const&) = delete;
  SyncPoint(SyncPoint&&) noexcept;
  SyncPoint& operator=(SyncPoint&&) noexcept;

  /// Initialize as a fence (CPU-GPU sync)
  bool InitializeAsFence(rhi::IDevice* device, bool signaled = false);

  /// Initialize as a semaphore (GPU-GPU sync)
  bool InitializeAsSemaphore(rhi::IDevice* device);

  /// Check if initialized
  bool IsValid() const;

  /// Get the underlying fence (if fence type)
  rhi::IFence* GetFence() const;

  /// Get the underlying semaphore (if semaphore type)
  rhi::ISemaphore* GetSemaphore() const;

  /// Wait on CPU (fence only)
  void Wait();

  /// Signal from CPU (fence only)
  void Signal();

  /// Reset fence for reuse
  void Reset();

  /// Get primitive type
  SyncPrimitiveType GetType() const;

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

/**
 * @brief QueueSyncPoint tracks synchronization between queues.
 *
 * Records which semaphore to wait on and signal for cross-queue dependencies.
 */
struct QueueSyncPoint {
  QueueId queue{QueueId::Graphics};
  rhi::ISemaphore* waitSemaphore{nullptr};    // Wait before executing
  rhi::ISemaphore* signalSemaphore{nullptr};  // Signal after executing
  uint64_t waitValue{0};                      // Timeline semaphore wait value
  uint64_t signalValue{0};                    // Timeline semaphore signal value
};

/**
 * @brief SubmitBatch represents a batch of command lists to submit together.
 */
struct SubmitBatch {
  QueueId queue{QueueId::Graphics};
  std::vector<rhi::ICommandList*> commandLists;
  std::vector<QueueSyncPoint> waitSyncs;
  std::vector<QueueSyncPoint> signalSyncs;
  rhi::IFence* signalFence{nullptr};          // Optional: signal fence when done
};

/**
 * @brief SubmitContext manages command submission to GPU queues.
 *
 * Tracks pending submissions and their synchronization requirements.
 * Batches commands for efficient submission.
 */
class SubmitContext {
public:
  SubmitContext();
  ~SubmitContext();

  SubmitContext(SubmitContext const&) = delete;
  SubmitContext& operator=(SubmitContext const&) = delete;
  SubmitContext(SubmitContext&&) noexcept;
  SubmitContext& operator=(SubmitContext&&) noexcept;

  /// Set the device
  void SetDevice(rhi::IDevice* device);

  // === Queue Access ===

  /// Get queue by type
  rhi::IQueue* GetQueue(QueueId queue);

  /// Get graphics queue (convenience)
  rhi::IQueue* GetGraphicsQueue();

  /// Get compute queue
  rhi::IQueue* GetComputeQueue();

  /// Get copy queue
  rhi::IQueue* GetCopyQueue();

  // === Command Recording ===

  /// Begin recording commands for a queue
  rhi::ICommandList* BeginCommandList(QueueId queue);

  /// End recording and add to pending batch
  void EndCommandList(rhi::ICommandList* cmd);

  // === Submission ===

  /// Submit all pending command lists for a queue
  void SubmitQueue(QueueId queue);

  /// Submit all pending command lists for all queues
  void SubmitAll();

  /// Submit a specific batch
  void SubmitBatch(SubmitBatch const& batch);

  // === Synchronization ===

  /// Create a semaphore for queue synchronization
  rhi::ISemaphore* CreateSemaphore();

  /// Destroy a semaphore
  void DestroySemaphore(rhi::ISemaphore* sem);

  /// Create a fence
  rhi::IFence* CreateFence(bool signaled = false);

  /// Destroy a fence
  void DestroyFence(rhi::IFence* fence);

  /// Wait for a queue to be idle
  void WaitQueueIdle(QueueId queue);

  /// Wait for all queues to be idle
  void WaitAllIdle();

  // === Frame Synchronization ===

  /// Get current frame fence for a queue
  rhi::IFence* GetCurrentFrameFence(QueueId queue);

  /// Wait for current frame to complete on a queue
  void WaitForCurrentFrame(QueueId queue);

  /// Advance to next frame (recycle fences)
  void AdvanceFrame();

  /// Get current frame index
  uint32_t GetCurrentFrameIndex() const;

  /// Get frames in flight count
  uint32_t GetFramesInFlight() const;

  // === Cleanup ===

  /// Release all resources
  void Reset();

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

/**
 * @brief MultiQueueScheduler orchestrates work across multiple queues.
 *
 * Features:
 * - Automatic dependency tracking between queues
 * - Optimal barrier placement
 * - Async compute scheduling
 * - Resource upload prioritization
 */
class MultiQueueScheduler {
public:
  MultiQueueScheduler();
  ~MultiQueueScheduler();

  MultiQueueScheduler(MultiQueueScheduler const&) = delete;
  MultiQueueScheduler& operator=(MultiQueueScheduler const&) = delete;

  /// Initialize with device
  void Initialize(rhi::IDevice* device);

  // === Queue Priority ===

  /// Set queue priority (affects scheduling order)
  void SetQueuePriority(QueueId queue, uint32_t priority);

  /// Get queue priority
  uint32_t GetQueuePriority(QueueId queue) const;

  // === Work Submission ===

  /// Submit work to graphics queue
  void SubmitGraphics(
    std::vector<rhi::ICommandList*> const& cmds,
    std::vector<QueueSyncPoint> const& waits = {},
    std::vector<QueueSyncPoint> const& signals = {});

  /// Submit work to compute queue
  void SubmitCompute(
    std::vector<rhi::ICommandList*> const& cmds,
    std::vector<QueueSyncPoint> const& waits = {},
    std::vector<QueueSyncPoint> const& signals = {});

  /// Submit work to copy queue
  void SubmitCopy(
    std::vector<rhi::ICommandList*> const& cmds,
    std::vector<QueueSyncPoint> const& waits = {},
    std::vector<QueueSyncPoint> const& signals = {});

  // === Cross-Queue Sync ===

  /// Create sync point for compute to graphics dependency
  /// Graphics will wait for compute to signal
  QueueSyncPoint CreateComputeToGraphicsSync();

  /// Create sync point for copy to graphics dependency
  QueueSyncPoint CreateCopyToGraphicsSync();

  /// Create sync point for copy to compute dependency
  QueueSyncPoint CreateCopyToComputeSync();

  // === Execution ===

  /// Execute all scheduled work
  void Execute();

  /// Wait for all work to complete
  void WaitAll();

  /// Advance to next frame
  void NextFrame();

  // === Stats ===

  /// Get pending work count for a queue
  uint32_t GetPendingWorkCount(QueueId queue) const;

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

// === Free Functions ===

/// Create a submit context
SubmitContext* CreateSubmitContext();

/// Destroy a submit context
void DestroySubmitContext(SubmitContext* ctx);

/// Create a multi-queue scheduler
MultiQueueScheduler* CreateMultiQueueScheduler();

/// Destroy a multi-queue scheduler
void DestroyMultiQueueScheduler(MultiQueueScheduler* scheduler);

}  // namespace te::pipelinecore
