#ifndef MINISWIFT_VALUE_H
#define MINISWIFT_VALUE_H

#include <string>
#include <variant>
#include <vector>
#include <unordered_map>
#include <memory>
#include "ErrorHandling.h"

// Forward declaration for UI support
namespace MiniSwift {
    namespace UI {
        class UIWidget;
        class UIWidgetValue;
    }
}

namespace miniswift {

// Forward declarations
struct Value;
struct FunctionStmt;
struct Closure;
class Environment;
class InstancePropertyContainer;
class ConstructorValue;
class DestructorValue;
class SubscriptManager;
struct ErrorValue;
template<typename T> struct Result;

// Collection types (using shared_ptr to avoid recursive definition)
using Array = std::shared_ptr<std::vector<Value>>;
using Dictionary = std::shared_ptr<std::unordered_map<std::string, Value>>;

// Tuple wrapper to distinguish from Array in variant
struct TupleValue {
    std::shared_ptr<std::vector<Value>> elements;
    
    TupleValue() : elements(std::make_shared<std::vector<Value>>()) {}
    TupleValue(std::shared_ptr<std::vector<Value>> elems) : elements(elems) {}
    TupleValue(std::vector<Value> elems) : elements(std::make_shared<std::vector<Value>>(std::move(elems))) {}
    
    bool operator==(const TupleValue& other) const {
        return elements == other.elements;
    }
    
    bool operator!=(const TupleValue& other) const {
        return !(*this == other);
    }
};

using Tuple = TupleValue;

// Struct value type with property support
struct StructValue {
    std::string structName;
    std::shared_ptr<std::unordered_map<std::string, Value>> members; // Legacy member storage
    std::shared_ptr<InstancePropertyContainer> properties; // New property system (shared for value semantics)
    std::unique_ptr<SubscriptManager> subscripts; // Subscript support
    
    StructValue(const std::string& structName);
    StructValue(const std::string& structName, std::unordered_map<std::string, Value> memberMap);
    StructValue(const std::string& structName, std::shared_ptr<InstancePropertyContainer> props);
    
    // Destructor
    ~StructValue();
    
    // Copy constructor
    StructValue(const StructValue& other);
    
    // Move constructor
    StructValue(StructValue&& other) noexcept;
    
    // Copy assignment operator
    StructValue& operator=(const StructValue& other);
    
    // Move assignment operator
    StructValue& operator=(StructValue&& other) noexcept;
    
    bool operator==(const StructValue& other) const;
    bool operator!=(const StructValue& other) const;
};

// Forward declaration for inheritance support
class InheritanceManager;

// Class instance type (with inheritance support)
struct ClassInstance {
    std::string className;
    std::shared_ptr<std::unordered_map<std::string, Value>> members; // Legacy member storage
    std::unique_ptr<InstancePropertyContainer> properties; // New property system
    std::unique_ptr<SubscriptManager> subscripts; // Subscript support
    mutable int refCount; // For ARC implementation
    
    // Inheritance support
    std::string actualClassName; // The actual runtime type (for polymorphism)
    
    ClassInstance(const std::string& className);
    ClassInstance(const std::string& className, std::unordered_map<std::string, Value> memberMap);
    ClassInstance(const std::string& className, std::unique_ptr<InstancePropertyContainer> props);
    
    // Destructor
    ~ClassInstance();
    
    // Copy constructor
    ClassInstance(const ClassInstance& other);
    
    // Move constructor
    ClassInstance(ClassInstance&& other) noexcept;
    
    // Copy assignment operator
    ClassInstance& operator=(const ClassInstance& other);
    
    // Move assignment operator
    ClassInstance& operator=(ClassInstance&& other) noexcept;
    
    // Inheritance-related methods
    std::string getClassName() const { return actualClassName.empty() ? className : actualClassName; }
    void setActualClassName(const std::string& actualClass) { actualClassName = actualClass; }
    
    // Type checking methods
    bool isInstanceOf(const std::string& targetClass, InheritanceManager* inheritanceManager = nullptr) const;
    bool canCastTo(const std::string& targetClass, InheritanceManager* inheritanceManager = nullptr) const;
    
    bool operator==(const ClassInstance& other) const;
    bool operator!=(const ClassInstance& other) const;
};

// Keep ClassValue as alias for backward compatibility
using ClassValue = ClassInstance;

// Forward declaration for EnumValue
struct EnumValue;

// Callable type (for both functions and closures)
struct Callable {
    const FunctionStmt* functionDecl;
    const Closure* closureDecl;
    std::shared_ptr<Environment> closure;
    bool isFunction;
    
