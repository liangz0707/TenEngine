# 002-Object

Reflection, serialization, property system, type registry. Contract: `specs/_contracts/002-object-public-api.md`.

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
