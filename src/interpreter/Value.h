#ifndef MINISWIFT_VALUE_H
#define MINISWIFT_VALUE_H

#include <string>
#include <variant>
#include <vector>
#include <unordered_map>
#include <memory>

namespace miniswift {

// Forward declaration for recursive types
struct Value;

// Collection types
using Array = std::vector<Value>;
using Dictionary = std::unordered_map<std::string, Value>;

enum class ValueType {
    Nil,
    Bool,
    Int,
    Double,
    String,
    Array,
    Dictionary
};

struct Value {
    ValueType type;
    std::variant<std::monostate, bool, int, double, std::string, Array, Dictionary> value;

    Value() : type(ValueType::Nil), value(std::monostate{}) {}
    Value(bool v) : type(ValueType::Bool), value(v) {}
    Value(int v) : type(ValueType::Int), value(v) {}
    Value(double v) : type(ValueType::Double), value(v) {}
    Value(std::string v) : type(ValueType::String), value(v) {}
    Value(Array v) : type(ValueType::Array), value(v) {}
    Value(Dictionary v) : type(ValueType::Dictionary), value(v) {}
    
    // Helper methods for collections
    bool isArray() const { return type == ValueType::Array; }
    bool isDictionary() const { return type == ValueType::Dictionary; }
    
    Array& asArray() { return std::get<Array>(value); }
    const Array& asArray() const { return std::get<Array>(value); }
    
    Dictionary& asDictionary() { return std::get<Dictionary>(value); }
    const Dictionary& asDictionary() const { return std::get<Dictionary>(value); }
    
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