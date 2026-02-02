/**
 * @file command_list.cpp
 * @brief RHI command list implementation.
 */
#include "te/rhi/command_list.hpp"
#include "te/rhi/queue.hpp"

namespace te {
namespace rhi {

void Begin(ICommandList* cmd) {
  if (cmd) cmd->Begin();
}

void End(ICommandList* cmd) {
  if (cmd) cmd->End();
}

void Submit(ICommandList* cmd, IQueue* queue) {
  if (cmd && queue)
    queue->Submit(cmd, nullptr, nullptr, nullptr);
}

}  // namespace rhi
}  // namespace te
