/**
 * @file command_list.cpp
 * @brief RHI command list implementation (stub).
 */
#include "te/rhi/command_list.hpp"

namespace te {
namespace rhi {

namespace {

bool g_recording{false};

}  // namespace

void Begin(ICommandList* cmd) {
  if (cmd) g_recording = true;
}

void End(ICommandList* cmd) {
  if (cmd) g_recording = false;
}

void Submit(ICommandList* cmd, IQueue* queue) {
  (void)cmd;
  (void)queue;
}

}  // namespace rhi
}  // namespace te
