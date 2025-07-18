#include "Value.h"
#include "OOP/Property.h"

namespace miniswift {

// StructValue constructors and destructor
StructValue::StructValue(const std::string& structName)
    : structName(structName), members(std::make_shared<std::unordered_map<std::string, Value>>()) {}

StructValue::StructValue(const std::string& structName, std::unordered_map<std::string, Value> memberMap)
    : structName(structName), members(std::make_shared<std::unordered_map<std::string, Value>>(std::move(memberMap))) {}

StructValue::StructValue(const std::string& structName, std::unique_ptr<InstancePropertyContainer> props)
    : structName(structName), members(std::make_shared<std::unordered_map<std::string, Value>>()), 
      properties(std::move(props)) {}

StructValue::~StructValue() = default;

// ClassValue constructors and destructor
ClassValue::ClassValue(const std::string& className)
    : className(className), members(std::make_shared<std::unordered_map<std::string, Value>>()), refCount(1) {}

ClassValue::ClassValue(const std::string& className, std::unordered_map<std::string, Value> memberMap)
    : className(className), members(std::make_shared<std::unordered_map<std::string, Value>>(std::move(memberMap))), refCount(1) {}

ClassValue::ClassValue(const std::string& className, std::unique_ptr<InstancePropertyContainer> props)
    : className(className), members(std::make_shared<std::unordered_map<std::string, Value>>()), 
      properties(std::move(props)), refCount(1) {}

ClassValue::~ClassValue() = default;

// StructValue copy constructor
StructValue::StructValue(const StructValue& other)
    : structName(other.structName), members(other.members), properties(nullptr) {
    // Copy property container if it exists
    if (other.properties) {
        // Create a new property container with the same manager and environment
        properties = std::make_unique<InstancePropertyContainer>(*other.properties);
    }
}

// StructValue move constructor
StructValue::StructValue(StructValue&& other) noexcept
    : structName(std::move(other.structName)), 
      members(std::move(other.members)), 
      properties(std::move(other.properties)) {}

// StructValue copy assignment operator
StructValue& StructValue::operator=(const StructValue& other) {
    if (this != &other) {
        structName = other.structName;
        members = other.members;
        // Copy property container if it exists
        if (other.properties) {
            properties = std::make_unique<InstancePropertyContainer>(*other.properties);
        } else {
            properties.reset();
        }
    }
    return *this;
}

// StructValue move assignment operator
StructValue& StructValue::operator=(StructValue&& other) noexcept {
    if (this != &other) {
        structName = std::move(other.structName);
        members = std::move(other.members);
        properties = std::move(other.properties);
    }
    return *this;
}

// StructValue comparison operators
bool StructValue::operator==(const StructValue& other) const {
    return structName == other.structName && *members == *other.members;
}

bool StructValue::operator!=(const StructValue& other) const {
    return !(*this == other);
}

// ClassValue copy constructor
ClassValue::ClassValue(const ClassValue& other)
    : className(other.className), members(other.members), properties(nullptr), refCount(1) {
    // Note: properties are not copied, only shared members
    // This is intentional as property containers are instance-specific
    // refCount starts at 1 for new instance
}

// ClassValue move constructor
ClassValue::ClassValue(ClassValue&& other) noexcept
    : className(std::move(other.className)), 
      members(std::move(other.members)), 
      properties(std::move(other.properties)),
      refCount(other.refCount) {
    other.refCount = 0; // Mark moved-from object
}

// ClassValue copy assignment operator
ClassValue& ClassValue::operator=(const ClassValue& other) {
    if (this != &other) {
        className = other.className;
        members = other.members;
        properties.reset(); // Reset properties, don't copy
        refCount = 1; // Reset reference count
    }
    return *this;
}

// ClassValue move assignment operator
ClassValue& ClassValue::operator=(ClassValue&& other) noexcept {
    if (this != &other) {
        className = std::move(other.className);
        members = std::move(other.members);
        properties = std::move(other.properties);
        refCount = other.refCount;
        other.refCount = 0; // Mark moved-from object
    }
    return *this;
}

// ClassValue comparison operators
bool ClassValue::operator==(const ClassValue& other) const {
    return className == other.className && *members == *other.members;
}

bool ClassValue::operator!=(const ClassValue& other) const {
    return !(*this == other);
}

} // namespace miniswift