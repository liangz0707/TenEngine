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
    DemoSubsystem() : m_desc{}, started(false) {
        m_desc.typeInfo = &typeid(DemoSubsystem);
        m_desc.name = "DemoSubsystem";
        m_desc.version = nullptr;
        m_desc.dependencies = nullptr;
        m_desc.dependencyCount = 0;
        m_desc.priority = 0;
        m_desc.platformFilter = 0;
        m_desc.configData = nullptr;
    }
    
    bool started;
    
    bool Initialize() override { return true; }
    void Start() override { started = true; }
    void Stop() override { started = false; }
    void Shutdown() override {}
    
    te::subsystems::SubsystemDescriptor const& GetDescriptor() const override { return m_desc; }
    char const* GetName() const override { return m_desc.name; }
    
private:
    te::subsystems::SubsystemDescriptor m_desc;
};

int run_tests() {
    using namespace te::subsystems;

    auto& reg = Registry::GetInstance();

    SubsystemDescriptor desc{};
    desc.typeInfo = &typeid(DemoSubsystem);
    desc.name = "DemoSubsystem";
    desc.version = nullptr;
    desc.dependencies = nullptr;
    desc.dependencyCount = 0;
    desc.priority = 0;
    desc.platformFilter = 0;
    desc.configData = nullptr;

    assert(Registry::Register(desc, std::make_unique<DemoSubsystem>()) && "Register");
    
    auto initResult = Lifecycle::InitializeAll(reg);
    assert(initResult.success && "InitializeAll");
    
    auto startResult = Lifecycle::StartAll(reg);
    assert(startResult.success && "StartAll");

    DemoSubsystem* p = Registry::GetSubsystem<DemoSubsystem>();
    assert(p != nullptr && p->started && "GetSubsystem returns started instance");

    auto stopResult = Lifecycle::StopAll(reg);
    assert(stopResult.success && "StopAll");
    
    Lifecycle::ShutdownAll(reg);

    p = Registry::GetSubsystem<DemoSubsystem>();
    assert(p == nullptr && "GetSubsystem after ShutdownAll returns nullptr");

    return 0;
}

}  // namespace

int main() {
    return run_tests();
}
