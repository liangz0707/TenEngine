/**
 * @file test_discovery.cpp
 * @brief Unit tests for Discovery: ScanPlugins (no-op), RegisterFromPlugin with invalid handle.
 */
#include "te/subsystems/discovery.hpp"
#include "te/subsystems/registry.hpp"

#include <cassert>

namespace {

int run_tests() {
    using namespace te::subsystems;

    Registry reg;

    bool scanOk = Discovery::ScanPlugins(reg);
    assert(scanOk && "ScanPlugins should return true (no-op)");

    bool plugOk = Discovery::RegisterFromPlugin(reg, nullptr);
    assert(!plugOk && "RegisterFromPlugin with nullptr should return false");

    return 0;
}

}  // namespace

int main() {
    return run_tests();
}
