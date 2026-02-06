/**
 * @file helpers.hpp
 * @brief Helper utilities for subsystems: platform filtering, descriptor building, dependency validation.
 */
#ifndef TE_SUBSYSTEMS_HELPERS_HPP
#define TE_SUBSYSTEMS_HELPERS_HPP

#include "te/subsystems/descriptor.hpp"
#include "te/core/platform.h"

#include <cstdint>
#include <cstddef>

namespace te {
namespace subsystems {

/**
 * Platform filter bits (aligned with Core platform detection).
 */
namespace PlatformFilter {
    constexpr std::uint32_t Windows = 1u;
    constexpr std::uint32_t Linux = 2u;
    constexpr std::uint32_t macOS = 4u;
    constexpr std::uint32_t Android = 8u;
    constexpr std::uint32_t iOS = 16u;
    constexpr std::uint32_t All = 0u;
}

/**
 * Get current platform filter bits.
 */
inline std::uint32_t GetCurrentPlatformBits() {
    std::uint32_t bits = 0;
#if TE_PLATFORM_WINDOWS
    bits |= PlatformFilter::Windows;
#endif
#if TE_PLATFORM_LINUX
    bits |= PlatformFilter::Linux;
#endif
#if TE_PLATFORM_MACOS
    bits |= PlatformFilter::macOS;
#endif
#if TE_PLATFORM_ANDROID
    bits |= PlatformFilter::Android;
#endif
#if TE_PLATFORM_IOS
    bits |= PlatformFilter::iOS;
#endif
    return bits;
}

/**
 * Check if platform filter matches current platform.
 * Returns true if filter is 0 (all platforms) or matches current platform.
 */
inline bool MatchesCurrentPlatform(std::uint32_t platformFilter) {
    if (platformFilter == 0)
        return true;  // All platforms
    return (platformFilter & GetCurrentPlatformBits()) != 0;
}

/**
 * Builder for SubsystemDescriptor.
 * Provides a fluent interface for constructing descriptors.
 */
class SubsystemDescriptorBuilder {
public:
    SubsystemDescriptorBuilder() {
        m_descriptor = {};
        m_descriptor.typeInfo = nullptr;
        m_descriptor.name = nullptr;
        m_descriptor.version = nullptr;
        m_descriptor.dependencies = nullptr;
        m_descriptor.dependencyCount = 0;
        m_descriptor.priority = 0;
        m_descriptor.platformFilter = PlatformFilter::All;
        m_descriptor.configData = nullptr;
    }
    
    SubsystemDescriptorBuilder& SetTypeInfo(void const* typeInfo) {
        m_descriptor.typeInfo = typeInfo;
        return *this;
    }
    
    SubsystemDescriptorBuilder& SetName(char const* name) {
        m_descriptor.name = name;
        return *this;
    }
    
    SubsystemDescriptorBuilder& SetVersion(char const* version) {
        m_descriptor.version = version;
        return *this;
    }
    
    SubsystemDescriptorBuilder& SetDependencies(char const* const* deps, std::size_t count) {
        m_descriptor.dependencies = deps;
        m_descriptor.dependencyCount = count;
        return *this;
    }
    
    SubsystemDescriptorBuilder& SetPriority(int priority) {
        m_descriptor.priority = priority;
        return *this;
    }
    
    SubsystemDescriptorBuilder& SetPlatformFilter(std::uint32_t filter) {
        m_descriptor.platformFilter = filter;
        return *this;
    }
    
    SubsystemDescriptorBuilder& SetConfigData(void const* config) {
        m_descriptor.configData = config;
        return *this;
    }
    
    SubsystemDescriptor Build() const {
        return m_descriptor;
    }
    
private:
    SubsystemDescriptor m_descriptor;
};

/**
 * Validate subsystem descriptor.
 * Returns true if descriptor is valid, false otherwise.
 */
inline bool ValidateDescriptor(SubsystemDescriptor const& desc, char const** errorMsg = nullptr) {
    if (!desc.typeInfo) {
        if (errorMsg) *errorMsg = "TypeInfo is null";
        return false;
    }
    
    if (!desc.name || desc.name[0] == '\0') {
        if (errorMsg) *errorMsg = "Name is null or empty";
        return false;
    }
    
    if (desc.dependencyCount > 0 && !desc.dependencies) {
        if (errorMsg) *errorMsg = "DependencyCount > 0 but dependencies is null";
        return false;
    }
    
    // Validate dependencies array
    if (desc.dependencies) {
        for (std::size_t i = 0; i < desc.dependencyCount; ++i) {
            if (!desc.dependencies[i] || desc.dependencies[i][0] == '\0') {
                if (errorMsg) *errorMsg = "Dependency name is null or empty";
                return false;
            }
        }
    }
    
    return true;
}

/**
 * Check if descriptor is valid for current platform.
 */
inline bool IsValidForCurrentPlatform(SubsystemDescriptor const& desc) {
    return MatchesCurrentPlatform(desc.platformFilter);
}

}  // namespace subsystems
}  // namespace te

#endif  // TE_SUBSYSTEMS_HELPERS_HPP
