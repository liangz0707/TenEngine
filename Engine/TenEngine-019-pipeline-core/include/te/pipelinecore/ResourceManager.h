/**
 * @file ResourceManager.h
 * @brief 019-PipelineCore: Transient resource management for RDG-style frame graphs.
 *
 * Provides resource lifetime management inspired by Unreal RDG and Unity RenderGraph:
 * - Transient resource allocation (frame-local textures/buffers)
 * - Automatic lifetime tracking based on pass dependencies
 * - Resource barrier building for state transitions
 * - Frame-pooled memory management (simplified, no aliasing for now)
 */

#pragma once

#include <te/pipelinecore/Config.h>
#include <te/rendercore/types.hpp>
#include <te/rhi/types.hpp>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>
#include <unordered_map>

namespace te::rhi {
struct IDevice;
struct IBuffer;
struct ITexture;
struct ICommandList;
}

namespace te::pipelinecore {

/// Transient resource types
enum class TransientResourceType : uint8_t {
  Texture = 0,
  Buffer = 1,
};

/// Transient texture descriptor
struct TransientTextureDesc {
  uint32_t width{0};
  uint32_t height{0};
  uint32_t depth{1};
  uint32_t format{0};       // Backend-specific format
  uint32_t mipLevels{1};
  uint32_t arrayLayers{1};
  uint32_t sampleCount{1};
  rhi::ResourceState initialState{rhi::ResourceState::Common};
  char const* debugName{nullptr};
};

/// Transient buffer descriptor
struct TransientBufferDesc {
  size_t size{0};
  uint32_t usage{0};        // BufferUsage bitmask
  rhi::ResourceState initialState{rhi::ResourceState::Common};
  char const* debugName{nullptr};
};

/// Internal handle for transient resources
struct TransientResourceHandle {
  uint64_t id{0};
  TransientResourceType type{TransientResourceType::Texture};

  bool IsValid() const { return id != 0; }
  bool IsTexture() const { return type == TransientResourceType::Texture; }
  bool IsBuffer() const { return type == TransientResourceType::Buffer; }

  static TransientResourceHandle Invalid() { return {0, TransientResourceType::Texture}; }
};

/// Resource lifetime info tracked by the manager
struct ResourceLifetimeInfo {
  TransientResourceHandle handle;
  uint32_t firstUsePass{0xFFFFFFFF};     // First pass that uses this resource
  uint32_t lastUsePass{0};               // Last pass that uses this resource
  uint32_t createPass{0xFFFFFFFF};       // Pass where resource is created
  uint32_t releasePass{0xFFFFFFFF};      // Pass after which resource can be released
  bool isUsed{false};
};

/// Barrier to be inserted between passes
struct ResourceBarrier {
  TransientResourceHandle resource;
  rhi::ResourceState srcState;
  rhi::ResourceState dstState;
  uint32_t beforePass{0};                // Insert before this pass
};

/// Callback for resource creation
using CreateTextureCallback = std::function<rhi::ITexture*(TransientTextureDesc const&)>;
using CreateBufferCallback = std::function<rhi::IBuffer*(TransientBufferDesc const&)>;

/**
 * @brief TransientResourcePool manages frame-local resources for the render graph.
 *
 * Resources are allocated on-demand and released at frame end. The pool tracks
 * resource lifetimes based on pass dependencies and generates appropriate barriers.
 *
 * Usage:
 * 1. BeginFrame() - Reset pool for new frame
 * 2. DeclareTransientTexture/Buffer() - Declare resources needed
 * 3. MarkResourceRead/Write() - Track usage per pass
 * 4. Compile() - Calculate lifetimes and barriers
 * 5. GetOrCreateResource() - Get actual RHI resource
 * 6. InsertBarriersForPass() - Insert barriers before pass execution
 * 7. EndFrame() - Release all resources
 */
class TransientResourcePool {
public:
  TransientResourcePool();
  ~TransientResourcePool();

  TransientResourcePool(TransientResourcePool const&) = delete;
  TransientResourcePool& operator=(TransientResourcePool const&) = delete;
  TransientResourcePool(TransientResourcePool&&) noexcept;
  TransientResourcePool& operator=(TransientResourcePool&&) noexcept;

  /// Set device for resource creation
  void SetDevice(rhi::IDevice* device);

  /// Set custom create callbacks (optional, default uses IDevice directly)
  void SetCreateCallbacks(CreateTextureCallback textureCb, CreateBufferCallback bufferCb);

  // === Frame Lifecycle ===

  /// Begin a new frame: reset pool state, prepare for new allocations
  void BeginFrame();

