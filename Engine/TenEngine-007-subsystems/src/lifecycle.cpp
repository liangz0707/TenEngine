/**
 * @file lifecycle.cpp
 * @brief Lifecycle implementation (contract: 007-subsystems-public-api.md).
 */
#include "te/subsystems/lifecycle.hpp"
#include "te/subsystems/detail/registry_detail.hpp"
#include "te/subsystems/detail/registry_state.hpp"

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <typeinfo>
#include <vector>

namespace te {
namespace subsystems {

namespace {

std::string TypeInfoToName(void const* typeInfo) {
    if (!typeInfo) return {};
    auto const* ti = static_cast<std::type_info const*>(typeInfo);
    return ti->name();
}

/** Kahn's algorithm; returns sorted indices or empty on cycle. */
bool TopoSort(std::vector<detail::SubsystemEntry> const& entries,
              std::map<std::string, size_t> const& nameToIdx,
              std::vector<size_t>& out) {
    size_t const n = entries.size();
    std::vector<std::vector<size_t>> adj(n);  // adj[j] = {i : i depends on j}
    std::vector<int> inDeg(n, 0);

    for (size_t i = 0; i < n; ++i) {
        auto const& desc = entries[i].first;
        for (size_t k = 0; k < desc.dependencyCount && desc.dependencies && desc.dependencies[k]; ++k) {
            std::string depName(desc.dependencies[k]);
            auto it = nameToIdx.find(depName);
            if (it != nameToIdx.end() && it->second != i) {
                size_t j = it->second;
                adj[j].push_back(i);
                ++inDeg[i];
            }
        }
    }

    std::vector<size_t> q;
    for (size_t i = 0; i < n; ++i)
        if (inDeg[i] == 0)
            q.push_back(i);

    out.clear();
    while (!q.empty()) {
        std::sort(q.begin(), q.end(), [&entries](size_t a, size_t b) {
            return entries[a].first.priority < entries[b].first.priority;  // asc for init
        });
        size_t u = q.back();
        q.pop_back();
        out.push_back(u);
        for (size_t i : adj[u]) {
            --inDeg[i];
            if (inDeg[i] == 0)
                q.push_back(i);
        }
    }

    if (out.size() != n)
        return false;  // cycle
    return true;
}

}  // namespace

bool Lifecycle::InitializeAll(Registry const&) {
    auto ents = detail::GetEntriesForLifecycle();
    if (ents.empty())
        return true;

    std::map<std::string, size_t> nameToIdx;
    for (size_t i = 0; i < ents.size(); ++i)
        nameToIdx[TypeInfoToName(ents[i].first.typeInfo)] = i;

    std::vector<size_t> order;
    if (!TopoSort(ents, nameToIdx, order))
        return false;  // cycle

    for (size_t idx : order) {
        if (ents[idx].second)
            ents[idx].second->Initialize();
    }
    return true;
}

void Lifecycle::StartAll(Registry const&) {
    auto ents = detail::GetEntriesForLifecycle();
    std::vector<size_t> indices(ents.size());
    for (size_t i = 0; i < ents.size(); ++i)
        indices[i] = i;
    std::sort(indices.begin(), indices.end(), [&ents](size_t a, size_t b) {
        return ents[a].first.priority < ents[b].first.priority;
    });
    for (size_t idx : indices) {
        if (ents[idx].second)
            ents[idx].second->Start();
    }
}

void Lifecycle::StopAll(Registry const&) {
    auto ents = detail::GetEntriesForLifecycle();
    std::vector<size_t> indices(ents.size());
    for (size_t i = 0; i < ents.size(); ++i)
        indices[i] = i;
    std::sort(indices.begin(), indices.end(), [&ents](size_t a, size_t b) {
        return ents[a].first.priority > ents[b].first.priority;  // desc for stop
    });
    for (size_t idx : indices) {
        if (ents[idx].second)
            ents[idx].second->Stop();
    }
}

void Lifecycle::ShutdownAll(Registry&) {
    auto ents = detail::GetEntriesForLifecycle();
    std::vector<size_t> indices(ents.size());
    for (size_t i = 0; i < ents.size(); ++i)
        indices[i] = i;
    std::sort(indices.begin(), indices.end(), [&ents](size_t a, size_t b) {
        return ents[a].first.priority > ents[b].first.priority;
    });
    for (size_t idx : indices) {
        if (ents[idx].second)
            ents[idx].second->Shutdown();
    }
    detail::SetShutdown(true);
}

}  // namespace subsystems
}  // namespace te
