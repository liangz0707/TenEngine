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
    void Initialize() override {}
    void Start() override {}
    void Stop() override {}
    void Shutdown() override {}
};

int run_tests() {
    using namespace te::subsystems;

    SubsystemDescriptor desc{};
    desc.typeInfo = &typeid(TestSubsystem);
    desc.dependencies = nullptr;
    desc.dependencyCount = 0;
    desc.priority = 0;
    desc.platformFilter = 0;

    // Register one, query
    bool ok = Registry::Register(desc, std::make_unique<TestSubsystem>());
    assert(ok && "Register should succeed");

    TestSubsystem* p = Registry::GetSubsystem<TestSubsystem>();
    assert(p != nullptr && "GetSubsystem should return instance");

    // Duplicate returns false
    bool dup = Registry::Register(desc, std::make_unique<TestSubsystem>());
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
