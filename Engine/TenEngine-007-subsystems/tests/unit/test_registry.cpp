/**
 * @file test_registry.cpp
 * @brief Unit tests for Registry: Register, GetSubsystem, Unregister, duplicate reject.
 */
#include "te/subsystems/registry.hpp"
#include "te/subsystems/subsystem.hpp"
#include "te/subsystems/descriptor.hpp"

#include <cassert>
#include <memory>

namespace {

class TestSubsystem : public te::subsystems::ISubsystem {
public:
    TestSubsystem() : m_desc{} {
        m_desc.typeInfo = &typeid(TestSubsystem);
        m_desc.name = "TestSubsystem";
        m_desc.version = nullptr;
        m_desc.dependencies = nullptr;
        m_desc.dependencyCount = 0;
        m_desc.priority = 0;
        m_desc.platformFilter = 0;
        m_desc.configData = nullptr;
    }
    
    bool Initialize() override { return true; }
    void Start() override {}
    void Stop() override {}
    void Shutdown() override {}
    
    te::subsystems::SubsystemDescriptor const& GetDescriptor() const override { return m_desc; }
    char const* GetName() const override { return m_desc.name; }
    
private:
    te::subsystems::SubsystemDescriptor m_desc;
};

int run_tests() {
    using namespace te::subsystems;

    SubsystemDescriptor desc{};
    desc.typeInfo = &typeid(TestSubsystem);
    desc.name = "TestSubsystem";
    desc.version = nullptr;
    desc.dependencies = nullptr;
    desc.dependencyCount = 0;
    desc.priority = 0;
    desc.platformFilter = 0;
    desc.configData = nullptr;

    // Register one, query
    auto testSubsystem = std::make_unique<TestSubsystem>();
    bool ok = Registry::Register(desc, std::unique_ptr<ISubsystem>(testSubsystem.release()));
    assert(ok && "Register should succeed");

    TestSubsystem* p = Registry::GetSubsystem<TestSubsystem>();
    assert(p != nullptr && "GetSubsystem should return instance");

    // Test GetSubsystemByName
    ISubsystem* pByName = Registry::GetInstance().GetSubsystemByName("TestSubsystem");
    assert(pByName != nullptr && "GetSubsystemByName should return instance");
    assert(pByName == p && "GetSubsystemByName should return same instance");

    // Test GetAllSubsystems
    auto all = Registry::GetInstance().GetAllSubsystems();
    assert(all.size() == 1 && "GetAllSubsystems should return one subsystem");

    // Duplicate returns false
    auto testSubsystem2 = std::make_unique<TestSubsystem>();
    bool dup = Registry::Register(desc, std::unique_ptr<ISubsystem>(testSubsystem2.release()));
    assert(!dup && "Duplicate Register should return false");

    p = Registry::GetSubsystem<TestSubsystem>();
    assert(p != nullptr && "GetSubsystem still valid after duplicate attempt");

    // Unregister
    Registry::Unregister(desc.typeInfo);
    p = Registry::GetSubsystem<TestSubsystem>();
    assert(p == nullptr && "GetSubsystem should return nullptr after Unregister");

    return 0;
}

}  // namespace

int main() {
    return run_tests();
}
