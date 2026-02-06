/**
 * @file XMLSerializer.cpp
 * @brief Implementation of XMLSerializer (contract: specs/_contracts/002-object-public-api.md).
 * XML format serialization with structured data and tool compatibility.
 */

#include "te/object/Serializer.h"
#include "te/object/TypeRegistry.h"
#include "te/object/TypeId.h"
#include "te/object/Guid.h"
#include "te/core/alloc.h"
#include <sstream>
#include <cstring>
#include <cctype>
#include <vector>
#include <string>

namespace te {
namespace object {

namespace {

constexpr std::uint32_t kCurrentVersion = 1;

// Helper: Check if type is GUID
bool IsGUIDType(TypeId typeId) {
    // GUID is a struct with 16 bytes
    TypeDescriptor const* desc = TypeRegistry::GetTypeById(typeId);
    if (!desc) return false;
    return desc->size == sizeof(GUID) && desc->name && std::strcmp(desc->name, "GUID") == 0;
}

// Helper: Check if type is ObjectRef
bool IsObjectRefType(TypeId typeId) {
    TypeDescriptor const* desc = TypeRegistry::GetTypeById(typeId);
    if (!desc) return false;
    return desc->size == sizeof(ObjectRef) && desc->name && std::strcmp(desc->name, "ObjectRef") == 0;
}

// Helper: Check if type is a complex type (has properties)
bool IsComplexType(TypeId typeId) {
    if (typeId == kInvalidTypeId) return false;
    TypeDescriptor const* desc = TypeRegistry::GetTypeById(typeId);
    if (!desc) return false;
    return desc->propertyCount > 0;
}

// Helper: Check if type is a basic numeric/string type
bool IsBasicType(TypeId typeId, std::size_t size) {
    if (typeId == kInvalidTypeId) return false;
    
    // Check for GUID/ObjectRef first
    if (IsGUIDType(typeId) || IsObjectRefType(typeId)) {
        return false;  // These are handled specially
    }
    
    // Basic types: int, float, double, bool, char*
    return size == sizeof(int32_t) || 
           size == sizeof(int64_t) || 
           size == sizeof(float) || 
           size == sizeof(double) || 
           size == sizeof(bool) ||
           size == sizeof(char*);
}

// XML Writer with proper formatting
class XMLWriter {
public:
    XMLWriter() {
        stream_ << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    }
    
    void StartElement(char const* name) {
        CloseOpenTag();
        Indent();
        stream_ << '<' << EscapeXML(name);
        elementStack_.push_back(name ? name : "");
        openTagClosed_ = false;
        ++indentLevel_;
    }
    
    void EndElement() {
        --indentLevel_;
        if (!openTagClosed_) {
            stream_ << "/>\n";
            openTagClosed_ = true;
        } else {
            Indent();
            if (!elementStack_.empty()) {
                stream_ << "</" << elementStack_.back() << ">\n";
            }
        }
        if (!elementStack_.empty()) {
            elementStack_.pop_back();
        }
    }
    
    void WriteAttribute(char const* name, char const* value) {
        if (!openTagClosed_ && name && value) {
            stream_ << ' ' << EscapeXML(name) << "=\"" << EscapeAttributeValue(value) << "\"";
        }
    }
    
    void WriteAttribute(char const* name, int64_t value) {
        if (!openTagClosed_ && name) {
            stream_ << ' ' << EscapeXML(name) << "=\"" << value << "\"";
        }
    }
    
    void WriteAttribute(char const* name, uint64_t value) {
        if (!openTagClosed_ && name) {
            stream_ << ' ' << EscapeXML(name) << "=\"" << value << "\"";
        }
    }
    
    void WriteAttribute(char const* name, double value) {
        if (!openTagClosed_ && name) {
            stream_ << ' ' << EscapeXML(name) << "=\"" << value << "\"";
        }
    }
    
    void WriteAttribute(char const* name, bool value) {
        if (!openTagClosed_ && name) {
            stream_ << ' ' << EscapeXML(name) << "=\"" << (value ? "true" : "false") << "\"";
        }
    }
    
    void WriteText(char const* text) {
        CloseOpenTag();
        if (text) {
            stream_ << EscapeXML(text);
        }
    }
    
    void WriteText(int32_t value) {
        CloseOpenTag();
        stream_ << static_cast<int64_t>(value);
    }
    
