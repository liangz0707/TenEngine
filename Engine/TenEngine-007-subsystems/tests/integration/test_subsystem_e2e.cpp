/**
 * @file test_subsystem_e2e.cpp
 * @brief E2E: Register -> Lifecycle -> GetSubsystem -> ShutdownAll -> GetSubsystem nullptr.
 */
#include "te/subsystems/registry.hpp"
#include "te/subsystems/lifecycle.hpp"
#include "te/subsystems/subsystem.hpp"
#include "te/subsystems/descriptor.hpp"

#include <cassert>
#include <memory>

namespace {

class DemoSubsystem : public te::subsystems::ISubsystem {
public:
    bool started = false;
    void Initialize() override {}
    void Start() override { started = true; }
    void Stop() override { started = false; }
    void Shutdown() override {}
};

int run_tests() {
    using namespace te::subsystems;

    Registry reg;

    SubsystemDescriptor desc{};
    desc.typeInfo = &typeid(DemoSubsystem);
    desc.dependencies = nullptr;
    desc.dependencyCount = 0;
    desc.priority = 0;
    desc.platformFilter = 0;

    assert(Registry::Register(desc, std::make_unique<DemoSubsystem>()) && "Register");
    assert(Lifecycle::InitializeAll(reg) && "InitializeAll");
    Lifecycle::StartAll(reg);

    DemoSubsystem* p = Registry::GetSubsystem<DemoSubsystem>();
    assert(p != nullptr && p->started && "GetSubsystem returns started instance");

    Lifecycle::StopAll(reg);
    Lifecycle::ShutdownAll(reg);

    p = Registry::GetSubsystem<DemoSubsystem>();
    assert(p == nullptr && "GetSubsystem after ShutdownAll returns nullptr");

    return 0;
}

}  // namespace

int main() {
    return run_tests();
}