  /// End frame: release all transient resources
  void EndFrame();

  // === Resource Declaration ===

  /// Declare a transient texture; returns handle for reference
  TransientResourceHandle DeclareTransientTexture(TransientTextureDesc const& desc);

  /// Declare a transient buffer; returns handle for reference
  TransientResourceHandle DeclareTransientBuffer(TransientBufferDesc const& desc);

  /// Get descriptor for a texture handle
  TransientTextureDesc const* GetTextureDesc(TransientResourceHandle handle) const;

  /// Get descriptor for a buffer handle
  TransientBufferDesc const* GetBufferDesc(TransientResourceHandle handle) const;

  // === Usage Tracking ===

  /// Mark resource as read by a pass (updates lifetime)
  void MarkResourceRead(TransientResourceHandle handle, uint32_t passIndex);

  /// Mark resource as written by a pass (updates lifetime)
  void MarkResourceWrite(TransientResourceHandle handle, uint32_t passIndex);

  /// Set explicit release point (resource can be freed after this pass)
  void ReleaseAfterPass(TransientResourceHandle handle, uint32_t passIndex);

  // === Compilation ===

  /// Compile: calculate lifetimes and generate barriers
  /// Call after all passes have declared their resource usage
  void Compile();

  /// Get barriers to insert before a pass
  std::vector<ResourceBarrier> const& GetBarriersForPass(uint32_t passIndex) const;

  /// Get all barriers in execution order
  std::vector<ResourceBarrier> const& GetAllBarriers() const;

  // === Resource Access ===

  /// Get or create the actual RHI resource for a handle
  /// Creates on first access, returns cached on subsequent
  rhi::ITexture* GetOrCreateTexture(TransientResourceHandle handle);
  rhi::IBuffer* GetOrCreateBuffer(TransientResourceHandle handle);

  /// Get existing RHI resource (returns nullptr if not yet created)
  rhi::ITexture* GetTexture(TransientResourceHandle handle) const;
  rhi::IBuffer* GetBuffer(TransientResourceHandle handle) const;

  /// Check if resource has been created
  bool IsResourceCreated(TransientResourceHandle handle) const;

  // === Barrier Execution ===

  /// Insert barriers for a pass into command list
  void InsertBarriersForPass(uint32_t passIndex, rhi::ICommandList* cmd);

  // === Info ===

  /// Get lifetime info for a resource
  ResourceLifetimeInfo const* GetLifetimeInfo(TransientResourceHandle handle) const;

  /// Get total allocated texture count this frame
  size_t GetAllocatedTextureCount() const;

  /// Get total allocated buffer count this frame
  size_t GetAllocatedBufferSize() const;

  /// Get total memory used (approximate, for profiling)
  size_t GetTotalMemoryUsed() const;

  /// Set default dimensions (used when attachment width/height is 0)
  void SetDefaultDimensions(uint32_t width, uint32_t height);

  /// Create transient texture from PassAttachmentDesc
  /// Uses default dimensions if desc.width/height is 0
  rhi::ITexture* CreateTextureFromAttachment(
      uint32_t width, 
      uint32_t height,
      uint32_t format,
      rhi::ResourceState initialState = rhi::ResourceState::RenderTarget);

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

/**
 * @brief ResourceBarrierBuilder helps construct barrier sequences.
 *
 * Collects barrier requests and produces optimized barrier lists.
 */
class ResourceBarrierBuilder {
public:
  ResourceBarrierBuilder();
  ~ResourceBarrierBuilder();

  /// Add a texture transition
  void AddTextureTransition(
    TransientResourceHandle handle,
    rhi::ResourceState srcState,
    rhi::ResourceState dstState,
    uint32_t beforePass);

  /// Add a buffer transition
  void AddBufferTransition(
    TransientResourceHandle handle,
    rhi::ResourceState srcState,
    rhi::ResourceState dstState,
    uint32_t beforePass);

  /// Build final barrier list (sorts and deduplicates)
  std::vector<ResourceBarrier> Build();

  /// Clear all pending barriers
  void Clear();

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

// === Free Functions ===

/// Create a transient resource pool
TransientResourcePool* CreateTransientResourcePool();

/// Destroy a transient resource pool
void DestroyTransientResourcePool(TransientResourcePool* pool);

/// Create a barrier builder
ResourceBarrierBuilder* CreateResourceBarrierBuilder();

/// Destroy a barrier builder
void DestroyResourceBarrierBuilder(ResourceBarrierBuilder* builder);

}  // namespace te::pipelinecore
