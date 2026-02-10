/** @file device_vulkan.cpp
 *  Vulkan backend: CreateDeviceVulkan, DestroyDeviceVulkan, DeviceVulkan, QueueVulkan,
 *  CommandListVulkan, FenceVulkan (T027).
 */
#if defined(TE_RHI_VULKAN)

#include <te/rhi/backend_vulkan.hpp>
#include <te/rhi/command_list.hpp>
#include <te/rhi/device.hpp>
#include <te/rhi/pso.hpp>
#include <te/rhi/queue.hpp>
#include <te/rhi/resources.hpp>
#include <te/rhi/swapchain.hpp>
#include <te/rhi/sync.hpp>
#include <te/rhi/types.hpp>
#include <volk.h>
#include <cstddef>
#include <cstring>
#include <vector>

namespace te {
namespace rhi {

namespace {

struct FenceVulkan final : IFence {
  VkDevice device = VK_NULL_HANDLE;
  VkFence fence = VK_NULL_HANDLE;
  void Wait() override {
    if (fence != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
      vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);
  }
  void Signal() override { (void)0; }
  void Reset() override {
    if (fence != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
      vkResetFences(device, 1, &fence);
  }
  ~FenceVulkan() override {
    if (fence != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
      vkDestroyFence(device, fence, nullptr);
  }
};

struct BufferVulkan final : IBuffer {
  VkDevice device = VK_NULL_HANDLE;
  VkBuffer buffer = VK_NULL_HANDLE;
  VkDeviceMemory memory = VK_NULL_HANDLE;
  ~BufferVulkan() override {
    if (buffer != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
      vkDestroyBuffer(device, buffer, nullptr);
    if (memory != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
      vkFreeMemory(device, memory, nullptr);
  }
};

struct CommandListVulkan final : ICommandList {
  VkDevice device = VK_NULL_HANDLE;
  VkCommandPool pool = VK_NULL_HANDLE;
  VkCommandBuffer cmd = VK_NULL_HANDLE;
  VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
  VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
  bool recording = false;

  void Begin() override {
    if (cmd == VK_NULL_HANDLE || recording) return;
    VkCommandBufferBeginInfo bi = {};
    bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmd, &bi);
    recording = true;
  }
  void End() override {
    if (cmd == VK_NULL_HANDLE || !recording) return;
    vkEndCommandBuffer(cmd);
    recording = false;
  }
  void Draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) override {
    if (cmd != VK_NULL_HANDLE && recording)
      vkCmdDraw(cmd, vertex_count, instance_count, first_vertex, first_instance);
  }
  void DrawIndexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset, uint32_t first_instance) override {
    if (cmd != VK_NULL_HANDLE && recording)
      vkCmdDrawIndexed(cmd, index_count, instance_count, first_index, vertex_offset, first_instance);
  }
  void SetViewport(uint32_t first, uint32_t count, Viewport const* viewports) override {
    if (cmd == VK_NULL_HANDLE || !viewports || count == 0) return;
    std::vector<VkViewport> v(count);
    for (uint32_t i = 0; i < count; ++i) {
      v[i].x = viewports[i].x;
      v[i].y = viewports[i].y;
      v[i].width = viewports[i].width;
      v[i].height = viewports[i].height;
      v[i].minDepth = viewports[i].minDepth;
      v[i].maxDepth = viewports[i].maxDepth;
    }
    vkCmdSetViewport(cmd, first, count, v.data());
  }
  void SetScissor(uint32_t first, uint32_t count, ScissorRect const* scissors) override {
    if (cmd == VK_NULL_HANDLE || !scissors || count == 0) return;
    std::vector<VkRect2D> s(count);
    for (uint32_t i = 0; i < count; ++i) {
      s[i].offset = { scissors[i].x, scissors[i].y };
      s[i].extent = { scissors[i].width, scissors[i].height };
    }
    vkCmdSetScissor(cmd, first, count, s.data());
  }
  void SetUniformBuffer(uint32_t slot, IBuffer* buffer, size_t offset) override {
    if (cmd == VK_NULL_HANDLE || !recording || descriptorSet == VK_NULL_HANDLE || pipelineLayout == VK_NULL_HANDLE) return;
    if (!buffer) return;
    BufferVulkan* b = static_cast<BufferVulkan*>(buffer);
    if (b->buffer == VK_NULL_HANDLE) return;
    VkDescriptorBufferInfo binfo = {};
    binfo.buffer = b->buffer;
    binfo.offset = offset;
    binfo.range = VK_WHOLE_SIZE;
    VkWriteDescriptorSet write = {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = descriptorSet;
    write.dstBinding = slot;
    write.dstArrayElement = 0;
    write.descriptorCount = 1;
    write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write.pBufferInfo = &binfo;
    vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
  }
  void SetVertexBuffer(uint32_t slot, IBuffer* buffer, size_t offset, uint32_t stride) override {
    (void)stride;  /* Vulkan stride from pipeline vertex input */
    if (cmd == VK_NULL_HANDLE || !recording) return;
    if (!buffer) return;
    BufferVulkan* b = static_cast<BufferVulkan*>(buffer);
    if (b->buffer == VK_NULL_HANDLE) return;
    VkDeviceSize off = static_cast<VkDeviceSize>(offset);
    vkCmdBindVertexBuffers(cmd, slot, 1, &b->buffer, &off);
  }
  void SetIndexBuffer(IBuffer* buffer, size_t offset, uint32_t indexFormat) override {
    if (cmd == VK_NULL_HANDLE || !recording) return;
    if (!buffer) return;
    BufferVulkan* b = static_cast<BufferVulkan*>(buffer);
    if (b->buffer == VK_NULL_HANDLE) return;
    VkIndexType idxType = (indexFormat == 1) ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16;
    vkCmdBindIndexBuffer(cmd, b->buffer, static_cast<VkDeviceSize>(offset), idxType);
  }
  void SetGraphicsPSO(IPSO* pso) override {
    (void)pso;
    /* Vulkan: PSOVulkan currently only has shader modules; vkCmdBindPipeline would need VkPipeline from CreateGraphicsPSO. */
  }
  void BeginRenderPass(RenderPassDesc const& desc) override {
    (void)desc;
    if (cmd == VK_NULL_HANDLE || !recording) return;
    if (desc.colorAttachmentCount == 0) return;
    /* Requires VkRenderPass/VkFramebuffer from CreateRenderPass; skip real call when not set. */
  }
  void EndRenderPass() override {
    if (cmd == VK_NULL_HANDLE || !recording) return;
    /* Paired with BeginRenderPass; no vkCmdEndRenderPass if Begin was skipped. */
  }
  void BeginOcclusionQuery(uint32_t queryIndex) override { (void)queryIndex; }
  void EndOcclusionQuery(uint32_t queryIndex) override { (void)queryIndex; }
  void CopyBuffer(IBuffer* src, size_t srcOffset, IBuffer* dst, size_t dstOffset, size_t size) override {
    (void)src;(void)srcOffset;(void)dst;(void)dstOffset;(void)size;
    if (cmd == VK_NULL_HANDLE || !recording) return;
  }
  void CopyBufferToTexture(IBuffer* src, size_t srcOffset, ITexture* dst, TextureRegion const& dstRegion) override {
    (void)src;(void)srcOffset;(void)dst;(void)dstRegion;
  }
  void CopyTextureToBuffer(ITexture* src, TextureRegion const& srcRegion, IBuffer* dst, size_t dstOffset) override {
    (void)src;(void)srcRegion;(void)dst;(void)dstOffset;
  }
  /** Raytracing unsupported on Vulkan backend (no VK_KHR_ray_tracing in this build). */
  void BuildAccelerationStructure(RaytracingAccelerationStructureDesc const& desc, IBuffer* scratch, IBuffer* result) override {
    (void)desc;(void)scratch;(void)result;
  }
  void DispatchRays(DispatchRaysDesc const& desc) override { (void)desc; }
  void Dispatch(uint32_t x, uint32_t y, uint32_t z) override {
    if (cmd != VK_NULL_HANDLE && recording)
      vkCmdDispatch(cmd, x, y, z);
  }
  void Copy(void const* src, void* dst, size_t size) override {
    (void)src;(void)dst;(void)size;
  }
  void ResourceBarrier(uint32_t bufferBarrierCount, BufferBarrier const* bufferBarriers,
                       uint32_t textureBarrierCount, TextureBarrier const* textureBarriers) override {
    if (cmd == VK_NULL_HANDLE || !recording) return;
    if (bufferBarrierCount == 0 && textureBarrierCount == 0) return;
    std::vector<VkMemoryBarrier> memBarriers;
    std::vector<VkBufferMemoryBarrier> bufBarriers;
    std::vector<VkImageMemoryBarrier> imgBarriers;
    for (uint32_t i = 0; i < bufferBarrierCount && bufferBarriers; ++i) {
      VkBufferMemoryBarrier b = {};
      b.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
      b.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
      b.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
      b.buffer = VK_NULL_HANDLE;
      b.offset = bufferBarriers[i].offset;
      b.size = bufferBarriers[i].size;
      bufBarriers.push_back(b);
    }
    for (uint32_t i = 0; i < textureBarrierCount && textureBarriers; ++i) {
      VkImageMemoryBarrier im = {};
      im.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
      im.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
      im.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
      im.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
      im.newLayout = VK_IMAGE_LAYOUT_GENERAL;
      im.image = VK_NULL_HANDLE;
      im.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, textureBarriers[i].mipLevel, 1, textureBarriers[i].arrayLayer, 1 };
      imgBarriers.push_back(im);
    }
    if (!bufBarriers.empty() || !imgBarriers.empty())
      vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                           0, 0, nullptr, (uint32_t)bufBarriers.size(), bufBarriers.data(),
                           (uint32_t)imgBarriers.size(), imgBarriers.data());
  }
  ~CommandListVulkan() override {
    if (cmd != VK_NULL_HANDLE && device != VK_NULL_HANDLE && pool != VK_NULL_HANDLE)
      vkFreeCommandBuffers(device, pool, 1, &cmd);
  }
};

struct TextureVulkan final : ITexture {
  VkDevice device = VK_NULL_HANDLE;
  VkImage image = VK_NULL_HANDLE;
  VkDeviceMemory memory = VK_NULL_HANDLE;
  ~TextureVulkan() override {
    if (image != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
      vkDestroyImage(device, image, nullptr);
    if (memory != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
      vkFreeMemory(device, memory, nullptr);
  }
};

struct SamplerVulkan final : ISampler {
  VkDevice device = VK_NULL_HANDLE;
  VkSampler sampler = VK_NULL_HANDLE;
  ~SamplerVulkan() override {
    if (sampler != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
      vkDestroySampler(device, sampler, nullptr);
  }
};

struct PSOVulkan final : IPSO {
  VkDevice device = VK_NULL_HANDLE;
  VkShaderModule vertModule = VK_NULL_HANDLE;
  VkShaderModule fragModule = VK_NULL_HANDLE;
  VkShaderModule computeModule = VK_NULL_HANDLE;
  ~PSOVulkan() override {
    if (device == VK_NULL_HANDLE) return;
    if (vertModule != VK_NULL_HANDLE) { vkDestroyShaderModule(device, vertModule, nullptr); vertModule = VK_NULL_HANDLE; }
    if (fragModule != VK_NULL_HANDLE) { vkDestroyShaderModule(device, fragModule, nullptr); fragModule = VK_NULL_HANDLE; }
    if (computeModule != VK_NULL_HANDLE) { vkDestroyShaderModule(device, computeModule, nullptr); computeModule = VK_NULL_HANDLE; }
  }
};

struct SwapChainVulkan final : ISwapChain {
  IDevice* device = nullptr;
  ITexture* backBuffer = nullptr;
  uint32_t width = 0;
  uint32_t height = 0;
  bool Present() override { return true; }
  ITexture* GetCurrentBackBuffer() override { return backBuffer; }
  uint32_t GetCurrentBackBufferIndex() const override { return 0; }
  void Resize(uint32_t w, uint32_t h) override { width = w; height = h; }
  uint32_t GetWidth() const override { return width; }
  uint32_t GetHeight() const override { return height; }
  ~SwapChainVulkan() override {
    if (device && backBuffer) device->DestroyTexture(backBuffer);
  }
};

struct QueueVulkan final : IQueue {
  VkQueue queue = VK_NULL_HANDLE;
  void Submit(ICommandList* cmdList, IFence* signalFence,
              ISemaphore* waitSemaphore, ISemaphore* signalSemaphore) override {
    (void)waitSemaphore;
    (void)signalSemaphore;
    if (queue == VK_NULL_HANDLE || !cmdList) return;
    CommandListVulkan* vkCmd = static_cast<CommandListVulkan*>(cmdList);
    if (vkCmd->cmd == VK_NULL_HANDLE) return;
    VkSubmitInfo si = {};
    si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    si.commandBufferCount = 1;
    si.pCommandBuffers = &vkCmd->cmd;
    VkFence sigFence = VK_NULL_HANDLE;
    if (signalFence) {
      FenceVulkan* f = static_cast<FenceVulkan*>(signalFence);
      sigFence = f->fence;
    }
    vkQueueSubmit(queue, 1, &si, sigFence);
  }
  void WaitIdle() override {
    if (queue != VK_NULL_HANDLE)
      vkQueueWaitIdle(queue);
  }
  ~QueueVulkan() override = default;
};

struct DeviceVulkan final : IDevice {
  VkInstance instance = VK_NULL_HANDLE;
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
  VkDevice   device   = VK_NULL_HANDLE;
  VkQueue    queue    = VK_NULL_HANDLE;
  VkCommandPool commandPool = VK_NULL_HANDLE;
  VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
  VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
  VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
  QueueVulkan* queueWrapper = nullptr;
  DeviceFeatures features{};
  DeviceLimits    limits{};

  Backend GetBackend() const override { return Backend::Vulkan; }
  IQueue* GetQueue(QueueType type, uint32_t index) override {
    (void)type;
    (void)index;
    return queueWrapper;
  }
  DeviceFeatures const& GetFeatures() const override { return features; }
  DeviceLimits const& GetLimits() const override { return limits; }
  ICommandList* CreateCommandList() override {
    if (device == VK_NULL_HANDLE || commandPool == VK_NULL_HANDLE) return nullptr;
    if (descriptorPool == VK_NULL_HANDLE || descriptorSetLayout == VK_NULL_HANDLE || pipelineLayout == VK_NULL_HANDLE)
      return nullptr;
    VkCommandBufferAllocateInfo ai = {};
    ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    ai.commandPool = commandPool;
    ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    ai.commandBufferCount = 1;
    VkCommandBuffer cb = VK_NULL_HANDLE;
    if (vkAllocateCommandBuffers(device, &ai, &cb) != VK_SUCCESS) return nullptr;
    VkDescriptorSetAllocateInfo dsAi = {};
    dsAi.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    dsAi.descriptorPool = descriptorPool;
    dsAi.descriptorSetCount = 1;
    dsAi.pSetLayouts = &descriptorSetLayout;
    VkDescriptorSet ds = VK_NULL_HANDLE;
    if (vkAllocateDescriptorSets(device, &dsAi, &ds) != VK_SUCCESS) {
      vkFreeCommandBuffers(device, commandPool, 1, &cb);
      return nullptr;
    }
    auto* cl = new CommandListVulkan();
    cl->device = device;
    cl->pool = commandPool;
    cl->cmd = cb;
    cl->descriptorSet = ds;
    cl->pipelineLayout = pipelineLayout;
    return cl;
  }
  void DestroyCommandList(ICommandList* cmd) override {
    delete static_cast<CommandListVulkan*>(cmd);
  }
  void UpdateBuffer(IBuffer* buf, size_t offset, void const* data, size_t size) override {
    if (device == VK_NULL_HANDLE || !buf || !data || size == 0) return;
    BufferVulkan* b = static_cast<BufferVulkan*>(buf);
    if (b->buffer == VK_NULL_HANDLE || b->memory == VK_NULL_HANDLE) return;
    void* ptr = nullptr;
    if (vkMapMemory(device, b->memory, offset, size, 0, &ptr) != VK_SUCCESS) return;
    std::memcpy(ptr, data, size);
    vkUnmapMemory(device, b->memory);
  }
  IBuffer* CreateBuffer(BufferDesc const& desc) override {
    if (device == VK_NULL_HANDLE || physicalDevice == VK_NULL_HANDLE || desc.size == 0)
      return nullptr;
    VkBufferCreateInfo bci = {};
    bci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bci.size = desc.size;
    bci.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    if (desc.usage != 0) {
      VkBufferUsageFlags u = 0;
      if (desc.usage & static_cast<uint32_t>(BufferUsage::Vertex)) u |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
      if (desc.usage & static_cast<uint32_t>(BufferUsage::Index)) u |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
      if (desc.usage & static_cast<uint32_t>(BufferUsage::Uniform)) u |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
      if (desc.usage & static_cast<uint32_t>(BufferUsage::Storage)) u |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
      if (desc.usage & static_cast<uint32_t>(BufferUsage::CopySrc)) u |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
      if (desc.usage & static_cast<uint32_t>(BufferUsage::CopyDst)) u |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
      if (u != 0) bci.usage = u;
    }
    VkBuffer buf = VK_NULL_HANDLE;
    if (vkCreateBuffer(device, &bci, nullptr, &buf) != VK_SUCCESS)
      return nullptr;
    VkMemoryRequirements memReq = {};
    vkGetBufferMemoryRequirements(device, buf, &memReq);
    VkPhysicalDeviceMemoryProperties memProps = {};
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProps);
    bool wantHostVisible = (desc.usage & static_cast<uint32_t>(BufferUsage::Uniform)) != 0;
    uint32_t memTypeIndex = (uint32_t)-1;
    for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i) {
      if (!(memReq.memoryTypeBits & (1u << i))) continue;
      VkMemoryPropertyFlags flags = memProps.memoryTypes[i].propertyFlags;
      if (wantHostVisible && (flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) && (flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
        memTypeIndex = i;
        break;
      }
      if (!wantHostVisible && (flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
        memTypeIndex = i;
        break;
      }
    }
    if (memTypeIndex == (uint32_t)-1 && !wantHostVisible) {
      for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i) {
        if ((memReq.memoryTypeBits & (1u << i)) &&
            (memProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
          memTypeIndex = i;
          break;
        }
      }
    }
    if (memTypeIndex == (uint32_t)-1) {
      vkDestroyBuffer(device, buf, nullptr);
      return nullptr;
    }
    VkMemoryAllocateInfo mai = {};
    mai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mai.allocationSize = memReq.size;
    mai.memoryTypeIndex = memTypeIndex;
    VkDeviceMemory mem = VK_NULL_HANDLE;
    if (vkAllocateMemory(device, &mai, nullptr, &mem) != VK_SUCCESS) {
      vkDestroyBuffer(device, buf, nullptr);
      return nullptr;
    }
    if (vkBindBufferMemory(device, buf, mem, 0) != VK_SUCCESS) {
      vkFreeMemory(device, mem, nullptr);
      vkDestroyBuffer(device, buf, nullptr);
      return nullptr;
    }
    auto* b = new BufferVulkan();
    b->device = device;
    b->buffer = buf;
    b->memory = mem;
    return b;
  }
  ITexture* CreateTexture(TextureDesc const& desc) override {
    if (device == VK_NULL_HANDLE || physicalDevice == VK_NULL_HANDLE || desc.width == 0 || desc.height == 0)
      return nullptr;
    VkFormat fmt = (desc.format == 0) ? VK_FORMAT_R8G8B8A8_UNORM : static_cast<VkFormat>(desc.format);
    VkImageCreateInfo ici = {};
    ici.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ici.imageType = desc.depth > 1 ? VK_IMAGE_TYPE_3D : VK_IMAGE_TYPE_2D;
    ici.format = fmt;
    ici.extent = { desc.width, desc.height, desc.depth > 0 ? desc.depth : 1u };
    ici.mipLevels = 1;
    ici.arrayLayers = 1;
    ici.samples = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling = VK_IMAGE_TILING_OPTIMAL;
    ici.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImage img = VK_NULL_HANDLE;
    if (vkCreateImage(device, &ici, nullptr, &img) != VK_SUCCESS)
      return nullptr;
    VkMemoryRequirements memReq = {};
    vkGetImageMemoryRequirements(device, img, &memReq);
    VkPhysicalDeviceMemoryProperties memProps = {};
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProps);
    uint32_t memTypeIndex = (uint32_t)-1;
    for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i) {
      if ((memReq.memoryTypeBits & (1u << i)) &&
          (memProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
        memTypeIndex = i;
        break;
      }
    }
    if (memTypeIndex == (uint32_t)-1) {
      vkDestroyImage(device, img, nullptr);
      return nullptr;
    }
    VkMemoryAllocateInfo mai = {};
    mai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mai.allocationSize = memReq.size;
    mai.memoryTypeIndex = memTypeIndex;
    VkDeviceMemory mem = VK_NULL_HANDLE;
    if (vkAllocateMemory(device, &mai, nullptr, &mem) != VK_SUCCESS) {
      vkDestroyImage(device, img, nullptr);
      return nullptr;
    }
    if (vkBindImageMemory(device, img, mem, 0) != VK_SUCCESS) {
      vkFreeMemory(device, mem, nullptr);
      vkDestroyImage(device, img, nullptr);
      return nullptr;
    }
    auto* t = new TextureVulkan();
    t->device = device;
    t->image = img;
    t->memory = mem;
    return t;
  }
  ISampler* CreateSampler(SamplerDesc const& desc) override {
    if (device == VK_NULL_HANDLE) return nullptr;
    VkSamplerCreateInfo sci = {};
    sci.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sci.magFilter = (desc.filter == 0) ? VK_FILTER_NEAREST : VK_FILTER_LINEAR;
    sci.minFilter = sci.magFilter;
    sci.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    sci.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sci.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sci.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    VkSampler samp = VK_NULL_HANDLE;
    if (vkCreateSampler(device, &sci, nullptr, &samp) != VK_SUCCESS)
      return nullptr;
    auto* s = new SamplerVulkan();
    s->device = device;
    s->sampler = samp;
    return s;
  }
  ViewHandle CreateView(ViewDesc const& desc) override {
    if (device == VK_NULL_HANDLE || !desc.resource) return 0;
    ITexture* tex = static_cast<ITexture*>(desc.resource);
    TextureVulkan* tv = static_cast<TextureVulkan*>(tex);
    if (!tv || tv->image == VK_NULL_HANDLE) return 0;
    VkImageViewCreateInfo ivci = {};
    ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ivci.image = tv->image;
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = VK_FORMAT_R8G8B8A8_UNORM;
    ivci.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    VkImageView iv = VK_NULL_HANDLE;
    if (vkCreateImageView(device, &ivci, nullptr, &iv) != VK_SUCCESS)
      return 0;
    return static_cast<ViewHandle>(reinterpret_cast<uintptr_t>(iv));
  }
  void DestroyBuffer(IBuffer* b) override { delete static_cast<BufferVulkan*>(b); }
  void DestroyTexture(ITexture* t) override { delete static_cast<TextureVulkan*>(t); }
  void DestroySampler(ISampler* s) override { delete static_cast<SamplerVulkan*>(s); }
  IPSO* CreateGraphicsPSO(GraphicsPSODesc const& desc) override {
    if (device == VK_NULL_HANDLE) return nullptr;
    constexpr size_t kMinSpirvSize = 20u;
    bool hasVert = desc.vertex_shader && desc.vertex_shader_size >= kMinSpirvSize;
    bool hasFrag = desc.fragment_shader && desc.fragment_shader_size >= kMinSpirvSize;
    if (!hasVert && !hasFrag) return nullptr;
    auto* p = new PSOVulkan();
    p->device = device;
    if (hasVert) {
      VkShaderModuleCreateInfo smci = {};
      smci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
      smci.codeSize = desc.vertex_shader_size;
      smci.pCode = static_cast<uint32_t const*>(desc.vertex_shader);
      if (vkCreateShaderModule(device, &smci, nullptr, &p->vertModule) != VK_SUCCESS) {
        delete p;
        return nullptr;
      }
    }
    if (hasFrag) {
      VkShaderModuleCreateInfo smci = {};
      smci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
      smci.codeSize = desc.fragment_shader_size;
      smci.pCode = static_cast<uint32_t const*>(desc.fragment_shader);
      if (vkCreateShaderModule(device, &smci, nullptr, &p->fragModule) != VK_SUCCESS) {
        delete p;
        return nullptr;
      }
    }
    return p;
  }
  IPSO* CreateComputePSO(ComputePSODesc const& desc) override {
    if (device == VK_NULL_HANDLE) return nullptr;
    constexpr size_t kMinSpirvSize = 20u;
    if (!desc.compute_shader || desc.compute_shader_size < kMinSpirvSize) return nullptr;
    VkShaderModuleCreateInfo smci = {};
    smci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    smci.codeSize = desc.compute_shader_size;
    smci.pCode = static_cast<uint32_t const*>(desc.compute_shader);
    VkShaderModule mod = VK_NULL_HANDLE;
    if (vkCreateShaderModule(device, &smci, nullptr, &mod) != VK_SUCCESS)
      return nullptr;
    auto* p = new PSOVulkan();
    p->device = device;
    p->computeModule = mod;
    return p;
  }
  void SetShader(IPSO* pso, void const* data, size_t size) override { (void)pso; (void)data; (void)size; }
  void Cache(IPSO* pso) override { (void)pso; }
  void DestroyPSO(IPSO* pso) override { delete static_cast<PSOVulkan*>(pso); }
  IFence* CreateFence(bool initialSignaled) override {
    if (device == VK_NULL_HANDLE) return nullptr;
    VkFenceCreateInfo fci = {};
    fci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fci.flags = initialSignaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0u;
    VkFence vkFence = VK_NULL_HANDLE;
    if (vkCreateFence(device, &fci, nullptr, &vkFence) != VK_SUCCESS) return nullptr;
    auto* fv = new FenceVulkan();
    fv->device = device;
    fv->fence = vkFence;
    return fv;
  }
  ISemaphore* CreateSemaphore() override { return nullptr; }
  void DestroyFence(IFence* f) override { delete static_cast<FenceVulkan*>(f); }
  void DestroySemaphore(ISemaphore* s) override { (void)s; }
  ISwapChain* CreateSwapChain(SwapChainDesc const& desc) override {
    if (device == VK_NULL_HANDLE || desc.width == 0 || desc.height == 0) return nullptr;
    TextureDesc td = {};
    td.width = desc.width;
    td.height = desc.height;
    td.depth = 1;
    td.format = desc.format;
    ITexture* tex = CreateTexture(td);
    if (!tex) return nullptr;
    auto* sc = new SwapChainVulkan();
    sc->device = this;
    sc->backBuffer = tex;
    sc->width = desc.width;
    sc->height = desc.height;
    return sc;
  }
  IDescriptorSetLayout* CreateDescriptorSetLayout(DescriptorSetLayoutDesc const& desc) override { (void)desc; return nullptr; }
  IDescriptorSet* AllocateDescriptorSet(IDescriptorSetLayout* layout) override { (void)layout; return nullptr; }
  void UpdateDescriptorSet(IDescriptorSet* set, DescriptorWrite const* writes, uint32_t writeCount) override { (void)set; (void)writes; (void)writeCount; }
  void DestroyDescriptorSetLayout(IDescriptorSetLayout* layout) override { (void)layout; }
  void DestroyDescriptorSet(IDescriptorSet* set) override { (void)set; }
  ~DeviceVulkan() override {
    delete queueWrapper;
    queueWrapper = nullptr;
    if (descriptorPool != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
      vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    if (pipelineLayout != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
      vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    if (descriptorSetLayout != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
      vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
    if (commandPool != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
      vkDestroyCommandPool(device, commandPool, nullptr);
  }
};

}  // namespace

IDevice* CreateDeviceVulkan() {
  if (volkInitialize() != VK_SUCCESS)
    return nullptr;
  VkInstance instance = VK_NULL_HANDLE;
  VkApplicationInfo appInfo = {};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "te_rhi";
  appInfo.apiVersion = VK_API_VERSION_1_2;
  VkInstanceCreateInfo ici = {};
  ici.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  ici.pApplicationInfo = &appInfo;
  if (vkCreateInstance(&ici, nullptr, &instance) != VK_SUCCESS)
    return nullptr;
  volkLoadInstance(instance);
  uint32_t count = 0;
  vkEnumeratePhysicalDevices(instance, &count, nullptr);
  if (count == 0) {
    vkDestroyInstance(instance, nullptr);
    return nullptr;
  }
  std::vector<VkPhysicalDevice> phys(count);
  vkEnumeratePhysicalDevices(instance, &count, phys.data());
  VkPhysicalDevice physDev = phys[0];
  float qPriorities[] = {1.0f};
  VkDeviceQueueCreateInfo qci = {};
  qci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  qci.queueFamilyIndex = 0;
  qci.queueCount = 1;
  qci.pQueuePriorities = qPriorities;
  VkDeviceCreateInfo dci = {};
  dci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  dci.queueCreateInfoCount = 1;
  dci.pQueueCreateInfos = &qci;
  VkDevice device = VK_NULL_HANDLE;
  if (vkCreateDevice(physDev, &dci, nullptr, &device) != VK_SUCCESS) {
    vkDestroyInstance(instance, nullptr);
    return nullptr;
  }
  volkLoadDevice(device);
  VkQueue queue = VK_NULL_HANDLE;
  vkGetDeviceQueue(device, 0, 0, &queue);
  VkCommandPool commandPool = VK_NULL_HANDLE;
  VkCommandPoolCreateInfo pci = {};
  pci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  pci.queueFamilyIndex = 0;
  pci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  if (vkCreateCommandPool(device, &pci, nullptr, &commandPool) != VK_SUCCESS) {
    vkDestroyDevice(device, nullptr);
    vkDestroyInstance(instance, nullptr);
    return nullptr;
  }
  /* Descriptor set layout: up to 16 uniform buffer bindings (slots 0..15). */
  enum { kMaxUniformSlots = 16 };
  VkDescriptorSetLayoutBinding bindings[kMaxUniformSlots] = {};
  for (uint32_t i = 0; i < kMaxUniformSlots; ++i) {
    bindings[i].binding = i;
    bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[i].descriptorCount = 1;
    bindings[i].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
  }
  VkDescriptorSetLayoutCreateInfo dsLayoutCi = {};
  dsLayoutCi.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  dsLayoutCi.bindingCount = kMaxUniformSlots;
  dsLayoutCi.pBindings = bindings;
  VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
  if (vkCreateDescriptorSetLayout(device, &dsLayoutCi, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
    vkDestroyCommandPool(device, commandPool, nullptr);
    vkDestroyDevice(device, nullptr);
    vkDestroyInstance(instance, nullptr);
    return nullptr;
  }
  VkDescriptorPoolSize poolSizes[] = { { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, kMaxUniformSlots * 64 } };
  VkDescriptorPoolCreateInfo poolCi = {};
  poolCi.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolCi.maxSets = 64;
  poolCi.poolSizeCount = 1;
  poolCi.pPoolSizes = poolSizes;
  VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
  if (vkCreateDescriptorPool(device, &poolCi, nullptr, &descriptorPool) != VK_SUCCESS) {
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
    vkDestroyCommandPool(device, commandPool, nullptr);
    vkDestroyDevice(device, nullptr);
    vkDestroyInstance(instance, nullptr);
    return nullptr;
  }
  VkPipelineLayoutCreateInfo plCi = {};
  plCi.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  plCi.setLayoutCount = 1;
  plCi.pSetLayouts = &descriptorSetLayout;
  VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
  if (vkCreatePipelineLayout(device, &plCi, nullptr, &pipelineLayout) != VK_SUCCESS) {
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
    vkDestroyCommandPool(device, commandPool, nullptr);
    vkDestroyDevice(device, nullptr);
    vkDestroyInstance(instance, nullptr);
    return nullptr;
  }
  auto* d = new DeviceVulkan();
  d->instance = instance;
  d->physicalDevice = physDev;
  d->device = device;
  d->queue = queue;
  d->commandPool = commandPool;
  d->descriptorSetLayout = descriptorSetLayout;
  d->descriptorPool = descriptorPool;
  d->pipelineLayout = pipelineLayout;
  VkPhysicalDeviceProperties props = {};
  vkGetPhysicalDeviceProperties(physDev, &props);
  d->limits.maxBufferSize = 256 * 1024 * 1024ull;
  d->limits.maxTextureDimension2D = props.limits.maxImageDimension2D;
  d->limits.maxTextureDimension3D = props.limits.maxImageDimension3D;
  d->limits.minUniformBufferOffsetAlignment = props.limits.minUniformBufferOffsetAlignment;
  d->features.maxTextureDimension2D = props.limits.maxImageDimension2D;
  d->features.maxTextureDimension3D = props.limits.maxImageDimension3D;
  d->queueWrapper = new QueueVulkan();
  d->queueWrapper->queue = queue;
  return d;
}

void DestroyDeviceVulkan(IDevice* dev) {
  auto* d = static_cast<DeviceVulkan*>(dev);
  if (!d) return;
  if (d->commandPool != VK_NULL_HANDLE && d->device != VK_NULL_HANDLE)
    vkDestroyCommandPool(d->device, d->commandPool, nullptr);
  d->commandPool = VK_NULL_HANDLE;
  if (d->device != VK_NULL_HANDLE)
    vkDestroyDevice(d->device, nullptr);
  if (d->instance != VK_NULL_HANDLE)
    vkDestroyInstance(d->instance, nullptr);
  delete d;
}

}  // namespace rhi
}  // namespace te

#endif  // TE_RHI_VULKAN