    void WriteText(int64_t value) {
        CloseOpenTag();
        stream_ << value;
    }
    
    void WriteText(double value) {
        CloseOpenTag();
        stream_ << value;
    }
    
    void WriteText(bool value) {
        CloseOpenTag();
        stream_ << (value ? "true" : "false");
    }
    
    std::string GetString() const {
        return stream_.str();
    }
    
private:
    void CloseOpenTag() {
        if (!openTagClosed_) {
            stream_ << ">\n";
            openTagClosed_ = true;
        }
    }
    
    void Indent() {
        for (int i = 0; i < indentLevel_; ++i) {
            stream_ << "  ";
        }
    }
    
    std::string EscapeXML(char const* str) {
        std::string result;
        if (str) {
            for (char const* p = str; *p; ++p) {
                switch (*p) {
                    case '<': result += "&lt;"; break;
                    case '>': result += "&gt;"; break;
                    case '&': result += "&amp;"; break;
                    case '"': result += "&quot;"; break;
                    case '\'': result += "&apos;"; break;
                    default: 
                        if (std::isprint(*p)) {
                            result += *p;
                        }
                        break;
                }
            }
        }
        return result;
    }
    
    std::string EscapeAttributeValue(char const* str) {
        std::string result;
        if (str) {
            for (char const* p = str; *p; ++p) {
                switch (*p) {
                    case '<': result += "&lt;"; break;
                    case '>': result += "&gt;"; break;
                    case '&': result += "&amp;"; break;
                    case '"': result += "&quot;"; break;
                    case '\'': result += "&apos;"; break;
                    case '\t': result += "&#9;"; break;
                    case '\n': result += "&#10;"; break;
                    case '\r': result += "&#13;"; break;
                    default: 
                        if (std::isprint(*p)) {
                            result += *p;
                        }
                        break;
                }
            }
        }
        return result;
    }
    
    std::ostringstream stream_;
    std::vector<std::string> elementStack_;
    int indentLevel_ = 0;
    bool openTagClosed_ = true;
};

// XML Parser
class XMLParser {
public:
    XMLParser(char const* xml) : xml_(xml), pos_(0) {
        if (!xml_) {
            xml_ = "";
        }
        SkipWhitespace();
        // Skip XML declaration if present
        if (StartsWith("<?xml")) {
            SkipUntil('>');
            Advance();  // Skip '>'
            SkipWhitespace();
        }
    }
    
    bool ParseElement(void* obj, TypeDescriptor const* desc) {
        if (!obj || !desc) return false;
        
        SkipWhitespace();
        if (Peek() != '<') return false;
        Advance();  // Skip '<'
        
        std::string tagName = ParseTagName();
        if (tagName.empty()) return false;
        
        std::uint32_t version = kCurrentVersion;
        std::string typeName;
        
        // Parse attributes
        SkipWhitespace();
        while (Peek() != '>' && Peek() != '/' && Peek() != '\0') {
            std::string attrName = ParseAttributeName();
            if (attrName.empty()) break;
            
            SkipWhitespace();
            if (Peek() != '=') break;
            Advance();  // Skip '='
            SkipWhitespace();
            
            if (Peek() != '"') break;
            Advance();  // Skip '"'
            std::string attrValue = ParseAttributeValue();
            if (Peek() != '"') break;
            Advance();  // Skip '"'
            
            // Handle known attributes
            if (attrName == "version") {
                version = static_cast<std::uint32_t>(std::strtoul(attrValue.c_str(), nullptr, 10));
            } else if (attrName == "type") {
                typeName = attrValue;
            }
            
            SkipWhitespace();
        }
        
        // Check for self-closing tag
        if (Peek() == '/') {
            Advance();  // Skip '/'
            if (Peek() != '>') return false;
            Advance();  // Skip '>'
            return true;
        }
        
        if (Peek() != '>') return false;
        Advance();  // Skip '>'
        
        // Parse child elements and text content
        SkipWhitespace();
        while (Peek() != '\0' && Peek() != '<') {
            Advance();  // Skip text content
            SkipWhitespace();
        }
        
        // Parse child elements
        while (Peek() == '<') {
            std::size_t savedPos = pos_;
            Advance();  // Skip '<'
            
            // Check for closing tag
            if (Peek() == '/') {
                Advance();  // Skip '/'
                std::string closeTag = ParseTagName();
                SkipUntil('>');
                Advance();  // Skip '>'
                if (closeTag == tagName) {
                    break;  // End of this element
                }
                return false;  // Mismatched closing tag
            }
            
            pos_ = savedPos;  // Restore position
            
            // Parse child element
            std::string childTag = PeekTagName();
            if (childTag.empty()) break;
            
            // Find property by name
            PropertyDescriptor const* prop = nullptr;
            for (std::size_t i = 0; i < desc->propertyCount; ++i) {
                if (desc->properties[i].name && childTag == desc->properties[i].name) {
                    prop = &desc->properties[i];
                    break;
                }
            }
            
            // Check for array item (propertyName + "Item")
            if (!prop) {
                for (std::size_t i = 0; i < desc->propertyCount; ++i) {
                    if (desc->properties[i].name) {
                        std::string itemName = std::string(desc->properties[i].name) + "Item";
                        if (childTag == itemName) {
                            prop = &desc->properties[i];
                            // TODO: Handle array deserialization
                            SkipElement();  // Skip for now
                            continue;
                        }
                    }
                }
            }
            
            if (prop) {
                void* propPtr = static_cast<char*>(obj) + prop->offset;
                if (!ParseValue(propPtr, prop)) {
                    return false;
                }
            } else {
                // Unknown element, skip it
                SkipElement();
            }
            
            SkipWhitespace();
        }
        
        // Find closing tag
        SkipWhitespace();
        if (Peek() == '<') {
            Advance();  // Skip '<'
            if (Peek() == '/') {
                Advance();  // Skip '/'
                std::string closeTag = ParseTagName();
                SkipUntil('>');
                Advance();  // Skip '>'
                return closeTag == tagName;
            }
        }
        
        return true;
    }
    
private:
    char Peek() const {
        return xml_[pos_] ? xml_[pos_] : '\0';
    }
    
