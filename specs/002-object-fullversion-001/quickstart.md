# Quickstart: 002-Object (fullversion-001)

Contract: `specs/_contracts/002-object-public-api.md`. Only contract-declared types and APIs are used.

## Build

From repo root:

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

## Run tests

```bash
ctest -C Release --output-on-failure
```

Or run executables under `build/modules/object/tests/Release/`:
- `te_object_TypeRegistry_test.exe`
- `te_object_Serializer_roundtrip_test.exe`
- `te_object_PropertyBag_test.exe`
- `te_object_CreateInstance_test.exe`

## Precondition

Object APIs require Core to be initialized first (per contract). Ensure 001-Core is initialized before using TypeRegistry, serialization, or property APIs.

## Example (contract API only)

```cpp
#include "te/object/TypeRegistry.hpp"
#include "te/object/TypeDescriptor.hpp"
#include "te/object/TypeId.hpp"

// Register a type (contract: RegisterType, TypeDescriptor)
te::object::TypeDescriptor desc{};
desc.id = 1u;
desc.name = "MyType";
desc.size = sizeof(MyType);
desc.properties = nullptr;
desc.propertyCount = 0;
desc.methods = nullptr;
desc.methodCount = 0;
desc.baseTypeId = te::object::kInvalidTypeId;
te::object::TypeRegistry::RegisterType(desc);

// Query by name/ID (contract: GetTypeByName, GetTypeById)
auto* t = te::object::TypeRegistry::GetTypeByName("MyType");
void* instance = te::object::TypeRegistry::CreateInstance(t->id);
```
