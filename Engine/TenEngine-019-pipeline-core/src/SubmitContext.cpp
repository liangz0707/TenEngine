/**
 * @file SubmitContext.cpp
 * @brief Implementation of SubmitContext and MultiQueueScheduler.
 */

#include <te/pipelinecore/SubmitContext.h>

#include <te/rhi/device.hpp>
#include <te/rhi/queue.hpp>
#include <te/rhi/command_list.hpp>
#include <te/rhi/sync.hpp>

#include <algorithm>
#include <array>
#include <cassert>
#include <memory>

namespace te::pipelinecore {

// === SyncPoint::Impl ===

struct SyncPoint::Impl {
  SyncPrimitiveType type{SyncPrimitiveType::Fence};
  rhi::IFence* fence{nullptr};
  rhi::ISemaphore* semaphore{nullptr};
  rhi::IDevice* device{nullptr};
};

// === SyncPoint ===

SyncPoint::SyncPoint()
  : impl_(std::make_unique<Impl>()) {
}

SyncPoint::~SyncPoint() {
  if (impl_ && impl_->device) {
    if (impl_->fence) {
      impl_->device->DestroyFence(impl_->fence);
    }
    if (impl_->semaphore) {
      impl_->device->DestroySemaphore(impl_->semaphore);
    }
  }
}

SyncPoint::SyncPoint(SyncPoint&& other) noexcept
  : impl_(std::move(other.impl_)) {
}

SyncPoint& SyncPoint::operator=(SyncPoint&& other) noexcept {
  if (this != &other) {
    impl_ = std::move(other.impl_);
  }
  return *this;
}

bool SyncPoint::InitializeAsFence(rhi::IDevice* device, bool signaled) {
  impl_->device = device;
  impl_->type = SyncPrimitiveType::Fence;

  if (device) {
    impl_->fence = device->CreateFence(signaled);
    return impl_->fence != nullptr;
  }
  return false;
}

bool SyncPoint::InitializeAsSemaphore(rhi::IDevice* device) {
  impl_->device = device;
  impl_->type = SyncPrimitiveType::Semaphore;

  if (device) {
    impl_->semaphore = device->CreateSemaphore();
    return impl_->semaphore != nullptr;
  }
  return false;
}

bool SyncPoint::IsValid() const {
  return impl_ && impl_->device &&
         ((impl_->type == SyncPrimitiveType::Fence && impl_->fence) ||
          (impl_->type == SyncPrimitiveType::Semaphore && impl_->semaphore));
}

rhi::IFence* SyncPoint::GetFence() const {
  return impl_->fence;
}

rhi::ISemaphore* SyncPoint::GetSemaphore() const {
  return impl_->semaphore;
}

void SyncPoint::Wait() {
  if (impl_->fence) {
    impl_->fence->Wait();
  }
}

void SyncPoint::Signal() {
  if (impl_->fence) {
    impl_->fence->Signal();
  }
}

void SyncPoint::Reset() {
  if (impl_->fence) {
    impl_->fence->Reset();
  }
}

SyncPrimitiveType SyncPoint::GetType() const {
  return impl_->type;
}

// === SubmitContext::Impl ===

struct QueueData {
  rhi::IQueue* queue{nullptr};
  std::vector<rhi::ICommandList*> pendingCommands;
  std::vector<rhi::IFence*> frameFences;
  uint32_t currentFrameFence{0};
};

struct SubmitContext::Impl {
  rhi::IDevice* device{nullptr};
  std::array<QueueData, static_cast<size_t>(QueueId::Count)> queues;
  uint32_t currentFrame{0};
  uint32_t framesInFlight{kMaxFramesInFlight};

  void InitQueues() {
    if (!device) return;

    // Graphics queue (index 0)
    queues[0].queue = device->GetQueue(rhi::QueueType::Graphics, 0);

    // Compute queue (index 0)
    queues[1].queue = device->GetQueue(rhi::QueueType::Compute, 0);

    // Copy queue (index 0)
    queues[2].queue = device->GetQueue(rhi::QueueType::Copy, 0);

    // Create frame fences for each queue
    for (auto& qd : queues) {
      qd.frameFences.resize(framesInFlight);
      for (uint32_t i = 0; i < framesInFlight; ++i) {
        qd.frameFences[i] = device->CreateFence(true);
      }
    }
  }

