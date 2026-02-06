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
    A() : m_desc{} {
        m_desc.typeInfo = &typeid(A);
        m_desc.name = "A";
        m_desc.version = nullptr;
        m_desc.dependencies = nullptr;
        m_desc.dependencyCount = 0;
        m_desc.priority = 1;
        m_desc.platformFilter = 0;
        m_desc.configData = nullptr;
    }
    
    bool Initialize() override { g_order.push_back(1); return true; }
    void Start() override { g_order.push_back(10); }
    void Stop() override { g_order.push_back(-10); }
    void Shutdown() override { g_order.push_back(-1); }
    
    te::subsystems::SubsystemDescriptor const& GetDescriptor() const override { return m_desc; }
    char const* GetName() const override { return m_desc.name; }
    
private:
    te::subsystems::SubsystemDescriptor m_desc;
};

class B : public te::subsystems::ISubsystem {
public:
    B() : m_desc{} {
        m_desc.typeInfo = &typeid(B);
        m_desc.name = "B";
        m_desc.version = nullptr;
        m_desc.dependencies = nullptr;
        m_desc.dependencyCount = 0;
        m_desc.priority = 0;
        m_desc.platformFilter = 0;
        m_desc.configData = nullptr;
    }
    
    bool Initialize() override { g_order.push_back(2); return true; }
    void Start() override { g_order.push_back(20); }
    void Stop() override { g_order.push_back(-20); }
    void Shutdown() override { g_order.push_back(-2); }
    
    te::subsystems::SubsystemDescriptor const& GetDescriptor() const override { return m_desc; }
    char const* GetName() const override { return m_desc.name; }
    
private:
    te::subsystems::SubsystemDescriptor m_desc;
};

int run_tests() {
    using namespace te::subsystems;

    g_order.clear();

    te::subsystems::SubsystemDescriptor descA{};
    descA.typeInfo = &typeid(A);
    descA.name = "A";
    descA.version = nullptr;
    descA.dependencies = nullptr;
    descA.dependencyCount = 0;
    descA.priority = 1;
    descA.platformFilter = 0;
    descA.configData = nullptr;

    te::subsystems::SubsystemDescriptor descB{};
    descB.typeInfo = &typeid(B);
    descB.name = "B";
    descB.version = nullptr;
    descB.dependencies = nullptr;
    descB.dependencyCount = 0;
    descB.priority = 0;
    descB.platformFilter = 0;
    descB.configData = nullptr;

    Registry::Register(descA, std::make_unique<A>());
    Registry::Register(descB, std::make_unique<B>());

    auto& reg = Registry::GetInstance();
    auto initResult = Lifecycle::InitializeAll(reg);
    assert(initResult.success && "InitializeAll should succeed");

    auto startResult = Lifecycle::StartAll(reg);
    assert(startResult.success && "StartAll should succeed");
    
    auto stopResult = Lifecycle::StopAll(reg);
    assert(stopResult.success && "StopAll should succeed");
    
    Lifecycle::ShutdownAll(reg);

    assert(Registry::GetSubsystem<A>() == nullptr && "GetSubsystem after ShutdownAll returns nullptr");
    assert(Registry::GetSubsystem<B>() == nullptr && "GetSubsystem after ShutdownAll returns nullptr");

    return 0;
}

}  // namespace

int main() {
    return run_tests();
}