    // Constructor for function
    Callable(const FunctionStmt* decl, std::shared_ptr<Environment> env)
        : functionDecl(decl), closureDecl(nullptr), closure(env), isFunction(true) {}
    
    // Constructor for closure
    Callable(const Closure* decl, std::shared_ptr<Environment> env)
        : functionDecl(nullptr), closureDecl(decl), closure(env), isFunction(false) {}
};

// Keep Function as alias for backward compatibility
using Function = Callable;



// Forward declaration
struct Value;

// Optional value type
struct OptionalValue {
    bool hasValue;
    std::shared_ptr<Value> wrappedValue;
    
    OptionalValue() : hasValue(false), wrappedValue(nullptr) {}
    OptionalValue(const Value& value);
    OptionalValue(const OptionalValue& other) : hasValue(other.hasValue), wrappedValue(other.wrappedValue) {}
    OptionalValue& operator=(const OptionalValue& other) {
        if (this != &other) {
            hasValue = other.hasValue;
            wrappedValue = other.wrappedValue;
        }
        return *this;
    }
    
    bool operator==(const OptionalValue& other) const;
    
    bool operator!=(const OptionalValue& other) const {
        return !(*this == other);
    }
};

enum class ValueType {
  Nil,
  Bool,
  Int,
  Double,
  String,
  Array,
  Dictionary,
  Tuple,
  Function,
  Enum,
  Struct,
  Class,
  Constructor,
  Destructor,
  Optional,
  Error,
  Result,
  
  // Extended Integer Types
  Int8,
  Int16,
  Int32,
  Int64,
  UInt,
  UInt8,
  UInt16,
  UInt64,
  
  // Additional Basic Types
  Float,
  Character,
  
  // Special Types
  Any,
  Void,
  Set,
  
  // UI Types
  UIWidget
};

struct Value {
    ValueType type;
    std::variant<std::monostate, bool, int, double, std::string, Array, Dictionary, Tuple, std::shared_ptr<Function>, std::shared_ptr<EnumValue>, StructValue, std::shared_ptr<ClassValue>, std::shared_ptr<ConstructorValue>, std::shared_ptr<DestructorValue>, OptionalValue, ErrorValue, std::shared_ptr<ValueResult>, 
                 int8_t, int16_t, int64_t, uint32_t, uint8_t, uint16_t, uint64_t, float, char, std::shared_ptr<MiniSwift::UI::UIWidgetValue>> value;

    Value() : type(ValueType::Nil), value(std::monostate{}) {}
    Value(bool v) : type(ValueType::Bool), value(v) {}
    Value(int v) : type(ValueType::Int), value(v) {}
    Value(double v) : type(ValueType::Double), value(v) {}
    Value(const char* v) : type(ValueType::String), value(std::string(v)) {}
    Value(std::string v) : type(ValueType::String), value(v) {}
    Value(Array v) : type(ValueType::Array), value(v) {}
    Value(Dictionary v) : type(ValueType::Dictionary), value(v) {}
    Value(Tuple v) : type(ValueType::Tuple), value(v) {}
    Value(std::shared_ptr<Function> v) : type(ValueType::Function), value(v) {}
    Value(std::shared_ptr<EnumValue> v) : type(ValueType::Enum), value(v) {}
    Value(StructValue v) : type(ValueType::Struct), value(v) {}
    Value(std::shared_ptr<ClassValue> v) : type(ValueType::Class), value(v) {}
    Value(std::shared_ptr<ConstructorValue> v) : type(ValueType::Constructor), value(v) {}
    Value(std::shared_ptr<DestructorValue> v) : type(ValueType::Destructor), value(v) {}
    Value(OptionalValue v) : type(ValueType::Optional), value(v) {}
    Value(ErrorValue v) : type(ValueType::Error), value(v) {}
    Value(ValueResult v) : type(ValueType::Result), value(std::make_shared<ValueResult>(v)) {}
    Value(std::shared_ptr<ValueResult> v) : type(ValueType::Result), value(v) {}
    