    void Advance() {
        if (xml_[pos_]) ++pos_;
    }
    
    void SkipWhitespace() {
        while (std::isspace(Peek())) {
            Advance();
        }
    }
    
    bool StartsWith(char const* str) {
        if (!str) return false;
        std::size_t len = std::strlen(str);
        for (std::size_t i = 0; i < len; ++i) {
            if (xml_[pos_ + i] != str[i]) {
                return false;
            }
        }
        return true;
    }
    
    void SkipUntil(char c) {
        while (Peek() && Peek() != c) {
            Advance();
        }
    }
    
    std::string ParseTagName() {
        std::string result;
        SkipWhitespace();
        while (Peek() && !std::isspace(Peek()) && Peek() != '>' && Peek() != '/' && Peek() != '=') {
            result += Peek();
            Advance();
        }
        return result;
    }
    
    std::string ParseAttributeName() {
        std::string result;
        SkipWhitespace();
        while (Peek() && !std::isspace(Peek()) && Peek() != '=') {
            result += Peek();
            Advance();
        }
        return result;
    }
    
    std::string ParseAttributeValue() {
        std::string result;
        while (Peek() && Peek() != '"') {
            if (Peek() == '\\') {
                Advance();
                if (Peek()) {
                    result += Peek();
                    Advance();
                }
            } else {
                result += Peek();
                Advance();
            }
        }
        return result;
    }
    
    std::string ParseTextContent() {
        std::string result;
        while (Peek() && Peek() != '<') {
            result += Peek();
            Advance();
        }
        return result;
    }
    
    std::string PeekTagName() {
        std::size_t savedPos = pos_;
        if (Peek() == '<') {
            Advance();
            std::string name = ParseTagName();
            pos_ = savedPos;
            return name;
        }
        return "";
    }
    
