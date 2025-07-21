#include "Value.h"
#include "OOP/Property.h"
#include "OOP/Subscript.h"

namespace miniswift {

// StructValue constructors and destructor
StructValue::StructValue(const std::string& structName)
    : structName(structName), members(std::make_shared<std::unordered_map<std::string, Value>>()), 
      subscripts(std::make_unique<SubscriptManager>()) {}

StructValue::StructValue(const std::string& structName, std::unordered_map<std::string, Value> memberMap)
    : structName(structName), members(std::make_shared<std::unordered_map<std::string, Value>>(std::move(memberMap))), 
      subscripts(std::make_unique<SubscriptManager>()) {}

StructValue::StructValue(const std::string& structName, std::shared_ptr<InstancePropertyContainer> props)
    : structName(structName), members(std::make_shared<std::unordered_map<std::string, Value>>()), 
      properties(std::move(props)), subscripts(std::make_unique<SubscriptManager>()) {}

StructValue::~StructValue() = default;

// ClassInstance constructors and destructor
ClassInstance::ClassInstance(const std::string& className)
    : className(className), members(std::make_shared<std::unordered_map<std::string, Value>>()), 
      subscripts(std::make_unique<SubscriptManager>()), refCount(1) {}

ClassInstance::ClassInstance(const std::string& className, std::unordered_map<std::string, Value> memberMap)
    : className(className), members(std::make_shared<std::unordered_map<std::string, Value>>(std::move(memberMap))), 
      subscripts(std::make_unique<SubscriptManager>()), refCount(1) {}

ClassInstance::ClassInstance(const std::string& className, std::unique_ptr<InstancePropertyContainer> props)
    : className(className), members(std::make_shared<std::unordered_map<std::string, Value>>()), 
      properties(std::move(props)), subscripts(std::make_unique<SubscriptManager>()), refCount(1) {}

ClassInstance::~ClassInstance() = default;

// StructValue copy constructor
StructValue::StructValue(const StructValue& other)
    : structName(other.structName), members(other.members), properties(other.properties), 
      subscripts(std::make_unique<SubscriptManager>()) {
    // Share property container instead of copying it
    // This ensures that modifications to properties are reflected across all copies
    // Note: subscripts are not copied, each instance gets its own manager
}

// StructValue move constructor
StructValue::StructValue(StructValue&& other) noexcept
    : structName(std::move(other.structName)), 
      members(std::move(other.members)), 
      properties(std::move(other.properties)),
      subscripts(std::move(other.subscripts)) {}

// StructValue copy assignment operator
StructValue& StructValue::operator=(const StructValue& other) {
    if (this != &other) {
        structName = other.structName;
        members = other.members;
        // Share property container instead of copying it
        properties = other.properties;
        // Reset subscripts, each instance gets its own manager
        subscripts = std::make_unique<SubscriptManager>();
    }
    return *this;
}

// StructValue move assignment operator
StructValue& StructValue::operator=(StructValue&& other) noexcept {
    if (this != &other) {
        structName = std::move(other.structName);
        members = std::move(other.members);
        properties = std::move(other.properties);
        subscripts = std::move(other.subscripts);
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
    : className(other.className), members(other.members), properties(nullptr), 
      subscripts(std::make_unique<SubscriptManager>()), refCount(1) {
    // Note: properties are not copied, only shared members
    // This is intentional as property containers are instance-specific
    // Note: subscripts are not copied, each instance gets its own manager
    // refCount starts at 1 for new instance
}

// ClassInstance move constructor
ClassInstance::ClassInstance(ClassInstance&& other) noexcept
    : className(std::move(other.className)), 
      members(std::move(other.members)), 
      properties(std::move(other.properties)),
      subscripts(std::move(other.subscripts)),
      refCount(other.refCount) {
    other.refCount = 0; // Mark moved-from object
}

// ClassInstance copy assignment operator
ClassInstance& ClassInstance::operator=(const ClassInstance& other) {
    if (this != &other) {
        className = other.className;
        members = other.members;
        properties.reset(); // Reset properties, don't copy
        subscripts = std::make_unique<SubscriptManager>(); // Reset subscripts
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
        subscripts = std::move(other.subscripts);
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

// OptionalValue comparison operator implementation
bool OptionalValue::operator==(const OptionalValue& other) const {
    return compareOptionalValues(*this, other);
}

// Helper function to compare OptionalValue objects
bool compareOptionalValues(const OptionalValue& lhs, const OptionalValue& rhs) {
    if (lhs.hasValue != rhs.hasValue) return false;
    if (!lhs.hasValue) return true; // Both are nil
    if (!lhs.wrappedValue || !rhs.wrappedValue) return false;
    
    // Compare the actual values
    const Value& leftVal = *lhs.wrappedValue;
    const Value& rightVal = *rhs.wrappedValue;
    
    if (leftVal.type != rightVal.type) return false;
    
    switch (leftVal.type) {
        case ValueType::Nil:
            return true;
        case ValueType::Bool:
            return std::get<bool>(leftVal.value) == std::get<bool>(rightVal.value);
        case ValueType::Int:
            return std::get<int>(leftVal.value) == std::get<int>(rightVal.value);
        case ValueType::Double:
            return std::get<double>(leftVal.value) == std::get<double>(rightVal.value);
        case ValueType::String:
            return std::get<std::string>(leftVal.value) == std::get<std::string>(rightVal.value);
        case ValueType::Array:
            return std::get<Array>(leftVal.value) == std::get<Array>(rightVal.value);
        case ValueType::Dictionary:
            return std::get<Dictionary>(leftVal.value) == std::get<Dictionary>(rightVal.value);
        case ValueType::Function:
            return std::get<std::shared_ptr<Function>>(leftVal.value) == std::get<std::shared_ptr<Function>>(rightVal.value);
        case ValueType::Enum:
            return std::get<EnumValue>(leftVal.value) == std::get<EnumValue>(rightVal.value);
        case ValueType::Struct:
            return std::get<StructValue>(leftVal.value) == std::get<StructValue>(rightVal.value);
        case ValueType::Class:
            return std::get<std::shared_ptr<ClassValue>>(leftVal.value) == std::get<std::shared_ptr<ClassValue>>(rightVal.value);
        case ValueType::Constructor:
            return std::get<std::shared_ptr<ConstructorValue>>(leftVal.value) == std::get<std::shared_ptr<ConstructorValue>>(rightVal.value);
        case ValueType::Destructor:
            return std::get<std::shared_ptr<DestructorValue>>(leftVal.value) == std::get<std::shared_ptr<DestructorValue>>(rightVal.value);
        case ValueType::Optional:
            // Recursive comparison for nested optionals
            return compareOptionalValues(std::get<OptionalValue>(leftVal.value), std::get<OptionalValue>(rightVal.value));
        default:
            return false;
    }
}

} // namespace miniswift