  void Cleanup() {
    if (!device) return;

    for (auto& qd : queues) {
      for (auto* cmd : qd.pendingCommands) {
        device->DestroyCommandList(cmd);
      }
      qd.pendingCommands.clear();

      for (auto* fence : qd.frameFences) {
        device->DestroyFence(fence);
      }
      qd.frameFences.clear();
    }
  }
};

// === SubmitContext ===

SubmitContext::SubmitContext()
  : impl_(std::make_unique<Impl>()) {
}

SubmitContext::~SubmitContext() {
  if (impl_) {
    impl_->Cleanup();
  }
}

SubmitContext::SubmitContext(SubmitContext&& other) noexcept
  : impl_(std::move(other.impl_)) {
}

SubmitContext& SubmitContext::operator=(SubmitContext&& other) noexcept {
  if (this != &other) {
    impl_ = std::move(other.impl_);
  }
  return *this;
}

void SubmitContext::SetDevice(rhi::IDevice* device) {
  impl_->device = device;
  impl_->InitQueues();
}

rhi::IQueue* SubmitContext::GetQueue(QueueId queue) {
  size_t idx = static_cast<size_t>(queue);
  if (idx < impl_->queues.size()) {
    return impl_->queues[idx].queue;
  }
  return nullptr;
}

rhi::IQueue* SubmitContext::GetGraphicsQueue() {
  return GetQueue(QueueId::Graphics);
}

rhi::IQueue* SubmitContext::GetComputeQueue() {
  return GetQueue(QueueId::Compute);
}

rhi::IQueue* SubmitContext::GetCopyQueue() {
  return GetQueue(QueueId::Copy);
}

rhi::ICommandList* SubmitContext::BeginCommandList(QueueId queue) {
  if (!impl_->device) return nullptr;

  auto* cmd = impl_->device->CreateCommandList();
  if (cmd) {
    cmd->Begin();
  }
  return cmd;
}

void SubmitContext::EndCommandList(rhi::ICommandList* cmd) {
  if (cmd) {
    cmd->End();
    // Add to graphics queue by default
    impl_->queues[0].pendingCommands.push_back(cmd);
  }
}

void SubmitContext::SubmitQueue(QueueId queue) {
  size_t idx = static_cast<size_t>(queue);
  if (idx >= impl_->queues.size() || !impl_->device) return;

  auto& qd = impl_->queues[idx];
  if (!qd.queue || qd.pendingCommands.empty()) return;

  // Submit with frame fence
  auto* fence = qd.frameFences[qd.currentFrameFence];
  qd.queue->Submit(qd.pendingCommands.data(),
                   static_cast<uint32_t>(qd.pendingCommands.size()),
                   fence, nullptr, nullptr);

  // Clear pending (don't destroy - let frame tracking handle it)
  qd.pendingCommands.clear();
}

void SubmitContext::SubmitAll() {
  for (size_t i = 0; i < static_cast<size_t>(QueueId::Count); ++i) {
    SubmitQueue(static_cast<QueueId>(i));
  }
}

void SubmitContext::SubmitBatch(SubmitBatch const& batch) {
  size_t idx = static_cast<size_t>(batch.queue);
  if (idx >= impl_->queues.size()) return;

  auto& qd = impl_->queues[idx];
  if (!qd.queue || batch.commandLists.empty()) return;

  // Extract wait semaphores
  std::vector<rhi::ISemaphore*> waitSemaphores;
  for (auto const& sync : batch.waitSyncs) {
    if (sync.waitSemaphore) {
      waitSemaphores.push_back(sync.waitSemaphore);
    }
  }

  // Extract signal semaphores
  std::vector<rhi::ISemaphore*> signalSemaphores;
  for (auto const& sync : batch.signalSyncs) {
    if (sync.signalSemaphore) {
      signalSemaphores.push_back(sync.signalSemaphore);
    }
  }

  qd.queue->Submit(
    batch.commandLists.data(),
    static_cast<uint32_t>(batch.commandLists.size()),
    batch.signalFence,
    waitSemaphores.empty() ? nullptr : waitSemaphores.data(),
    signalSemaphores.empty() ? nullptr : signalSemaphores.data());
}

rhi::ISemaphore* SubmitContext::CreateSemaphore() {
  return impl_->device ? impl_->device->CreateSemaphore() : nullptr;
}

void SubmitContext::DestroySemaphore(rhi::ISemaphore* sem) {
  if (impl_->device && sem) {
    impl_->device->DestroySemaphore(sem);
  }
}

rhi::IFence* SubmitContext::CreateFence(bool signaled) {
  return impl_->device ? impl_->device->CreateFence(signaled) : nullptr;
}

void SubmitContext::DestroyFence(rhi::IFence* fence) {
  if (impl_->device && fence) {
    impl_->device->DestroyFence(fence);
  }
}

void SubmitContext::WaitQueueIdle(QueueId queue) {
  auto* q = GetQueue(queue);
  if (q) {
    q->WaitIdle();
  }
}

void SubmitContext::WaitAllIdle() {
  for (size_t i = 0; i < static_cast<size_t>(QueueId::Count); ++i) {
    WaitQueueIdle(static_cast<QueueId>(i));
  }
}

rhi::IFence* SubmitContext::GetCurrentFrameFence(QueueId queue) {
  size_t idx = static_cast<size_t>(queue);
  if (idx < impl_->queues.size()) {
    return impl_->queues[idx].frameFences[impl_->queues[idx].currentFrameFence];
  }
  return nullptr;
}

void SubmitContext::WaitForCurrentFrame(QueueId queue) {
  auto* fence = GetCurrentFrameFence(queue);
  if (fence) {
    fence->Wait();
  }
}

void SubmitContext::AdvanceFrame() {
  // Wait for oldest frame to complete
  uint32_t oldestFrame = (impl_->currentFrame + 1) % impl_->framesInFlight;
  for (auto& qd : impl_->queues) {
    if (qd.frameFences[oldestFrame]) {
      qd.frameFences[oldestFrame]->Wait();
      qd.frameFences[oldestFrame]->Reset();
    }
  }

  // Advance frame
  impl_->currentFrame = (impl_->currentFrame + 1) % impl_->framesInFlight;
  for (auto& qd : impl_->queues) {
    qd.currentFrameFence = impl_->currentFrame;
  }
}

uint32_t SubmitContext::GetCurrentFrameIndex() const {
  return impl_->currentFrame;
}

uint32_t SubmitContext::GetFramesInFlight() const {
  return impl_->framesInFlight;
}

void SubmitContext::Reset() {
  impl_->Cleanup();
  impl_->currentFrame = 0;
}

// === MultiQueueScheduler::Impl ===

struct QueueWork {
  std::vector<SubmitBatch> batches;
  uint32_t priority{0};
};

struct MultiQueueScheduler::Impl {
  rhi::IDevice* device{nullptr};
  std::array<QueueWork, static_cast<size_t>(QueueId::Count)> queueWork;
  SubmitContext submitCtx;