    bool ParseValue(void* out, PropertyDescriptor const* prop) {
        if (!out || !prop) return false;
        
        SkipWhitespace();
        if (Peek() != '<') return false;
        Advance();  // Skip '<'
        
        std::string tagName = ParseTagName();
        if (tagName.empty()) return false;
        
        // Check for attributes (for GUID/ObjectRef or basic types as attributes)
        SkipWhitespace();
        bool hasGuidAttr = false;
        std::string guidValue;
        
        while (Peek() != '>' && Peek() != '/' && Peek() != '\0') {
            std::string attrName = ParseAttributeName();
            if (attrName.empty()) break;
            
            SkipWhitespace();
            if (Peek() != '=') break;
            Advance();  // Skip '='
            SkipWhitespace();
            
            if (Peek() != '"') break;
            Advance();  // Skip '"'
            std::string attrValue = ParseAttributeValue();
            if (Peek() != '"') break;
            Advance();  // Skip '"'
            
            if (attrName == "guid" || attrName == "value") {
                guidValue = attrValue;
                hasGuidAttr = true;
            }
            
            SkipWhitespace();
        }
        
        // Handle GUID/ObjectRef from attribute
        if (hasGuidAttr && !guidValue.empty()) {
            if (IsGUIDType(prop->valueTypeId)) {
                GUID guid = GUID::FromString(guidValue.c_str());
                std::memcpy(out, &guid, sizeof(GUID));
                SkipUntil('>');
                Advance();
                if (Peek() == '/') {
                    Advance();
                    SkipUntil('>');
                    Advance();
                } else {
                    SkipUntil('<');
                    Advance();
                    if (Peek() == '/') {
                        Advance();
                        SkipTagName();
                        SkipUntil('>');
                        Advance();
                    }
                }
                return true;
            } else if (IsObjectRefType(prop->valueTypeId)) {
                GUID guid = GUID::FromString(guidValue.c_str());
                ObjectRef ref(guid);
                std::memcpy(out, &ref, sizeof(ObjectRef));
                SkipUntil('>');
                Advance();
                if (Peek() == '/') {
                    Advance();
                    SkipUntil('>');
                    Advance();
                } else {
                    SkipUntil('<');
                    Advance();
                    if (Peek() == '/') {
                        Advance();
                        SkipTagName();
                        SkipUntil('>');
                        Advance();
                    }
                }
                return true;
            }
        }
        
        // Check for self-closing tag
        if (Peek() == '/') {
            Advance();  // Skip '/'
            if (Peek() != '>') return false;
            Advance();  // Skip '>'
            return true;
        }
        
        if (Peek() != '>') return false;
        Advance();  // Skip '>'
        
        SkipWhitespace();
        
        // Parse text content for basic types
        if (Peek() != '<') {
            std::string text = ParseTextContent();
            SkipWhitespace();
            
            // Parse basic types from text
            if (prop->size == sizeof(int32_t)) {
                int32_t value = static_cast<int32_t>(std::strtol(text.c_str(), nullptr, 10));
                std::memcpy(out, &value, sizeof(int32_t));
            } else if (prop->size == sizeof(int64_t)) {
                int64_t value = std::strtoll(text.c_str(), nullptr, 10);
                std::memcpy(out, &value, sizeof(int64_t));
            } else if (prop->size == sizeof(float)) {
                float value = std::strtof(text.c_str(), nullptr);
                std::memcpy(out, &value, sizeof(float));
            } else if (prop->size == sizeof(double)) {
                double value = std::strtod(text.c_str(), nullptr);
                std::memcpy(out, &value, sizeof(double));
            } else if (prop->size == sizeof(bool)) {
                bool value = (text == "true" || text == "1");
                std::memcpy(out, &value, sizeof(bool));
            }
            // Note: string handling would need special treatment
            
            // Find closing tag
            SkipWhitespace();
            if (Peek() == '<') {
                Advance();
                if (Peek() == '/') {
                    Advance();
                    SkipTagName();
                    SkipUntil('>');
                    Advance();
                }
            }
            return true;
        }
        
        // Parse nested object
        if (IsComplexType(prop->valueTypeId)) {
            TypeDescriptor const* nestedDesc = TypeRegistry::GetTypeById(prop->valueTypeId);
            if (nestedDesc) {
                return ParseElement(out, nestedDesc);
            }
        }
        
        // Skip unknown element
        SkipElement();
        return true;
    }
    
    void SkipTagName() {
        SkipWhitespace();
        while (Peek() && !std::isspace(Peek()) && Peek() != '>' && Peek() != '/') {
            Advance();
        }
    }
    
    void SkipElement() {
        if (Peek() != '<') return;
        Advance();
        
        std::string tagName = ParseTagName();
        SkipUntil('>');
        Advance();
        
        if (Peek() == '/') {
            // Was self-closing
            return;
        }
        
        // Skip content until closing tag
        int depth = 1;
        while (depth > 0 && Peek()) {
            if (Peek() == '<') {
                Advance();
                if (Peek() == '/') {
                    Advance();
                    SkipTagName();
                    SkipUntil('>');
                    Advance();
                    --depth;
                } else {
                    SkipTagName();
                    SkipUntil('>');
                    Advance();
                    ++depth;
                }
            } else {
                Advance();
            }
        }
    }
    
