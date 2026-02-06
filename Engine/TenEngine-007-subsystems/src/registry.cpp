/**
 * @file registry.cpp
 * @brief Registry implementation (contract: 007-subsystems-public-api.md).
 */
#include "te/subsystems/registry.hpp"
#include "te/subsystems/detail/registry_detail.hpp"
#include "te/subsystems/detail/registry_state.hpp"
#include "te/core/platform.h"

#include <cstdint>
#include <map>

namespace {
std::uint32_t CurrentPlatformBits() {
    std::uint32_t bits = 0;
#if TE_PLATFORM_WINDOWS
    bits |= 1u;
#endif
#if TE_PLATFORM_LINUX
    bits |= 2u;
#endif
#if TE_PLATFORM_MACOS
    bits |= 4u;
#endif
    return bits;
}
}  // namespace
#include <memory>

namespace te {
namespace subsystems {

namespace {

struct Entry {
    SubsystemDescriptor desc;
    std::unique_ptr<ISubsystem> instance;
};

std::map<void const*, Entry>* g_entries = nullptr;

std::map<void const*, Entry>& entries() {
    if (!g_entries) {
        static std::map<void const*, Entry> s_entries;
        g_entries = &s_entries;
    }
    return *g_entries;
}

}  // namespace

bool Registry::Register(SubsystemDescriptor const& desc, std::unique_ptr<ISubsystem> instance) {
    if (!instance || !desc.typeInfo)
        return false;
    if (desc.platformFilter != 0 && (desc.platformFilter & CurrentPlatformBits()) == 0)
        return false;  // platform filter excludes current platform
    auto& e = entries();
    if (e.find(desc.typeInfo) != e.end())
        return false;  // duplicate
    e[desc.typeInfo] = Entry{desc, std::move(instance)};
    return true;
}

void Registry::Unregister(void const* typeInfo) {
    entries().erase(typeInfo);
}

namespace detail {
ISubsystem* GetSubsystemByTypeInfo(void const* typeInfo) {
    auto& e = entries();
    auto it = e.find(typeInfo);
    if (it == e.end())
        return nullptr;
    return it->second.instance.get();
}

std::vector<SubsystemEntry> GetEntriesForLifecycle() {
    std::vector<SubsystemEntry> out;
    for (auto& [typeInfo, ent] : entries()) {
        (void)typeInfo;
        out.emplace_back(ent.desc, ent.instance.get());
    }
    return out;
}
}  // namespace detail

}  // namespace subsystems
}  // namespace te
