/**
 * @file test_guid.cpp
 * @brief Unit tests for GUID.
 */

#include "te/object/Guid.h"
#include <cassert>
#include <cstring>

int main() {
    using namespace te::object;
    
    // Test GUID generation
    GUID guid1 = GUID::Generate();
    GUID guid2 = GUID::Generate();
    assert(!guid1.IsNull());
    assert(!guid2.IsNull());
    assert(guid1 != guid2);  // Should be different
    
    // Test null GUID
    GUID nullGuid{};
    assert(nullGuid.IsNull());
    
    // Test comparison
    assert(guid1 == guid1);
    assert(guid1 != guid2);
    
    // Test string conversion
    std::string str = guid1.ToString();
    assert(!str.empty());
    
    GUID parsed = GUID::FromString(str.c_str());
    assert(parsed == guid1);
    
    // Test ObjectRef
    ObjectRef ref1(guid1);
    assert(!ref1.IsNull());
    
    ObjectRef ref2;
    assert(ref2.IsNull());
    
    return 0;
}