    // Extended Integer Type constructors
    Value(int8_t v) : type(ValueType::Int8), value(v) {}
    Value(int16_t v) : type(ValueType::Int16), value(v) {}
    Value(int64_t v) : type(ValueType::Int64), value(v) {}
    Value(uint32_t v) : type(ValueType::UInt), value(v) {}
    Value(uint8_t v) : type(ValueType::UInt8), value(v) {}
    Value(uint16_t v) : type(ValueType::UInt16), value(v) {}
    Value(uint64_t v) : type(ValueType::UInt64), value(v) {}
    
    // Additional Basic Type constructors
    Value(float v) : type(ValueType::Float), value(v) {}
    Value(char v) : type(ValueType::Character), value(v) {}
    
    // Static factory methods for special types
    static Value createAny() {
        Value anyValue;
        anyValue.type = ValueType::Any;
        anyValue.value = std::monostate{};
        return anyValue;
    }
    
    static Value createVoid() {
        Value voidValue;
        voidValue.type = ValueType::Void;
        voidValue.value = std::monostate{};
        return voidValue;
    }
    
    static Value createSet() {
        Value setValue;
        setValue.type = ValueType::Set;
        setValue.value = std::monostate{};
        return setValue;
    }

    
    // Copy constructor
    Value(const Value& other) : type(other.type), value(other.value) {}
    
    // Copy assignment operator
    Value& operator=(const Value& other) {
        if (this != &other) {
            type = other.type;
            value = other.value;
        }
        return *this;
    }
    
    // Convenience constructors for collections
    Value(std::vector<Value> v) : type(ValueType::Array), value(std::make_shared<std::vector<Value>>(std::move(v))) {}
    Value(std::unordered_map<std::string, Value> v) : type(ValueType::Dictionary), value(std::make_shared<std::unordered_map<std::string, Value>>(std::move(v))) {}
    
    // Helper methods for collections and functions
    bool isArray() const { return type == ValueType::Array; }
    bool isDictionary() const { return type == ValueType::Dictionary; }
    bool isTuple() const { return type == ValueType::Tuple; }
    bool isFunction() const { return type == ValueType::Function; }
    bool isEnum() const { return type == ValueType::Enum; }
    bool isStruct() const { return type == ValueType::Struct; }
    bool isClass() const { return type == ValueType::Class; }
    bool isUIWidget() const { return type == ValueType::UIWidget; }
    
    // UIWidget constructor
    Value(std::shared_ptr<MiniSwift::UI::UIWidgetValue> widget) : type(ValueType::UIWidget), value(widget) {}
    bool isConstructor() const { return type == ValueType::Constructor; }
    bool isDestructor() const { return type == ValueType::Destructor; }
    bool isOptional() const { return type == ValueType::Optional; }
    bool isError() const { return type == ValueType::Error; }
    bool isResult() const { return type == ValueType::Result; }
    bool isClosure() const { 
        if (type != ValueType::Function) return false;
        auto func = std::get<std::shared_ptr<Function>>(value);
        return func && !func->isFunction;
    }
    
    // Type checking methods for new basic types
    bool isInt8() const { return type == ValueType::Int8; }
    bool isInt16() const { return type == ValueType::Int16; }
    bool isInt32() const { return type == ValueType::Int32; }
    bool isInt64() const { return type == ValueType::Int64; }
    bool isUInt() const { return type == ValueType::UInt; }
    bool isUInt8() const { return type == ValueType::UInt8; }
    bool isUInt16() const { return type == ValueType::UInt16; }
    bool isUInt64() const { return type == ValueType::UInt64; }
    bool isFloat() const { return type == ValueType::Float; }
    bool isCharacter() const { return type == ValueType::Character; }
    bool isSet() const { return type == ValueType::Set; }
    bool isAny() const { return type == ValueType::Any; }
    bool isVoid() const { return type == ValueType::Void; }
    
    Array& asArray() { return std::get<Array>(value); }
    const Array& asArray() const { return std::get<Array>(value); }
    
    Dictionary& asDictionary() { return std::get<Dictionary>(value); }
    const Dictionary& asDictionary() const { return std::get<Dictionary>(value); }
    
    Tuple& asTuple() { return std::get<Tuple>(value); }
    const Tuple& asTuple() const { return std::get<Tuple>(value); }
    
    // Convenience methods for accessing collection contents
    std::vector<Value>& asArrayRef() { return *std::get<Array>(value); }
    const std::vector<Value>& asArrayRef() const { return *std::get<Array>(value); }
    
    std::unordered_map<std::string, Value>& asDictionaryRef() { return *std::get<Dictionary>(value); }
    const std::unordered_map<std::string, Value>& asDictionaryRef() const { return *std::get<Dictionary>(value); }
    