    char const* xml_;
    std::size_t pos_;
};

} // namespace

class XMLSerializerImpl : public ISerializer {
public:
    XMLSerializerImpl() : version_(kCurrentVersion), migration_(nullptr) {}
    
    bool Serialize(SerializedBuffer& out, void const* obj, TypeId typeId) override {
        if (!obj || typeId == kInvalidTypeId) {
            return false;
        }
        
        TypeDescriptor const* desc = TypeRegistry::GetTypeById(typeId);
        if (!desc) {
            return false;
        }
        
        return SerializeInternal(out, obj, desc);
    }
    
    bool Serialize(SerializedBuffer& out, void const* obj, char const* typeName) override {
        if (!obj || !typeName) {
            return false;
        }
        
        TypeDescriptor const* desc = TypeRegistry::GetTypeByName(typeName);
        if (!desc) {
            return false;
        }
        
        return SerializeInternal(out, obj, desc);
    }
    
    bool Deserialize(SerializedBuffer const& buf, void* obj, TypeId typeId) override {
        if (!buf.IsValid() || !obj || typeId == kInvalidTypeId) {
            return false;
        }
        
        TypeDescriptor const* desc = TypeRegistry::GetTypeById(typeId);
        if (!desc) {
            return false;
        }
        
        return DeserializeInternal(buf, obj, desc);
    }
    
    bool Deserialize(SerializedBuffer const& buf, void* obj, char const* typeName) override {
        if (!buf.IsValid() || !obj || !typeName) {
            return false;
        }
        
        TypeDescriptor const* desc = TypeRegistry::GetTypeByName(typeName);
        if (!desc) {
            return false;
        }
        
        return DeserializeInternal(buf, obj, desc);
    }
    
    std::uint32_t GetCurrentVersion() const override {
        return version_;
    }
    
    void SetVersionMigration(IVersionMigration* migration) override {
        migration_ = migration;
    }
    
    SerializationFormat GetFormat() const override {
        return SerializationFormat::XML;
    }
    
private:
    bool SerializeInternal(SerializedBuffer& out, void const* obj, TypeDescriptor const* desc) {
        if (!obj || !desc) {
            return false;
        }
        
        XMLWriter writer;
        
        // Root element with type name
        char const* rootName = desc->name ? desc->name : "Object";
        writer.StartElement(rootName);
        
        // Write version and type as attributes
        writer.WriteAttribute("version", static_cast<int64_t>(version_));
        if (desc->name) {
            writer.WriteAttribute("type", desc->name);
        }
        
        // Serialize properties
        for (std::size_t i = 0; i < desc->propertyCount; ++i) {
            PropertyDescriptor const* prop = &desc->properties[i];
            if (!prop->name) continue;
            
            void const* propPtr = static_cast<char const*>(obj) + prop->offset;
            
            // Handle GUID
            if (IsGUIDType(prop->valueTypeId)) {
                GUID const* guid = static_cast<GUID const*>(propPtr);
                writer.StartElement(prop->name);
                writer.WriteAttribute("guid", guid->ToString().c_str());
                writer.EndElement();
                continue;
            }
            
            // Handle ObjectRef
            if (IsObjectRefType(prop->valueTypeId)) {
                ObjectRef const* ref = static_cast<ObjectRef const*>(propPtr);
                writer.StartElement(prop->name);
                writer.WriteAttribute("guid", ref->guid.ToString().c_str());
                writer.EndElement();
                continue;
            }
            
            // Handle basic types as attributes or text content
            if (IsBasicType(prop->valueTypeId, prop->size)) {
                writer.StartElement(prop->name);
                
                if (prop->size == sizeof(int32_t)) {
                    writer.WriteText(*static_cast<int32_t const*>(propPtr));
                } else if (prop->size == sizeof(int64_t)) {
                    writer.WriteText(*static_cast<int64_t const*>(propPtr));
                } else if (prop->size == sizeof(float)) {
                    writer.WriteText(static_cast<double>(*static_cast<float const*>(propPtr)));
                } else if (prop->size == sizeof(double)) {
                    writer.WriteText(*static_cast<double const*>(propPtr));
                } else if (prop->size == sizeof(bool)) {
                    writer.WriteText(*static_cast<bool const*>(propPtr));
                }
                
                writer.EndElement();
                continue;
            }
            
            // Handle complex types (nested objects) as child elements
            if (IsComplexType(prop->valueTypeId)) {
                TypeDescriptor const* nestedDesc = TypeRegistry::GetTypeById(prop->valueTypeId);
                if (nestedDesc) {
                    writer.StartElement(prop->name);
                    if (!SerializeProperty(writer, propPtr, nestedDesc)) {
                        return false;
                    }
                    writer.EndElement();
                    continue;
                }
            }
            
            // Unknown type, write as placeholder
            writer.StartElement(prop->name);
            writer.WriteText("[complex]");
            writer.EndElement();
        }
        
        writer.EndElement();
        
        // Convert to buffer
        std::string xmlStr = writer.GetString();
        std::size_t size = xmlStr.size() + 1;  // +1 for null terminator
        
        if (out.capacity < size) {
            if (out.data) {
                te::core::Free(out.data);
            }
            out.data = te::core::Alloc(size, alignof(char));
            if (!out.data) {
                return false;
            }
            out.capacity = size;
        }
        
        std::memcpy(out.data, xmlStr.c_str(), size);
        out.size = size - 1;  // Exclude null terminator
        
        return true;
    }
    
