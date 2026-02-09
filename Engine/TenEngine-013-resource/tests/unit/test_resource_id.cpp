/**
 * @file test_resource_id.cpp
 * @brief Unit tests for ResourceId (contract: specs/_contracts/013-resource-ABI.md).
 */

#include <te/resource/ResourceId.h>
#include <te/object/Guid.h>
#include <cassert>
#include <string>

using namespace te::resource;
using namespace te::object;

int main() {
    // Test ResourceId is alias of GUID
    ResourceId id1 = ResourceId::Generate();
    assert(!id1.IsNull());
    
    ResourceId id2 = ResourceId::Generate();
    assert(!id2.IsNull());
    assert(id1 != id2);  // Different GUIDs
    
    // Test FromString
    std::string idStr = id1.ToString();
    ResourceId id3 = ResourceId::FromString(idStr.c_str());
    assert(id3 == id1);
    
    // Test null GUID
    ResourceId nullId{};
    assert(nullId.IsNull());
    
    // Test ToString
    std::string str = id1.ToString();
    assert(!str.empty());
    
    return 0;
}
