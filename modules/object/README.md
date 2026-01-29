# 002-Object

Reflection, serialization, property system, type registry. Contract: `specs/_contracts/002-object-public-api.md`. Uses **001-Core** memory API (Alloc/Free) per contract.

## Use of 001-Core interface

- **Memory**: Heap allocation uses the same API as 001-core-public-api (Alloc(size, alignment), Free(ptr)). When 001-Core is linked and initialized, Core's Alloc/Free should be used; when building standalone, a fallback implementation (in `detail/CoreMemory.hpp` + `CoreMemory.cpp`) is used so the module builds without Core.
- **CreateInstance**: Allocates via Core Alloc; callers must free with Core Free (or the same allocator when standalone). When 001-Core is not linked, use `te::object::detail::Free(ptr)` to free the result of CreateInstance.
- **Containers/threading**: 001-Core declares Map, String, Mutex, etc. When Core is not linked, the implementation uses `std::map`, `std::string`, `std::mutex` as fallbacks; when Core is integrated, these can be switched to Core-provided types.

## Precondition (per contract)

**Object APIs require Core to be initialized first.** Callers must ensure 001-Core is initialized before using TypeRegistry, serialization, or property APIs. Behavior when Core is not initialized is undefined unless otherwise specified by the contract.

## Build

From repo root (or add as subdirectory):

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

Tests: `ctest` or run `te_object_*_test` executables.
