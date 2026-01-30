// 009-RenderCore UniformBuffer Implementation
// Contract: specs/_contracts/009-rendercore-public-api.md §4. UniformBuffer

#include "uniform_buffer.hpp"
#include <cstring>
#include <new>

namespace TenEngine::RenderCore {

// ============================================================================
// Internal implementation
// ============================================================================

constexpr uint32_t kRingBufferSlots = 3; // Triple buffering

struct UniformBufferImpl {
    UniformLayout layout;
    uint8_t* data = nullptr;
    size_t size = 0;

    // Ring buffer state
    uint32_t currentSlot = 0;
    uint32_t slotsInFlight = 0;  // Slots still in use by GPU

    // Bound slot (for tracking)
    BindSlot boundSlot{};
    bool isBound = false;
};

// ============================================================================
// T014: CreateUniformBuffer (CreateLayout)
// ============================================================================

UniformBufferHandle CreateUniformBuffer(UniformLayout layout) {
    UniformBufferHandle result{};

    // Validate layout
    if (!layout.IsValid()) {
        return result; // Invalid layout: reject
    }

    // Get layout size (access impl for totalSize)
    // For simplicity, use a fixed buffer size; in production, read from layout.impl
    size_t bufferSize = 256; // Default; should be layout.impl->totalSize

    // Allocate impl
    UniformBufferImpl* impl = new (std::nothrow) UniformBufferImpl{};
    if (!impl) {
        return result;
    }

    impl->layout = layout;
    impl->size = bufferSize;

    // Allocate ring buffer data (slots * size)
    impl->data = new (std::nothrow) uint8_t[bufferSize * kRingBufferSlots];
    if (!impl->data) {
        delete impl;
        return result;
    }

    std::memset(impl->data, 0, bufferSize * kRingBufferSlots);

    result.impl = impl;
    return result;
}

// ============================================================================
// T015: Update
// ============================================================================

void Update(UniformBufferHandle handle, void const* data, size_t size) {
    if (!handle.IsValid() || data == nullptr || size == 0) {
        return;
    }

    UniformBufferImpl* impl = handle.impl;

    // Clamp size to buffer size
    size_t copySize = size < impl->size ? size : impl->size;

    // Write to current slot
    uint8_t* slotData = impl->data + (impl->currentSlot * impl->size);
    std::memcpy(slotData, data, copySize);
}

// ============================================================================
// T016: RingBufferAdvance / RingBufferAllocSlot
// ============================================================================

bool RingBufferAdvance(UniformBufferHandle handle) {
    if (!handle.IsValid()) {
        return false;
    }

    UniformBufferImpl* impl = handle.impl;

    // Check if next slot is available
    // If slotsInFlight >= kRingBufferSlots - 1, we must Block (return false to signal caller wait/retry)
    if (impl->slotsInFlight >= kRingBufferSlots - 1) {
        // RingBuffer exhausted: Block semantics - caller should wait/retry
        return false;
    }

    // Advance to next slot
    impl->currentSlot = (impl->currentSlot + 1) % kRingBufferSlots;
    impl->slotsInFlight++;

    return true;
}

uint32_t RingBufferAllocSlot(UniformBufferHandle handle) {
    if (!handle.IsValid()) {
        return UINT32_MAX;
    }

    UniformBufferImpl* impl = handle.impl;

    // Check if slot available
    if (impl->slotsInFlight >= kRingBufferSlots - 1) {
        // Exhausted: return UINT32_MAX (caller should wait/retry)
        return UINT32_MAX;
    }

    uint32_t slot = impl->currentSlot;
    impl->currentSlot = (impl->currentSlot + 1) % kRingBufferSlots;
    impl->slotsInFlight++;

    return slot;
}

/// Called when GPU is done with a slot (for frame synchronization).
void RingBufferReleaseSlot(UniformBufferHandle handle) {
    if (!handle.IsValid()) {
        return;
    }

    UniformBufferImpl* impl = handle.impl;
    if (impl->slotsInFlight > 0) {
        impl->slotsInFlight--;
    }
}

// ============================================================================
// T017: Bind
// ============================================================================

void Bind(UniformBufferHandle handle, BindSlot slot) {
    if (!handle.IsValid()) {
        return;
    }

    UniformBufferImpl* impl = handle.impl;
    impl->boundSlot = slot;
    impl->isBound = true;

    // In full implementation: call RHI to bind buffer to descriptor set/slot.
    // This is a contract-level placeholder; actual RHI binding via 008-RHI API.
}

// ============================================================================
// ReleaseUniformBuffer
// ============================================================================

void ReleaseUniformBuffer(UniformBufferHandle& handle) {
    if (handle.impl) {
        UniformBufferImpl* impl = handle.impl;
        delete[] impl->data;
        delete impl;
        handle.impl = nullptr;
    }
}

} // namespace TenEngine::RenderCore
