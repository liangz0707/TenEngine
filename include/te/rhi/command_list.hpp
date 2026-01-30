/**
 * @file command_list.hpp
 * @brief RHI command list (contract: specs/_contracts/008-rhi-public-api.md section 4).
 */
#ifndef TE_RHI_COMMAND_LIST_HPP
#define TE_RHI_COMMAND_LIST_HPP

#include "te/rhi/types.hpp"

namespace te {
namespace rhi {

struct IQueue;

/** Command buffer; valid for single recording cycle; allocated by IDevice. */
struct ICommandList {
  virtual ~ICommandList() = default;
  virtual void Draw(uint32_t vertex_count, uint32_t instance_count = 1, uint32_t first_vertex = 0, uint32_t first_instance = 0) = 0;
  virtual void Dispatch(uint32_t x, uint32_t y = 1, uint32_t z = 1) = 0;
  virtual void Copy(void const* src, void* dst, size_t size) = 0;
  virtual void ResourceBarrier() = 0;
};

void Begin(ICommandList* cmd);
void End(ICommandList* cmd);
void Submit(ICommandList* cmd, IQueue* queue);

}  // namespace rhi
}  // namespace te

#endif  // TE_RHI_COMMAND_LIST_HPP