    std::vector<Value>& asTupleRef() { return *std::get<Tuple>(value).elements; }
    const std::vector<Value>& asTupleRef() const { return *std::get<Tuple>(value).elements; }
    
    std::shared_ptr<Function>& asFunction() { return std::get<std::shared_ptr<Function>>(value); }
    const std::shared_ptr<Function>& asFunction() const { return std::get<std::shared_ptr<Function>>(value); }
    
    std::shared_ptr<Function>& asClosure() { return std::get<std::shared_ptr<Function>>(value); }
    const std::shared_ptr<Function>& asClosure() const { return std::get<std::shared_ptr<Function>>(value); }
    
    std::shared_ptr<EnumValue>& asEnum() { return std::get<std::shared_ptr<EnumValue>>(value); }
    const std::shared_ptr<EnumValue>& asEnum() const { return std::get<std::shared_ptr<EnumValue>>(value); }
    
    StructValue& asStruct() { return std::get<StructValue>(value); }
    const StructValue& asStruct() const { return std::get<StructValue>(value); }
    
    std::shared_ptr<ClassValue>& asClass() { return std::get<std::shared_ptr<ClassValue>>(value); }
    const std::shared_ptr<ClassValue>& asClass() const { return std::get<std::shared_ptr<ClassValue>>(value); }
    
    std::shared_ptr<ConstructorValue>& asConstructor() { return std::get<std::shared_ptr<ConstructorValue>>(value); }
    const std::shared_ptr<ConstructorValue>& asConstructor() const { return std::get<std::shared_ptr<ConstructorValue>>(value); }
    
    std::shared_ptr<DestructorValue>& asDestructor() { return std::get<std::shared_ptr<DestructorValue>>(value); }
    const std::shared_ptr<DestructorValue>& asDestructor() const { return std::get<std::shared_ptr<DestructorValue>>(value); }
    
    OptionalValue& asOptional() { return std::get<OptionalValue>(value); }
    const OptionalValue& asOptional() const { return std::get<OptionalValue>(value); }
    
    ErrorValue& asError() { return std::get<ErrorValue>(value); }
    const ErrorValue& asError() const { return std::get<ErrorValue>(value); }
    std::shared_ptr<ValueResult>& asResult() { return std::get<std::shared_ptr<ValueResult>>(value); }
    const std::shared_ptr<ValueResult>& asResult() const { return std::get<std::shared_ptr<ValueResult>>(value); }
    
    // Accessor methods for new basic types
    int8_t& asInt8() { return std::get<int8_t>(value); }
    const int8_t& asInt8() const { return std::get<int8_t>(value); }
    
    int16_t& asInt16() { return std::get<int16_t>(value); }
    const int16_t& asInt16() const { return std::get<int16_t>(value); }
    
    int& asInt32() { return std::get<int>(value); }
    const int& asInt32() const { return std::get<int>(value); }
    
    int64_t& asInt64() { return std::get<int64_t>(value); }
    const int64_t& asInt64() const { return std::get<int64_t>(value); }
    
    uint32_t& asUInt() { return std::get<uint32_t>(value); }
    const uint32_t& asUInt() const { return std::get<uint32_t>(value); }
    
    uint8_t& asUInt8() { return std::get<uint8_t>(value); }
    const uint8_t& asUInt8() const { return std::get<uint8_t>(value); }
    
    uint16_t& asUInt16() { return std::get<uint16_t>(value); }
    const uint16_t& asUInt16() const { return std::get<uint16_t>(value); }
    
    uint64_t& asUInt64() { return std::get<uint64_t>(value); }
    const uint64_t& asUInt64() const { return std::get<uint64_t>(value); }
    
    float& asFloat() { return std::get<float>(value); }
    const float& asFloat() const { return std::get<float>(value); }
    
    char& asCharacter() { return std::get<char>(value); }
    const char& asCharacter() const { return std::get<char>(value); }

    
    // Helper methods for optional values
    Value getOptionalValue() const {
        const auto& opt = asOptional();
        if (opt.hasValue && opt.wrappedValue) {
            return *opt.wrappedValue;
        }
        return Value(); // Return nil
    }
    
