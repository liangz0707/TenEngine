/**
 * @file Guid.cpp
 * @brief Implementation of GUID system (contract: specs/_contracts/002-object-public-api.md).
 * Platform-specific GUID generation and string conversion.
 */

#include "te/object/Guid.h"
#include <cstring>
#include <sstream>
#include <iomanip>
#include <cstdio>

#if defined(_WIN32) || defined(_WIN64)
#include <objbase.h>
#pragma comment(lib, "ole32.lib")
#elif defined(__linux__) || defined(__APPLE__)
#include <uuid/uuid.h>
#endif

namespace te {
namespace object {

GUID GUID::Generate() {
    GUID guid{};
    
#if defined(_WIN32) || defined(_WIN64)
    ::GUID winGuid;
    if (CoCreateGuid(&winGuid) == S_OK) {
        std::memcpy(guid.data, &winGuid, 16);
    }
#elif defined(__linux__) || defined(__APPLE__)
    uuid_t uuid;
    uuid_generate(uuid);
    std::memcpy(guid.data, uuid, 16);
#else
    // Fallback: simple random generation (not cryptographically secure)
    // In production, should use proper platform API
    static std::uint32_t counter = 0;
    std::uint32_t* ptr = reinterpret_cast<std::uint32_t*>(guid.data);
    for (int i = 0; i < 4; ++i) {
        ptr[i] = ++counter;
    }
#endif
    
    return guid;
}

GUID GUID::FromString(char const* str) {
    GUID guid{};
    if (!str) {
        return guid;
    }
    // Parse standard UUID: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx (8-4-4-4-12 hex)
    std::uint32_t d0;
    std::uint16_t w0, w1, w2, w3, w4, w5;
    int result = std::sscanf(str, "%08x-%04hx-%04hx-%04hx-%04hx%04hx%04hx",
                             &d0, &w0, &w1, &w2, &w3, &w4, &w5);
    if (result == 7) {
        std::memcpy(&guid.data[0], &d0, 4);
        std::memcpy(&guid.data[4], &w0, 2);
        std::memcpy(&guid.data[6], &w1, 2);
        std::memcpy(&guid.data[8], &w2, 2);
        std::memcpy(&guid.data[10], &w3, 2);
        std::memcpy(&guid.data[12], &w4, 2);
        std::memcpy(&guid.data[14], &w5, 2);
    }
    return guid;
}

std::string GUID::ToString() const {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    
    // Format: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
    oss << std::setw(8) << (reinterpret_cast<std::uint32_t const*>(data)[0]);
    oss << '-';
    oss << std::setw(4) << (reinterpret_cast<std::uint16_t const*>(data)[2]);
    oss << '-';
    oss << std::setw(4) << (reinterpret_cast<std::uint16_t const*>(data)[3]);
    oss << '-';
    oss << std::setw(4) << (reinterpret_cast<std::uint16_t const*>(data)[4]);
    oss << '-';
    oss << std::setw(4) << (reinterpret_cast<std::uint16_t const*>(data)[5]);
    oss << std::setw(4) << (reinterpret_cast<std::uint16_t const*>(data)[6]);
    oss << std::setw(4) << (reinterpret_cast<std::uint16_t const*>(data)[7]);
    
    return oss.str();
}

bool GUID::operator==(GUID const& other) const {
    return std::memcmp(data, other.data, 16) == 0;
}

bool GUID::operator!=(GUID const& other) const {
    return !(*this == other);
}

bool GUID::operator<(GUID const& other) const {
    return std::memcmp(data, other.data, 16) < 0;
}

bool GUID::IsNull() const {
    for (int i = 0; i < 16; ++i) {
        if (data[i] != 0) {
            return false;
        }
    }
    return true;
}

} // namespace object
} // namespace te
