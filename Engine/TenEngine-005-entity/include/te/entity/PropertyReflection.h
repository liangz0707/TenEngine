/**
 * @file PropertyReflection.h
 * @brief Property reflection system for automatic UI generation.
 * Contract: specs/_contracts/005-entity-public-api.md
 *
 * This system allows components to declare their properties with metadata
 * for automatic editor UI generation without hardcoding each property.
 */

#ifndef TE_ENTITY_PROPERTY_REFLECTION_H
#define TE_ENTITY_PROPERTY_REFLECTION_H

#include <te/object/TypeId.h>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <string>
#include <functional>

namespace te {
namespace entity {

/**
 * @brief Property value types for UI rendering.
 */
enum class PropertyValueType : uint8_t {
    Unknown = 0,

    // Basic types
    Bool,
    Int32,
    Float32,
    Float64,

    // Vector types
    Float2,     // float[2]
    Float3,     // float[3] - can be vector or color
    Float4,     // float[4] - can be vector or color

    // Enum
    Enum,       // int32 with enum names

    // String
    String,     // char*/std::string

    // Resource reference
    ResourceId, // 64-bit resource ID

    // Custom (requires custom drawer)
    Custom
};

/**
 * @brief Editor hints for property rendering.
 */
struct EditorHints {
    float minValue = 0.0f;          // Minimum value for numeric types
    float maxValue = 0.0f;          // Maximum value for numeric types (0 = no limit)
    float step = 0.01f;             // Step size for drag widgets
    float speed = 1.0f;             // Drag speed multiplier
    bool isColor = false;           // True if Float3/Float4 should be rendered as color
    bool isRadians = false;         // True if value is in radians (display as degrees)
    bool noSlider = false;          // True to use drag instead of slider
    const char* format = "%.3f";    // Display format string
};

/**
 * @brief Extended property descriptor with editor metadata.
 */
struct PropertyMeta {
    const char* name = nullptr;         // Property display name
    PropertyValueType valueType = PropertyValueType::Unknown;
    size_t offset = 0;                  // Byte offset from component base
    size_t size = 0;                    // Size in bytes
    EditorHints hints;                  // Editor rendering hints

    // For enum types
    const char** enumNames = nullptr;   // Array of enum value names
    int enumCount = 0;                  // Number of enum values
    int enumDefault = 0;                // Default enum index

    // For custom types
    const char* customTypeName = nullptr;

    /**
     * @brief Get pointer to property value from component base pointer.
     */
    void* GetValuePtr(void* componentBase) const {
        return static_cast<char*>(componentBase) + offset;
    }

    const void* GetValuePtr(const void* componentBase) const {
        return static_cast<const char*>(componentBase) + offset;
    }
};

/**
 * @brief Component metadata containing all property descriptions.
 */
struct ComponentMeta {
    const char* typeName = nullptr;                 // Component type name
    size_t typeSize = 0;                            // Size of component in bytes
    const PropertyMeta* properties = nullptr;       // Array of property metadata
    size_t propertyCount = 0;                       // Number of properties

    // Factory functions for component manipulation
    std::function<void*(void*)> addComponent;       // Add component to entity
    std::function<void*(void*)> getComponent;       // Get component from entity
    std::function<void(void*)> removeComponent;     // Remove component from entity
    std::function<bool(void*)> hasComponent;        // Check if entity has component
};

/**
 * @brief Property registry interface for storing component metadata.
 */
class IPropertyRegistry {
public:
    virtual ~IPropertyRegistry() = default;

    /**
     * @brief Register component metadata.
     */
    virtual void RegisterComponentMeta(const ComponentMeta& meta) = 0;

    /**
     * @brief Get component metadata by type name.
     * @return Metadata or nullptr if not found
     */
    virtual const ComponentMeta* GetComponentMeta(const char* typeName) const = 0;

    /**
     * @brief Get all registered component types.
     */
    virtual std::vector<const ComponentMeta*> GetAllComponentMetas() const = 0;