  // Semaphores for cross-queue sync
  rhi::ISemaphore* computeToGraphicsSem{nullptr};
  rhi::ISemaphore* copyToGraphicsSem{nullptr};
  rhi::ISemaphore* copyToComputeSem{nullptr};
};

// === MultiQueueScheduler ===

MultiQueueScheduler::MultiQueueScheduler()
  : impl_(std::make_unique<Impl>()) {
}

MultiQueueScheduler::~MultiQueueScheduler() {
  if (impl_ && impl_->device) {
    if (impl_->computeToGraphicsSem) {
      impl_->device->DestroySemaphore(impl_->computeToGraphicsSem);
    }
    if (impl_->copyToGraphicsSem) {
      impl_->device->DestroySemaphore(impl_->copyToGraphicsSem);
    }
    if (impl_->copyToComputeSem) {
      impl_->device->DestroySemaphore(impl_->copyToComputeSem);
    }
  }
}

void MultiQueueScheduler::Initialize(rhi::IDevice* device) {
  impl_->device = device;
  impl_->submitCtx.SetDevice(device);

  // Create cross-queue semaphores
  if (device) {
    impl_->computeToGraphicsSem = device->CreateSemaphore();
    impl_->copyToGraphicsSem = device->CreateSemaphore();
    impl_->copyToComputeSem = device->CreateSemaphore();
  }

  // Set default priorities
  impl_->queueWork[0].priority = 2; // Graphics highest
  impl_->queueWork[1].priority = 1; // Compute
  impl_->queueWork[2].priority = 0; // Copy lowest
}

void MultiQueueScheduler::SetQueuePriority(QueueId queue, uint32_t priority) {
  size_t idx = static_cast<size_t>(queue);
  if (idx < impl_->queueWork.size()) {
    impl_->queueWork[idx].priority = priority;
  }
}

uint32_t MultiQueueScheduler::GetQueuePriority(QueueId queue) const {
  size_t idx = static_cast<size_t>(queue);
  if (idx < impl_->queueWork.size()) {
    return impl_->queueWork[idx].priority;
  }
  return 0;
}

void MultiQueueScheduler::SubmitGraphics(
    std::vector<rhi::ICommandList*> const& cmds,
    std::vector<QueueSyncPoint> const& waits,
    std::vector<QueueSyncPoint> const& signals) {
  SubmitBatch batch{};
  batch.queue = QueueId::Graphics;
  batch.commandLists = cmds;
  batch.waitSyncs = waits;
  batch.signalSyncs = signals;
  impl_->queueWork[0].batches.push_back(batch);
}

void MultiQueueScheduler::SubmitCompute(
    std::vector<rhi::ICommandList*> const& cmds,
    std::vector<QueueSyncPoint> const& waits,
    std::vector<QueueSyncPoint> const& signals) {
  SubmitBatch batch{};
  batch.queue = QueueId::Compute;
  batch.commandLists = cmds;
  batch.waitSyncs = waits;
  batch.signalSyncs = signals;
  impl_->queueWork[1].batches.push_back(batch);
}

void MultiQueueScheduler::SubmitCopy(
    std::vector<rhi::ICommandList*> const& cmds,
    std::vector<QueueSyncPoint> const& waits,
    std::vector<QueueSyncPoint> const& signals) {
  SubmitBatch batch{};
  batch.queue = QueueId::Copy;
  batch.commandLists = cmds;
  batch.waitSyncs = waits;
  batch.signalSyncs = signals;
  impl_->queueWork[2].batches.push_back(batch);
}

QueueSyncPoint MultiQueueScheduler::CreateComputeToGraphicsSync() {
  QueueSyncPoint sync{};
  sync.signalSemaphore = impl_->computeToGraphicsSem;
  return sync;
}

QueueSyncPoint MultiQueueScheduler::CreateCopyToGraphicsSync() {
  QueueSyncPoint sync{};
  sync.signalSemaphore = impl_->copyToGraphicsSem;
  return sync;
}

QueueSyncPoint MultiQueueScheduler::CreateCopyToComputeSync() {
  QueueSyncPoint sync{};
  sync.signalSemaphore = impl_->copyToComputeSem;
  return sync;
}

void MultiQueueScheduler::Execute() {
  // Sort queues by priority (higher first)
  std::array<size_t, 3> order = {0, 1, 2};
  std::sort(order.begin(), order.end(), [this](size_t a, size_t b) {
    return impl_->queueWork[a].priority > impl_->queueWork[b].priority;
  });

  // Execute in priority order
  for (size_t idx : order) {
    auto& work = impl_->queueWork[idx];
    for (auto& batch : work.batches) {
      impl_->submitCtx.SubmitBatch(batch);
    }
    work.batches.clear();
  }
}

void MultiQueueScheduler::WaitAll() {
  impl_->submitCtx.WaitAllIdle();
}

void MultiQueueScheduler::NextFrame() {
  impl_->submitCtx.AdvanceFrame();
}

uint32_t MultiQueueScheduler::GetPendingWorkCount(QueueId queue) const {
  size_t idx = static_cast<size_t>(queue);
  if (idx < impl_->queueWork.size()) {
    return static_cast<uint32_t>(impl_->queueWork[idx].batches.size());
  }
  return 0;
}

// === Free Functions ===

SubmitContext* CreateSubmitContext() {
  return new SubmitContext();
}

void DestroySubmitContext(SubmitContext* ctx) {
  delete ctx;
}

MultiQueueScheduler* CreateMultiQueueScheduler() {
  return new MultiQueueScheduler();
}

void DestroyMultiQueueScheduler(MultiQueueScheduler* scheduler) {
  delete scheduler;
}

}  // namespace te::pipelinecore
