#ifndef MINISWIFT_VALUE_H
#define MINISWIFT_VALUE_H

#include <string>
#include <variant>
#include <vector>
#include <unordered_map>
#include <memory>

namespace miniswift {

// Forward declarations
struct Value;
struct FunctionStmt;
struct Closure;
class Environment;
class InstancePropertyContainer;
class ConstructorValue;
class DestructorValue;

// Collection types (using shared_ptr to avoid recursive definition)
using Array = std::shared_ptr<std::vector<Value>>;
using Dictionary = std::shared_ptr<std::unordered_map<std::string, Value>>;

// Struct value type with property support
struct StructValue {
    std::string structName;
    std::shared_ptr<std::unordered_map<std::string, Value>> members; // Legacy member storage
    std::unique_ptr<InstancePropertyContainer> properties; // New property system
    
    StructValue(const std::string& structName);
    StructValue(const std::string& structName, std::unordered_map<std::string, Value> memberMap);
    StructValue(const std::string& structName, std::unique_ptr<InstancePropertyContainer> props);
    
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

// Class value type (with reference counting for ARC and property support)
struct ClassValue {
    std::string className;
    std::shared_ptr<std::unordered_map<std::string, Value>> members; // Legacy member storage
    std::unique_ptr<InstancePropertyContainer> properties; // New property system
    mutable int refCount; // For ARC implementation
    
    ClassValue(const std::string& className);
    ClassValue(const std::string& className, std::unordered_map<std::string, Value> memberMap);
    ClassValue(const std::string& className, std::unique_ptr<InstancePropertyContainer> props);
    
    // Destructor
    ~ClassValue();
    
    // Copy constructor
    ClassValue(const ClassValue& other);
    
    // Move constructor
    ClassValue(ClassValue&& other) noexcept;
    
    // Copy assignment operator
    ClassValue& operator=(const ClassValue& other);
    
    // Move assignment operator
    ClassValue& operator=(ClassValue&& other) noexcept;
    
    bool operator==(const ClassValue& other) const;
    bool operator!=(const ClassValue& other) const;
};

// Enum value type
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

enum class ValueType {
    Nil,
    Bool,
    Int,
    Double,
    String,
    Array,
    Dictionary,
    Function,
    Enum,
    Struct,
    Class,
    Constructor,
    Destructor
};

struct Value {
    ValueType type;
    std::variant<std::monostate, bool, int, double, std::string, Array, Dictionary, std::shared_ptr<Function>, EnumValue, StructValue, std::shared_ptr<ClassValue>, std::shared_ptr<ConstructorValue>, std::shared_ptr<DestructorValue>> value;

    Value() : type(ValueType::Nil), value(std::monostate{}) {}
    Value(bool v) : type(ValueType::Bool), value(v) {}
    Value(int v) : type(ValueType::Int), value(v) {}
    Value(double v) : type(ValueType::Double), value(v) {}
    Value(std::string v) : type(ValueType::String), value(v) {}
    Value(Array v) : type(ValueType::Array), value(v) {}
    Value(Dictionary v) : type(ValueType::Dictionary), value(v) {}
    Value(std::shared_ptr<Function> v) : type(ValueType::Function), value(v) {}
    Value(EnumValue v) : type(ValueType::Enum), value(v) {}
    Value(StructValue v) : type(ValueType::Struct), value(v) {}
    Value(std::shared_ptr<ClassValue> v) : type(ValueType::Class), value(v) {}
    Value(std::shared_ptr<ConstructorValue> v) : type(ValueType::Constructor), value(v) {}
    Value(std::shared_ptr<DestructorValue> v) : type(ValueType::Destructor), value(v) {}
    
    // Convenience constructors for collections
    Value(std::vector<Value> v) : type(ValueType::Array), value(std::make_shared<std::vector<Value>>(std::move(v))) {}
    Value(std::unordered_map<std::string, Value> v) : type(ValueType::Dictionary), value(std::make_shared<std::unordered_map<std::string, Value>>(std::move(v))) {}
    
    // Helper methods for collections and functions
    bool isArray() const { return type == ValueType::Array; }
    bool isDictionary() const { return type == ValueType::Dictionary; }
    bool isFunction() const { return type == ValueType::Function; }
    bool isEnum() const { return type == ValueType::Enum; }
    bool isStruct() const { return type == ValueType::Struct; }
    bool isClass() const { return type == ValueType::Class; }
    bool isConstructor() const { return type == ValueType::Constructor; }
    bool isDestructor() const { return type == ValueType::Destructor; }
    bool isClosure() const { 
        if (type != ValueType::Function) return false;
        auto func = std::get<std::shared_ptr<Function>>(value);
        return func && !func->isFunction;
    }
    
    Array& asArray() { return std::get<Array>(value); }
    const Array& asArray() const { return std::get<Array>(value); }
    
    Dictionary& asDictionary() { return std::get<Dictionary>(value); }
    const Dictionary& asDictionary() const { return std::get<Dictionary>(value); }
    
    // Convenience methods for accessing collection contents
    std::vector<Value>& asArrayRef() { return *std::get<Array>(value); }
    const std::vector<Value>& asArrayRef() const { return *std::get<Array>(value); }
    
    std::unordered_map<std::string, Value>& asDictionaryRef() { return *std::get<Dictionary>(value); }
    const std::unordered_map<std::string, Value>& asDictionaryRef() const { return *std::get<Dictionary>(value); }
    
    std::shared_ptr<Function>& asFunction() { return std::get<std::shared_ptr<Function>>(value); }
    const std::shared_ptr<Function>& asFunction() const { return std::get<std::shared_ptr<Function>>(value); }
    
    std::shared_ptr<Function>& asClosure() { return std::get<std::shared_ptr<Function>>(value); }
    const std::shared_ptr<Function>& asClosure() const { return std::get<std::shared_ptr<Function>>(value); }
    
    EnumValue& asEnum() { return std::get<EnumValue>(value); }
    const EnumValue& asEnum() const { return std::get<EnumValue>(value); }
    
    StructValue& asStruct() { return std::get<StructValue>(value); }
    const StructValue& asStruct() const { return std::get<StructValue>(value); }
    
    std::shared_ptr<ClassValue>& asClass() { return std::get<std::shared_ptr<ClassValue>>(value); }
    const std::shared_ptr<ClassValue>& asClass() const { return std::get<std::shared_ptr<ClassValue>>(value); }
    
    std::shared_ptr<ConstructorValue>& asConstructor() { return std::get<std::shared_ptr<ConstructorValue>>(value); }
    const std::shared_ptr<ConstructorValue>& asConstructor() const { return std::get<std::shared_ptr<ConstructorValue>>(value); }
    
    std::shared_ptr<DestructorValue>& asDestructor() { return std::get<std::shared_ptr<DestructorValue>>(value); }
    const std::shared_ptr<DestructorValue>& asDestructor() const { return std::get<std::shared_ptr<DestructorValue>>(value); }
    
    // Comparison operators
    bool operator==(const Value& other) const {
        if (type != other.type) return false;
        return value == other.value;
    }
    
    bool operator!=(const Value& other) const {
        return !(*this == other);
    }
};

} // namespace miniswift

#endif // MINISWIFT_VALUE_H