    // Comparison operators
    bool operator==(const Value& other) const {
        if (type != other.type) return false;
        
        switch (type) {
            case ValueType::Nil:
                return true;
            case ValueType::Bool:
                return std::get<bool>(value) == std::get<bool>(other.value);
            case ValueType::Int:
                return std::get<int>(value) == std::get<int>(other.value);
            case ValueType::Double:
                return std::get<double>(value) == std::get<double>(other.value);
            case ValueType::String:
                return std::get<std::string>(value) == std::get<std::string>(other.value);
            case ValueType::Array:
                return std::get<Array>(value) == std::get<Array>(other.value);
            case ValueType::Dictionary:
                return std::get<Dictionary>(value) == std::get<Dictionary>(other.value);
            case ValueType::Tuple:
                return std::get<Tuple>(value) == std::get<Tuple>(other.value);
            case ValueType::Function:
                return std::get<std::shared_ptr<Function>>(value) == std::get<std::shared_ptr<Function>>(other.value);
            case ValueType::Enum: {
                 auto thisEnum = std::get<std::shared_ptr<EnumValue>>(value);
                 auto otherEnum = std::get<std::shared_ptr<EnumValue>>(other.value);
                 return thisEnum == otherEnum; // Pointer comparison for now
             }
            case ValueType::Struct:
                return std::get<StructValue>(value) == std::get<StructValue>(other.value);
            case ValueType::Class:
                return std::get<std::shared_ptr<ClassValue>>(value) == std::get<std::shared_ptr<ClassValue>>(other.value);
            case ValueType::Constructor:
                return std::get<std::shared_ptr<ConstructorValue>>(value) == std::get<std::shared_ptr<ConstructorValue>>(other.value);
            case ValueType::Destructor:
                return std::get<std::shared_ptr<DestructorValue>>(value) == std::get<std::shared_ptr<DestructorValue>>(other.value);
            case ValueType::Optional:
                return std::get<OptionalValue>(value) == std::get<OptionalValue>(other.value);
            case ValueType::Error:
                return std::get<ErrorValue>(value) == std::get<ErrorValue>(other.value);
            case ValueType::Result:
                return *std::get<std::shared_ptr<ValueResult>>(value) == *std::get<std::shared_ptr<ValueResult>>(other.value);
            
            // Extended Integer Types
            case ValueType::Int8:
                return std::get<int8_t>(value) == std::get<int8_t>(other.value);
            case ValueType::Int16:
                return std::get<int16_t>(value) == std::get<int16_t>(other.value);
            case ValueType::Int32:
            return std::get<int>(value) == std::get<int>(other.value);
            case ValueType::Int64:
                return std::get<int64_t>(value) == std::get<int64_t>(other.value);
            case ValueType::UInt:
                return std::get<uint32_t>(value) == std::get<uint32_t>(other.value);
            case ValueType::UInt8:
                return std::get<uint8_t>(value) == std::get<uint8_t>(other.value);
            case ValueType::UInt16:
                return std::get<uint16_t>(value) == std::get<uint16_t>(other.value);
            case ValueType::UInt64:
                return std::get<uint64_t>(value) == std::get<uint64_t>(other.value);
            
            // Additional Basic Types
            case ValueType::Float:
                return std::get<float>(value) == std::get<float>(other.value);
            case ValueType::Character:
                return std::get<char>(value) == std::get<char>(other.value);
            
            // Special Types
             case ValueType::Set:
                 return true; // For now, all sets are considered equal (placeholder implementation)
             case ValueType::Any:
                 return true; // For now, all Any values are considered equal (placeholder implementation)
             case ValueType::Void:
                 return true; // All void values are equal
            
            default:
                return false;
        }
    }
    
    bool operator!=(const Value& other) const {
        return !(*this == other);
    }
};

// Enum value type (defined after Value to avoid circular dependency)
struct EnumValue {
    std::string enumName;
    std::string caseName;
    std::vector<Value> associatedValues;
    
    EnumValue(const std::string& enumName, const std::string& caseName)
        : enumName(enumName), caseName(caseName) {}
    
    EnumValue(const std::string& enumName, const std::string& caseName, std::vector<Value> values)
        : enumName(enumName), caseName(caseName), associatedValues(std::move(values)) {}
    
    bool operator==(const EnumValue& other) const {
        return enumName == other.enumName && caseName == other.caseName && associatedValues == other.associatedValues;
    }
    
    bool operator!=(const EnumValue& other) const {
        return !(*this == other);
    }
};



// Forward declaration for helper function
bool compareOptionalValues(const OptionalValue& lhs, const OptionalValue& rhs);

} // namespace miniswift

#endif // MINISWIFT_VALUE_H