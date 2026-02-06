/** @file command_list.cpp
 *  Command list free functions: Begin, End, Submit.
 */
#include <te/rhi/command_list.hpp>
#include <te/rhi/queue.hpp>
#include <te/rhi/sync.hpp>

namespace te {
namespace rhi {

void Begin(ICommandList* cmd) {
  if (cmd) cmd->Begin();
}

void End(ICommandList* cmd) {
  if (cmd) cmd->End();
}

void Submit(ICommandList* cmd, IQueue* queue) {
  if (cmd && queue) queue->Submit(cmd, nullptr, nullptr, nullptr);
}

void Submit(ICommandList* cmd, IQueue* queue, IFence* signalFence,
            ISemaphore* waitSem, ISemaphore* signalSem) {
  if (cmd && queue) queue->Submit(cmd, signalFence, waitSem, signalSem);
}

}  // namespace rhi
}  // namespace te
