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

// ClassInstance constructors and destructor
ClassInstance::ClassInstance(const std::string& className)
    : className(className), members(std::make_shared<std::unordered_map<std::string, Value>>()), refCount(1) {}

ClassInstance::ClassInstance(const std::string& className, std::unordered_map<std::string, Value> memberMap)
    : className(className), members(std::make_shared<std::unordered_map<std::string, Value>>(std::move(memberMap))), refCount(1) {}

ClassInstance::ClassInstance(const std::string& className, std::unique_ptr<InstancePropertyContainer> props)
    : className(className), members(std::make_shared<std::unordered_map<std::string, Value>>()), 
      properties(std::move(props)), refCount(1) {}

ClassInstance::~ClassInstance() = default;

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

// ClassInstance copy constructor
ClassInstance::ClassInstance(const ClassInstance& other)
    : className(other.className), members(other.members), properties(nullptr), refCount(1) {
    // Note: properties are not copied, only shared members
    // This is intentional as property containers are instance-specific
    // refCount starts at 1 for new instance
}

// ClassInstance move constructor
ClassInstance::ClassInstance(ClassInstance&& other) noexcept
    : className(std::move(other.className)), 
      members(std::move(other.members)), 
      properties(std::move(other.properties)),
      refCount(other.refCount) {
    other.refCount = 0; // Mark moved-from object
}

// ClassInstance copy assignment operator
ClassInstance& ClassInstance::operator=(const ClassInstance& other) {
    if (this != &other) {
        className = other.className;
        members = other.members;
        properties.reset(); // Reset properties, don't copy
        refCount = 1; // Reset reference count
    }
    return *this;
}

// ClassInstance move assignment operator
ClassInstance& ClassInstance::operator=(ClassInstance&& other) noexcept {
    if (this != &other) {
        className = std::move(other.className);
        members = std::move(other.members);
        properties = std::move(other.properties);
        refCount = other.refCount;
        other.refCount = 0; // Mark moved-from object
    }
    return *this;
}

// ClassInstance comparison operators
bool ClassInstance::operator==(const ClassInstance& other) const {
    return className == other.className && *members == *other.members;
}

bool ClassInstance::operator!=(const ClassInstance& other) const {
    return !(*this == other);
}

} // namespace miniswift