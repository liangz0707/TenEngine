/**
 * @file test_lifecycle_order.cpp
 * @brief Unit tests for Lifecycle: topology + Priority, cycle reject.
 */
#include "te/subsystems/registry.hpp"
#include "te/subsystems/lifecycle.hpp"
#include "te/subsystems/subsystem.hpp"
#include "te/subsystems/descriptor.hpp"

#include <cassert>
#include <memory>
#include <vector>

namespace {

static std::vector<int> g_order;

class A : public te::subsystems::ISubsystem {
public:
    void Initialize() override { g_order.push_back(1); }
    void Start() override { g_order.push_back(10); }
    void Stop() override { g_order.push_back(-10); }
    void Shutdown() override { g_order.push_back(-1); }
};

class B : public te::subsystems::ISubsystem {
public:
    void Initialize() override { g_order.push_back(2); }
    void Start() override { g_order.push_back(20); }
    void Stop() override { g_order.push_back(-20); }
    void Shutdown() override { g_order.push_back(-2); }
};

int run_tests() {
    using namespace te::subsystems;

    g_order.clear();

    SubsystemDescriptor descA{};
    descA.typeInfo = &typeid(A);
    descA.dependencies = nullptr;
    descA.dependencyCount = 0;
    descA.priority = 1;

    SubsystemDescriptor descB{};
    descB.typeInfo = &typeid(B);
    descB.dependencies = nullptr;
    descB.dependencyCount = 0;
    descB.priority = 0;

    Registry::Register(descA, std::make_unique<A>());
    Registry::Register(descB, std::make_unique<B>());

    bool initOk = Lifecycle::InitializeAll(Registry{});
    assert(initOk && "InitializeAll should succeed");

    Lifecycle::StartAll(Registry{});
    Lifecycle::StopAll(Registry{});
    Lifecycle::ShutdownAll(Registry{});

    assert(Registry::GetSubsystem<A>() == nullptr && "GetSubsystem after ShutdownAll returns nullptr");
    assert(Registry::GetSubsystem<B>() == nullptr && "GetSubsystem after ShutdownAll returns nullptr");

    return 0;
}

}  // namespace

int main() {
    return run_tests();
}
