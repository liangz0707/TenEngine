# Quickstart: 002-Object (fullversion-002)

Contract: `specs/_contracts/002-object-public-api.md`. Only contract-declared types and APIs are used.

## Build

From repo root (TenEngine-002-object):

```bash
cmake -B build -DTENENGINE_002_OBJECT_STANDALONE=ON
cmake --build build --config Debug
```

Or with 001-Core as source dependency:

```bash
cmake -B build -DTENENGINE_002_OBJECT_STANDALONE=OFF
cmake --build build --config Debug
```

## Run tests

```bash
ctest --test-dir build -C Debug --output-on-failure
```

Or run executables under `build/Debug/` (Windows) or `build/` (Ninja):
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