    /**
     * @brief Check if component type is registered.
     */
    virtual bool HasComponentMeta(const char* typeName) const = 0;
};

/**
 * @brief Get the global property registry singleton.
 */
IPropertyRegistry* GetPropertyRegistry();

/**
 * @brief Initialize the property registry (called at startup).
 */
void InitializePropertyRegistry();

/**
 * @brief Shutdown the property registry (called at shutdown).
 */
void ShutdownPropertyRegistry();

// ============================================================================
// Helper Macros for Property Registration
// ============================================================================

/**
 * @brief Helper macro to calculate property offset.
 */
#define TE_PROP_OFFSET(Type, Member) (offsetof(Type, Member))

/**
 * @brief Helper macro to get property size.
 */
#define TE_PROP_SIZE(Type, Member) (sizeof(((Type*)0)->Member))

/**
 * @brief Property registration helper struct.
 */
template<typename ComponentType>
struct PropertyMetaBuilder {
    std::vector<PropertyMeta> properties;

    PropertyMetaBuilder& AddFloat(const char* name, size_t offset,
                                   float minVal = 0.0f, float maxVal = 0.0f,
                                   float step = 0.01f, const char* fmt = "%.3f") {
        PropertyMeta meta{};
        meta.name = name;
        meta.valueType = PropertyValueType::Float32;
        meta.offset = offset;
        meta.size = sizeof(float);
        meta.hints.minValue = minVal;
        meta.hints.maxValue = maxVal;
        meta.hints.step = step;
        meta.hints.format = fmt;
        properties.push_back(meta);
        return *this;
    }

    PropertyMetaBuilder& AddFloat3(const char* name, size_t offset,
                                    bool isColor = false, float step = 0.01f) {
        PropertyMeta meta{};
        meta.name = name;
        meta.valueType = PropertyValueType::Float3;
        meta.offset = offset;
        meta.size = sizeof(float) * 3;
        meta.hints.isColor = isColor;
        meta.hints.step = step;
        properties.push_back(meta);
        return *this;
    }

    PropertyMetaBuilder& AddFloat4(const char* name, size_t offset,
                                    bool isColor = false, float step = 0.01f) {
        PropertyMeta meta{};
        meta.name = name;
        meta.valueType = PropertyValueType::Float4;
        meta.offset = offset;
        meta.size = sizeof(float) * 4;
        meta.hints.isColor = isColor;
        meta.hints.step = step;
        properties.push_back(meta);
        return *this;
    }

    PropertyMetaBuilder& AddBool(const char* name, size_t offset) {
        PropertyMeta meta{};
        meta.name = name;
        meta.valueType = PropertyValueType::Bool;
        meta.offset = offset;
        meta.size = sizeof(bool);
        properties.push_back(meta);
        return *this;
    }

    PropertyMetaBuilder& AddInt(const char* name, size_t offset,
                                 int minVal = 0, int maxVal = 0, float step = 1.0f) {
        PropertyMeta meta{};
        meta.name = name;
        meta.valueType = PropertyValueType::Int32;
        meta.offset = offset;
        meta.size = sizeof(int32_t);
        meta.hints.minValue = static_cast<float>(minVal);
        meta.hints.maxValue = static_cast<float>(maxVal);
        meta.hints.step = step;
        meta.hints.noSlider = true;
        properties.push_back(meta);
        return *this;
    }

    PropertyMetaBuilder& AddEnum(const char* name, size_t offset,
                                  const char** enumNames, int enumCount, int defaultVal = 0) {
        PropertyMeta meta{};
        meta.name = name;
        meta.valueType = PropertyValueType::Enum;
        meta.offset = offset;
        meta.size = sizeof(int32_t);
        meta.enumNames = enumNames;
        meta.enumCount = enumCount;
        meta.enumDefault = defaultVal;
        properties.push_back(meta);
        return *this;
    }

    PropertyMetaBuilder& AddAngleRadians(const char* name, size_t offset,
                                          float minDeg = 0.0f, float maxDeg = 360.0f,
                                          float stepDeg = 1.0f) {
        PropertyMeta meta{};
        meta.name = name;
        meta.valueType = PropertyValueType::Float32;
        meta.offset = offset;
        meta.size = sizeof(float);
        meta.hints.minValue = minDeg;
        meta.hints.maxValue = maxDeg;
        meta.hints.step = stepDeg;
        meta.hints.isRadians = true;
        meta.hints.format = "%.1f°";
        properties.push_back(meta);
        return *this;
    }

    const PropertyMeta* GetData() const {
        return properties.empty() ? nullptr : properties.data();
    }

    size_t GetCount() const {
        return properties.size();
    }
};

}  // namespace entity
}  // namespace te

#endif  // TE_ENTITY_PROPERTY_REFLECTION_H
