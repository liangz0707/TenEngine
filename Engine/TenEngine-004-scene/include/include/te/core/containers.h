/**
 * @file containers.h
 * @brief Array, Map, String, UniquePtr, SharedPtr (contract: 001-core-public-api.md).
 * Only contract-declared types; allocator support; no reflection/ECS.
 */
#ifndef TE_CORE_CONTAINERS_H
#define TE_CORE_CONTAINERS_H

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace te {
namespace core {

/** Dynamic array; works with custom allocator per contract capability 6. */
template <typename T, typename Allocator = std::allocator<T>>
using Array = std::vector<T, Allocator>;

/** Hash map; works with custom allocator per contract capability 6. */
template <typename Key, typename Value,
          typename Hash = std::hash<Key>,
          typename KeyEqual = std::equal_to<Key>,
          typename Allocator = std::allocator<std::pair<Key const, Value>>>
using Map = std::unordered_map<Key, Value, Hash, KeyEqual, Allocator>;

/** String; default char type. */
using String = std::string;

/** Unique ownership smart pointer per contract capability 6. */
template <typename T>
using UniquePtr = std::unique_ptr<T>;

/** Shared ownership smart pointer per contract capability 6. */
template <typename T>
using SharedPtr = std::shared_ptr<T>;

}  // namespace core
}  // namespace te

#endif  // TE_CORE_CONTAINERS_H