    bool SerializeProperty(XMLWriter& writer, void const* obj, TypeDescriptor const* desc) {
        if (!obj || !desc) {
            return false;
        }
        
        // Write properties of nested object
        for (std::size_t i = 0; i < desc->propertyCount; ++i) {
            PropertyDescriptor const* prop = &desc->properties[i];
            if (!prop->name) continue;
            
            void const* propPtr = static_cast<char const*>(obj) + prop->offset;
            
            // Handle GUID
            if (IsGUIDType(prop->valueTypeId)) {
                GUID const* guid = static_cast<GUID const*>(propPtr);
                writer.StartElement(prop->name);
                writer.WriteAttribute("guid", guid->ToString().c_str());
                writer.EndElement();
                continue;
            }
            
            // Handle ObjectRef
            if (IsObjectRefType(prop->valueTypeId)) {
                ObjectRef const* ref = static_cast<ObjectRef const*>(propPtr);
                writer.StartElement(prop->name);
                writer.WriteAttribute("guid", ref->guid.ToString().c_str());
                writer.EndElement();
                continue;
            }
            
            // Handle basic types
            if (IsBasicType(prop->valueTypeId, prop->size)) {
                writer.StartElement(prop->name);
                
                if (prop->size == sizeof(int32_t)) {
                    writer.WriteText(*static_cast<int32_t const*>(propPtr));
                } else if (prop->size == sizeof(int64_t)) {
                    writer.WriteText(*static_cast<int64_t const*>(propPtr));
                } else if (prop->size == sizeof(float)) {
                    writer.WriteText(static_cast<double>(*static_cast<float const*>(propPtr)));
                } else if (prop->size == sizeof(double)) {
                    writer.WriteText(*static_cast<double const*>(propPtr));
                } else if (prop->size == sizeof(bool)) {
                    writer.WriteText(*static_cast<bool const*>(propPtr));
                }
                
                writer.EndElement();
                continue;
            }
            
            // Handle nested complex types recursively
            if (IsComplexType(prop->valueTypeId)) {
                TypeDescriptor const* nestedDesc = TypeRegistry::GetTypeById(prop->valueTypeId);
                if (nestedDesc) {
                    writer.StartElement(prop->name);
                    if (!SerializeProperty(writer, propPtr, nestedDesc)) {
                        return false;
                    }
                    writer.EndElement();
                    continue;
                }
            }
            
            // Unknown type
            writer.StartElement(prop->name);
            writer.WriteText("[complex]");
            writer.EndElement();
        }
        
        return true;
    }
    
    bool DeserializeInternal(SerializedBuffer const& buf, void* obj, TypeDescriptor const* desc) {
        if (!buf.IsValid() || !obj || !desc) {
            return false;
        }
        
        // Ensure null-terminated string
        char const* xmlStr = static_cast<char const*>(buf.data);
        
        // Parse XML
        XMLParser parser(xmlStr);
        return parser.ParseElement(obj, desc);
    }
    
    std::uint32_t version_;
    IVersionMigration* migration_;
};

ISerializer* CreateXMLSerializer() {
    return new XMLSerializerImpl();
}

} // namespace object
} // namespace te
